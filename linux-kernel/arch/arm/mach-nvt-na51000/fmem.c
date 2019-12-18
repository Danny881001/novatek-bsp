#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/mmzone.h>
#include <linux/memory.h>
#include <linux/kallsyms.h>
#include <linux/nodemask.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <linux/cma.h>
#include <asm/setup.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/tlbflush.h>
#include <asm/glue-cache.h>
#include <asm/outercache.h>
#include <asm/mach/map.h>
#include <asm/mmu.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/of.h>
#include <mach/fmem.h>


static void __init fmem_proc_init(void);

/*
 * Local variables declaration
 */
static struct semaphore sema = __SEMAPHORE_INITIALIZER(sema, 1);
static struct device *fmem_dev_cma0 = NULL;

/* memory type */
static fmeminfo_t bank_info; /* used to maintain the real memory bank without any processing */

u32 nonlinear_cpuaddr_flush = 0;
u32 cpuaddr_flush = 0;
u32 va_not_64align = 0, length_not_64align = 0;
u32 debug_counter = 0;
static u32 vb_cache_flush_en = 0;

#define DEBUG_WRONG_CACHE_API   0x0
#define DEBUG_WRONG_CACHE_VA    0x1
#define DEBUG_WRONG_CACHE_LEN   0x2

#define CACHE_ALIGN_MASK  0x3F  //cache line with 64 bytes
#define MAX_CMD_LENGTH    30


#define NVT_DSP_CMA

/* proc function
 */
static struct proc_dir_entry *fmem_proc_root = NULL;
static struct proc_dir_entry *resolve_proc = NULL;
static struct proc_dir_entry *counter_proc = NULL;
static struct proc_dir_entry *vb_cacheflush_proc = NULL;

/* bank operation
 */
#define bank_pfn_start(bank)	__phys_to_pfn((bank)->start)
#define bank_pfn_end(bank)	    __phys_to_pfn((bank)->start + (bank)->size)
#define bank_pfn_size(bank)	    ((bank)->size >> PAGE_SHIFT)
#define bank_phys_start(bank)	((bank)->start)
#define bank_phys_end(bank)	    ((bank)->start + (bank)->size)
#define bank_phys_size(bank)	((bank)->size)
#define bank_phys_mtype(bank)   ((bank)->mtype)

void fmem_reserve_memory(void)
{
	return;
}

/* process the 2nd */
int __init fmem_post_init(void)
{
	fmem_proc_init();
    return 0;
}

static const struct of_device_id fmem_dt_match[] = {
        {.compatible = "nvt,fmem_cma" },
        {},
};
MODULE_DEVICE_TABLE(of, fmem_dt_match);

int fmem_add_bank(phys_addr_t paddr, unsigned long size, int chipid, int ddrid, fmem_mtype_t mtype)
{
	int	nr_bank, i;

	down(&sema);
	nr_bank = bank_info.nr_banks;
	if (nr_bank >= MAX_DDR_CHUNKS) {
		printk("%s fail, array is too small! \n", __func__);
    	dump_stack();
    	up(&sema);
		return -1;
	}
	/* checking duplication */
	for (i = 0; i < bank_info.nr_banks; i ++) {
		if ((bank_info.bank[i].chipid == chipid) &&
			(bank_info.bank[i].ddrid == ddrid) &&
			(bank_info.bank[i].mtype == mtype)) {
			printk("%s, chipid:%d, ddrid:%d, mtype:%d was registered already! \n", __func__, chipid, ddrid, mtype);
			dump_stack();
			up(&sema);
			return -1;
		}
	}

	bank_phys_start(&bank_info.bank[nr_bank]) = paddr;
	bank_phys_size(&bank_info.bank[nr_bank]) = size;
	bank_phys_mtype(&bank_info.bank[nr_bank]) = mtype;
	bank_info.bank[nr_bank].chipid = chipid;
	bank_info.bank[nr_bank].ddrid = ddrid;
	bank_info.nr_banks ++;
	up(&sema);

	printk("------------------------- %s, nr_bank = %d, %d \n", __func__, nr_bank, bank_info.nr_banks);

	return 0;
}

int fmem_del_bank(int chipid, int ddrid, fmem_mtype_t mtype)
{
	int	nr_bank, i, j, bFound = 0;

	down(&sema);
	nr_bank = bank_info.nr_banks;

	for (i = 0; i < bank_info.nr_banks; i ++) {
		if ((bank_info.bank[nr_bank].chipid == chipid) &&
			(bank_info.bank[nr_bank].ddrid == ddrid) &&
			(bank_info.bank[nr_bank].mtype == mtype)) {
				bFound = 1;
				break;
		}
	}
	if (!bFound) {
		printk("%s, chipid:%d, ddrid:%d, mtype:%d was not registered! \n", __func__, chipid, ddrid, mtype);
		up(&sema);
		return -1;
	}
	for (j = i; j < bank_info.nr_banks - 1; j ++) {
		memcpy(&bank_info.bank[j], &bank_info.bank[j+1], sizeof(bank_info.bank[0]));
	}
	/* last entry */
	if (j == (bank_info.nr_banks - 1)) {
		memset(&bank_info.bank[j], 0, sizeof(bank_info.bank[0]));
	}

	bank_info.nr_banks --;
	up(&sema);

	return 0;
}
static int fmem_cma_probe(struct platform_device *pdev)
{
	phys_addr_t	paddr;
	unsigned long size;
	struct cma *cma0;
	int ret = 0;

	/* pointer to shared-dma-pool in dts */
	of_reserved_mem_device_init(&pdev->dev);

	fmem_dev_cma0 = &pdev->dev;

	cma0 = dev_get_cma_area(&pdev->dev);
	if (cma0) {
		paddr = cma_get_base(cma0);
		size = cma_get_size(cma0);

		ret = fmem_add_bank(paddr, size, 0, 0, FMEM_MTYPE_MEM);
		if (!ret)
			printk("fmem: cma base:0x%x, size:0x%x \n", paddr, (u32)size);
	}
	return 0;
}

static struct platform_driver fmem_driver = {
	.probe = fmem_cma_probe,
	.driver = {
		.name = "fmem_cma",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(fmem_dt_match),
	},
};

/* @this function used to allocate memory from cma region.
 * @ 	Actually it call dma_alloc_from_contiguous() in kernel.
 * @param: cma_dev_idx: which cma resion. Currently only 0 is supported.
 * return: virtual address. NULL for fail, otherwise for success.
 * Note: It is only cachable.
 */
void *fmem_alloc_from_cma(int cma_region_idx, int page_count, phys_addr_t *paddr)
{
	struct page *page;
	void *vaddr = NULL;

	if (cma_region_idx != 0)
		return NULL;

	page = dma_alloc_from_contiguous(fmem_get_cmadev(cma_region_idx), page_count, 0);
	if (page != NULL) {
		*paddr = page_to_pfn(page) << PAGE_SHIFT;
		vaddr = __va(*paddr);
	}
	return vaddr;
}


/* @this function used to release memory to cma region.
 * @ 	Actually it call dma_release_from_contiguous() in kernel
 * @param: cma_dev_idx: which cma resion. Currently only 0 is supported.
 *		   vaddr: virtual address, paddr: physical address
 * return: 0 for success, <0 for fail.
 */
int fmem_release_from_cma(int cma_region_idx, void *vaddr, int page_count)
{
	struct page *pages;
	phys_addr_t	paddr;

	if (cma_region_idx != 0)
		return -1;

	paddr = __pa(vaddr);
	pages = pfn_to_page(paddr >> PAGE_SHIFT);
	if (!dma_release_from_contiguous(fmem_get_cmadev(cma_region_idx), pages, page_count))
		return -1;

	return 0;
}


/* This function returns those unecessary pages to kernel. */
static int __init fmem_early_init(void)
{
	phys_addr_t	paddr;
	struct cma *def_cma;
	unsigned long	size;

	def_cma = dev_get_cma_area(NULL);
	if (def_cma) {
		paddr = cma_get_base(def_cma);
		size = cma_get_size(def_cma);

		printk("%s: dma_contiguous_default_area: 0x%x, 0x%x \n", __func__, paddr, (u32)size);
	}

	platform_driver_register(&fmem_driver);

	fmem_post_init();
	return 0;
}
core_initcall(fmem_early_init);

#ifdef NVT_DSP_CMA
struct cma *nvt_dsp_cma0 = NULL;
EXPORT_SYMBOL(nvt_dsp_cma0);
struct cma *nvt_dsp_cma1 = NULL;
EXPORT_SYMBOL(nvt_dsp_cma1);

/*************************************************************************************************************
 * dsp cma handling
 *
 *
 *************************************************************************************************************/
static int nvt_dsp_cma_probe(struct platform_device *pdev)
{
	phys_addr_t paddr = 0;
	unsigned long size = 0;
	int ret = 0;
	unsigned int id = 0;

	/* pointer to shared-dma-pool in dts */
	ret = of_reserved_mem_device_init(&pdev->dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "memory reserved init failed with %d\n", ret);
		return ret;
	}

	of_property_read_u32(pdev->dev.of_node, "id",
				&id);

	if (id == 1) {
		nvt_dsp_cma1 = dev_get_cma_area(&pdev->dev);
		if (!nvt_dsp_cma1) {
			paddr = cma_get_base(nvt_dsp_cma1);
			size = cma_get_size(nvt_dsp_cma1);
			dev_err(&pdev->dev, "nvt_dsp_cma1: cma base:0x%x, size:0x%x failed\n", paddr, (u32)size);
			return -1;
		}
		dev_info(&pdev->dev, "nvt_dsp_cma1: cma base:0x%x, size:0x%x \n", paddr, (u32)size);
	} else {
		nvt_dsp_cma0 = dev_get_cma_area(&pdev->dev);
		if (!nvt_dsp_cma0) {
			paddr = cma_get_base(nvt_dsp_cma0);
			size = cma_get_size(nvt_dsp_cma0);
			dev_err(&pdev->dev, "nvt_dsp_cma0: cma base:0x%x, size:0x%x failed\n", paddr, (u32)size);
			return -1;
		}
		dev_info(&pdev->dev, "nvt_dsp_cma0: cma base:0x%x, size:0x%x \n", paddr, (u32)size);
	}

	dev_info(&pdev->dev, "Probe successfully \n");
	return 0;
}

static const struct of_device_id nvt_dsp_cma_dt_match[] = {
	{.compatible = "nvt,nvt_dsp_cma"},
	{},
};
MODULE_DEVICE_TABLE(of, nvt_dsp_cma_dt_match);

static struct platform_driver nvt_dsp_driver = {
	.probe = nvt_dsp_cma_probe,
	.driver = {
		.name = "nvt_dsp_cma",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(nvt_dsp_cma_dt_match),
	},
};

static int __init nvt_dsp_early_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&nvt_dsp_driver);
	if (ret < 0) {
		pr_err("%s: init failed\n", __func__);
	}

	return 0;
}
core_initcall(nvt_dsp_early_init);

void *fmem_alloc_from_dspcma(int page_count, phys_addr_t *paddr, unsigned int index)
{
	struct page *page;
	void *vaddr = NULL;
	struct cma * nvt_dsp_cma = NULL;

	if (index == 1)
		nvt_dsp_cma = nvt_dsp_cma1;
	else
		nvt_dsp_cma = nvt_dsp_cma0;

	if (!nvt_dsp_cma)
		return NULL;

	page = cma_alloc(nvt_dsp_cma, page_count, 0);
	if (page != NULL) {
		*paddr = page_to_pfn(page) << PAGE_SHIFT;
		vaddr = __va(*paddr);
	}
	return vaddr;
}

int fmem_release_from_dspcma(void *vaddr, int page_count, unsigned int index)
{
	struct page *pages;
	phys_addr_t	paddr;
	bool ret = false;
	struct cma * nvt_dsp_cma = NULL;

	if (index == 1)
		nvt_dsp_cma = nvt_dsp_cma1;
	else
		nvt_dsp_cma = nvt_dsp_cma0;

	if (!nvt_dsp_cma)
		return -1;

	paddr = __pa(vaddr);
	pages = pfn_to_page(paddr >> PAGE_SHIFT);
	ret = cma_release(nvt_dsp_cma, pages, page_count);
	if (!ret)
		return -1;

	return 0;
}

EXPORT_SYMBOL(fmem_alloc_from_dspcma);
EXPORT_SYMBOL(fmem_release_from_dspcma);
#endif /* NVT_DSP_CMA */

/* get cma dev pointer */
void *fmem_get_cmadev(int dev)
{
	return (void *)fmem_dev_cma0;
}


/* counter info
 */
static int proc_show_dbgcounter(struct seq_file *sfile, void *v)
{
	int i;

	seq_printf(sfile, "\n");
	for (i = 0; i < bank_info.nr_banks; i ++) {
		seq_printf(sfile, "bank_no: %d \n", i);
		seq_printf(sfile, "start: 0x%x \n", bank_phys_start(&bank_info.bank[i]));
		seq_printf(sfile, "size: %dM bytes \n", bank_phys_size(&bank_info.bank[i]) >> 20);
		seq_printf(sfile, "end: 0x%x \n", bank_phys_end(&bank_info.bank[i]));
		seq_printf(sfile, "mtype: %d \n", bank_phys_mtype(&bank_info.bank[i]));
		seq_printf(sfile, "----------------------------------------------\n");
	}

	seq_printf(sfile, "linear cpu_addr flush: %d \n", cpuaddr_flush);
	seq_printf(sfile, "non-linear cpu_addr flush: %d \n", nonlinear_cpuaddr_flush);
	seq_printf(sfile, "vaddr not cache alignment: %d \n", va_not_64align);
	seq_printf(sfile, "len not cache alignment: %d \n", length_not_64align);
	seq_printf(sfile, "debug_counter = 0x%x \n\n", debug_counter);

	seq_printf(sfile, "DEBUG_WRONG_CACHE_API = 0x%x \n", DEBUG_WRONG_CACHE_API);
	seq_printf(sfile, "DEBUG_WRONG_CACHE_VA = 0x%x \n", DEBUG_WRONG_CACHE_VA);
	seq_printf(sfile, "DEBUG_WRONG_CACHE_LEN = 0x%x \n\n", DEBUG_WRONG_CACHE_LEN);

	return 0;
}

/* debug counter */
static ssize_t proc_write_dbgcounter(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned char value[30];

	if (copy_from_user(value, buffer, count))
		return 0;

	sscanf(value, "%x\n", &debug_counter);

	printk("debug counter: 0x%x \n", debug_counter);

	return count;
}

static int dbgcounter_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_dbgcounter, NULL);
}

static const struct file_operations dbgcounter_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= dbgcounter_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= proc_write_dbgcounter,
};

/* address resolve
 */
static ssize_t proc_resolve_pa(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned char value[30];
	unsigned int vaddr;
	phys_addr_t  paddr;

	if (copy_from_user(value, buffer, count))
		return 0;

	sscanf(value, "%x\n", &vaddr);
	paddr = fmem_lookup_pa(vaddr);

	printk("Resolve vaddr: 0x%x ---> paddr: 0x%x \n", vaddr, paddr);

	return count;
}

static int resolve_proc_show(struct seq_file *m, void *v)
{
	return 0;
}
static int resolve_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, resolve_proc_show, NULL);
}

static const struct file_operations resolve_proc_fops = {
	.owner		= THIS_MODULE,
	.open       = resolve_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= proc_resolve_pa,
};

/* address resolve
 */
static ssize_t vb_cacheflush_proc_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		printk("Command length is too long!\n");
		goto ERR_OUT;
	}
	if (len == 0) {
		printk("Command length is 0!\n");
		goto ERR_OUT;
	}
	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}
	cmd_line[len - 1] = '\0';
	sscanf(cmd_line, "%d\n", &vb_cache_flush_en);
	printk("vb_cache_flush_en = 0x%x \n\n", vb_cache_flush_en);

	return size;
ERR_OUT:
	return -1;
}

static int vb_cacheflush_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "vb_cache_flush_en = 0x%x \n\n", vb_cache_flush_en);
	return 0;
}
static int vb_cacheflush_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, vb_cacheflush_proc_show, NULL);
}

static const struct file_operations vb_cacheflush_proc_fops = {
	.owner		= THIS_MODULE,
	.open       = vb_cacheflush_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= vb_cacheflush_proc_write,
};

static void __init fmem_proc_init(void)
{
	struct proc_dir_entry *p;

	p = proc_mkdir("fmem", NULL);
	if (p == NULL) {
		printk("%s, fail! \n", __func__);
		return;
	}

	fmem_proc_root = p;

	/* resolve, va->pa
	*/
	resolve_proc = proc_create("resolve", S_IRUGO, fmem_proc_root, &resolve_proc_fops);
	if (resolve_proc == NULL)
		panic("FMEM: Fail to create proc resolve_proc!\n");

	/* counter
	*/
	counter_proc = proc_create("dbg_info", S_IRUGO, fmem_proc_root, &dbgcounter_proc_fops);
	if (counter_proc == NULL)
		panic("FMEM: Fail to create proc counter_proc!\n");

	/* vb_cacheflush
	*/
	vb_cacheflush_proc = proc_create("vb_cacheflush", S_IRUGO, fmem_proc_root, &vb_cacheflush_proc_fops);
	if (vb_cacheflush_proc == NULL)
		panic("FMEM: Fail to create proc vb_cacheflush!\n");
}


/**
 * @brief to resolve the virtual address (including direct mapping, ioremap or user space address to
 *      its real physical address.
 *
 * @parm vaddr indicates any virtual address
 *
 * @return  >= 0 for success, 0xFFFFFFFF for fail
 */
phys_addr_t fmem_lookup_pa(unsigned int vaddr)
{
	pgd_t *pgdp;
	pmd_t *pmdp;
	pud_t *pudp;
	pte_t *ptep;
	phys_addr_t paddr = 0xFFFFFFFF;


	/* check if va is module space global variable */
	if (vaddr >= TASK_SIZE && vaddr < PAGE_OFFSET) {
		printk("Invalid va 0x%x , TASK_SIZE = 0x%x, PAGE_OFFSET = 0x%x\r\n", vaddr, (int)TASK_SIZE, (int)PAGE_OFFSET);
		dump_stack();
		return paddr;
	}
	/* for speed up */
	if (virt_addr_valid(vaddr))
		return (phys_addr_t)__pa(vaddr);

	/*
	* The pgd is never bad, and a pmd always exists(as it's folded into the pgd entry)
	*/
	if (vaddr >= TASK_SIZE) /* use init_mm as mmu to look up in kernel mode */{
		pgdp = pgd_offset_k(vaddr);
	} else if (current && current->mm) {
		pgdp = pgd_offset(current->mm, vaddr);
	} else {
		printk("Invalid va 0x%x \r\n", vaddr);
		dump_stack();
		return paddr;
	}
	if (unlikely(pgd_none(*pgdp))) {
		printk("fmem: 0x%x not mapped in pgd table! \n", vaddr);
		goto err;
	}

	/* because we don't have 3-level MMU, so pud = pgd. Here we are in order to meet generic Linux
	* version, pud is listed here.
	*/
	pudp = pud_offset(pgdp, vaddr);
	if (unlikely(pud_none(*pudp))) {
		printk(KERN_INFO"fmem: 0x%x not mapped in pud table! \n", vaddr);
		goto err;
	}

	pmdp = pmd_offset(pudp, vaddr);
	if (unlikely(pmd_none(*pmdp))) {
		printk(KERN_INFO"fmem: 0x%x not mapped in pmd 0x%x! \n", vaddr, (u32)pmdp);
		goto err;
	}

	if (!pmd_bad(*pmdp)) {
		//u32 v_addr;

		/* page mapping */
		ptep = pte_offset_kernel(pmdp, vaddr);
		if (pte_none(*ptep)) {
			printk(KERN_ERR"fmem: 0x%x not mapped in pte! \n", vaddr);
			goto err;
		}
#if 1   /* 2018/7/24 ¤U¤È 09:46:13 */
		paddr = (pte_pfn(*ptep) << PAGE_SHIFT) + (vaddr & (PAGE_SIZE - 1));
#else
		/* ifdef WANT_PAGE_VIRTUAL, then page->virtual has virtual address, NULL otherwise */
		v_addr = (unsigned int)page_address(pte_page(*ptep)) + (vaddr & (PAGE_SIZE - 1));
		paddr = __pa(v_addr);
#endif
    } else {
		/* section mapping. The cases are:
		* 1. DDR direct mapping
		* 2. ioremap's vaddr, size and paddr all are 2M alignment.
		*/
		if (vaddr & SECTION_SIZE)
			pmdp ++; /* move to pmd[1] */
		paddr = (pmd_val(*pmdp) & SECTION_MASK) | (vaddr & ~SECTION_MASK);
    }
err:
	return paddr;
}

static void dev_release(struct device *dev) { return; }
#define DRIVER_NAME "fmem_sync"
static struct platform_device pseudo_dev = {
	.name = DRIVER_NAME,
	.id   = 0,
	.num_resources = 0,
	.dev  = {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.release = dev_release,
	}
};

/* @this function is a data cache operation function,
 * @parm: vaddr: any virtual address
 * @parm: dir will be:
 * DMA_BIDIRECTIONAL = 0, it means flush operation.
 * DMA_TO_DEVICE = 1, it means clean operation.
 * DMA_FROM_DEVICE = 2, it means invalidate operation.
 * DMA_NONE = 3,
 */
void fmem_dcache_sync(void *vaddr, u32 len, enum dma_data_direction dir)
{
	struct device *dev = &pseudo_dev.dev;

	if (!valid_dma_direction(dir))
		panic("%s, invalid direction: %d \n", __func__, dir);

	/* kernel buffer may not cache line alignment, it only can use both clean/inv
	* for safe. Others we regard them as warning cases in coding.
	*/
	if (dir != DMA_BIDIRECTIONAL) {
		if ((u32)vaddr & CACHE_ALIGN_MASK) {
			va_not_64align ++;
			if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_VA)) {
				printk("%s, warning, vaddr: 0x%x not cache alignment! \n", __func__, (u32)vaddr);
				dump_stack();
			}
		}

		if (len & CACHE_ALIGN_MASK) {
			length_not_64align ++;
			if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_LEN)) {
				printk("%s, warning, len: %d not cache alignment! \n", __func__, len);
				dump_stack();
			}
		}
	}

	if (virt_addr_valid(vaddr) && virt_addr_valid(vaddr + len - 1)) {
		/* Notice:
		* The following code only for the direct mapping VA
		*/
		switch (dir) {
		case DMA_BIDIRECTIONAL:
			dma_map_single(dev, vaddr, len, DMA_TO_DEVICE);
			/* v7_dma_unmap_area is only valid for DMA_FROM_DEVICE */
			dma_unmap_single(dev, __pa(vaddr), len, DMA_FROM_DEVICE);
			break;
		case DMA_TO_DEVICE:
			dma_map_single(dev, vaddr, len, DMA_TO_DEVICE);
			break;
		case DMA_FROM_DEVICE:
			/* v7_dma_unmap_area is only valid for DMA_FROM_DEVICE */
			dma_unmap_single(dev, __pa(vaddr), len, DMA_FROM_DEVICE);
			break;
		case DMA_NONE:
			break;
		}
		cpuaddr_flush ++;
	} else {
		/* Notice:
		* We must process outer cache if have. The code is not implement yet!
		*/
#ifdef CONFIG_OUTER_CACHE
#error "Fmem: please implement outer cache!"
#endif
		switch (dir) {
		case DMA_BIDIRECTIONAL:
			dmac_flush_range(vaddr, vaddr + len);
			break;
		case DMA_TO_DEVICE:
			dmac_map_area(vaddr, len, DMA_TO_DEVICE);
			dmac_unmap_area(vaddr, len, DMA_TO_DEVICE);
			break;
		case DMA_FROM_DEVICE:
			dmac_map_area(vaddr, len, DMA_FROM_DEVICE);
			dmac_unmap_area(vaddr, len, DMA_FROM_DEVICE);
			break;
		case DMA_NONE:
			break;
		}
		nonlinear_cpuaddr_flush ++;
		if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_API)) {
			printk("%s, warning, memory addr 0x%x not direct mapped! \n", __func__, (u32)vaddr);
			dump_stack();
		}
	}
}

void fmem_dcache_sync_vb(void *vaddr, u32 len, enum dma_data_direction dir)
{
	struct device *dev = &pseudo_dev.dev;

	if (0 == vb_cache_flush_en){
		return;
	}
	if (!valid_dma_direction(dir))
		panic("%s, invalid direction: %d \n", __func__, dir);

	/* kernel buffer may not cache line alignment, it only can use both clean/inv
	* for safe. Others we regard them as warning cases in coding.
	*/
	if (dir != DMA_BIDIRECTIONAL) {
		if ((u32)vaddr & CACHE_ALIGN_MASK) {
			va_not_64align ++;
			if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_VA)) {
				printk("%s, warning, vaddr: 0x%x not cache alignment! \n", __func__, (u32)vaddr);
				dump_stack();
			}
		}

		if (len & CACHE_ALIGN_MASK) {
			length_not_64align ++;
			if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_LEN)) {
				printk("%s, warning, len: %d not cache alignment! \n", __func__, len);
				dump_stack();
			}
		}
	}

	if (virt_addr_valid(vaddr) && virt_addr_valid(vaddr + len - 1)) {
		/* Notice:
		* The following code only for the direct mapping VA
		*/
		switch (dir) {
		case DMA_BIDIRECTIONAL:
			dma_map_single(dev, vaddr, len, DMA_TO_DEVICE);
			/* v7_dma_unmap_area is only valid for DMA_FROM_DEVICE */
			dma_unmap_single(dev, __pa(vaddr), len, DMA_FROM_DEVICE);
			break;
		case DMA_TO_DEVICE:
			dma_map_single(dev, vaddr, len, DMA_TO_DEVICE);
			break;
		case DMA_FROM_DEVICE:
			/* v7_dma_unmap_area is only valid for DMA_FROM_DEVICE */
			dma_unmap_single(dev, __pa(vaddr), len, DMA_FROM_DEVICE);
			break;
		case DMA_NONE:
			break;
		}
		cpuaddr_flush ++;
	} else {
		/* Notice:
		* We must process outer cache if have. The code is not implement yet!
		*/
#ifdef CONFIG_OUTER_CACHE
#error "Fmem: please implement outer cache!"
#endif
		switch (dir) {
		case DMA_BIDIRECTIONAL:
			dmac_flush_range(vaddr, vaddr + len);
			break;
		case DMA_TO_DEVICE:
			dmac_map_area(vaddr, len, DMA_TO_DEVICE);
			dmac_unmap_area(vaddr, len, DMA_TO_DEVICE);
			break;
		case DMA_FROM_DEVICE:
			dmac_map_area(vaddr, len, DMA_FROM_DEVICE);
			dmac_unmap_area(vaddr, len, DMA_FROM_DEVICE);
			break;
		case DMA_NONE:
			break;
		}

		nonlinear_cpuaddr_flush ++;
		if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_API)) {
			printk("%s, warning, memory addr 0x%x not direct mapped! \n", __func__, (u32)vaddr);
			dump_stack();
		}
    }
}

static void __copy_bankinfo(fmeminfo_t *meminfo, fmem_mtype_t mtype)
{
	int	i, j;

	for (i = 0; i < bank_info.nr_banks; i ++) {
		if (bank_phys_mtype(&bank_info.bank[i]) != mtype)
			continue;
		j = meminfo->nr_banks;
		memcpy(&meminfo->bank[j], &bank_info.bank[i], sizeof(bank_info.bank[0]));
		meminfo->nr_banks ++;
	}
	return;
}

/* @this function gets bank information and size
 * @meminfo: memory related information
 * Please notice:
 *  bank_info database would NOT be changed even the hpg action.
 */
void fmem_get_bankinfo(fmeminfo_t *meminfo)
{
	memset(meminfo, 0, sizeof(fmeminfo_t));

	down(&sema);
	/* cma */
	__copy_bankinfo(meminfo, FMEM_MTYPE_MEM);

	up(&sema);

	return;
}

EXPORT_SYMBOL(fmem_alloc_from_cma);
EXPORT_SYMBOL(fmem_release_from_cma);
EXPORT_SYMBOL(fmem_get_cmadev);
EXPORT_SYMBOL(fmem_lookup_pa);
EXPORT_SYMBOL(fmem_dcache_sync);
EXPORT_SYMBOL(fmem_dcache_sync_vb);
EXPORT_SYMBOL(fmem_get_bankinfo);
