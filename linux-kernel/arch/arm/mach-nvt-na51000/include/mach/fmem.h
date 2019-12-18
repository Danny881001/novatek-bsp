#ifndef __FMEM_H
#define __FMEM_H

#include <linux/mm.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <linux/list.h>
#include <linux/dma-direction.h>
#include <linux/memblock.h>

#define MAX_DDR_CHUNKS  2

/* don't change the order and value */
typedef enum {
    FMEM_MTYPE_MEM = 0, /* kernel CMA memory */
    FMEM_MTYPE_OTHER,
    FMEM_MTYPE_MAX,
} fmem_mtype_t;

typedef struct fmeminfo_s {
	int nr_banks;
	struct membank {
		int	chipid;	/* start from 0 */
		int	ddrid;	/* start from 0 */
	    fmem_mtype_t    mtype;
	    phys_addr_t     start;
	    unsigned int    size;
	} bank[MAX_DDR_CHUNKS];
} fmeminfo_t;

/* entry function from board level */
void fmem_reserve_memory(void);

/*
 * mem_fixup_cb: used to fixup the memory in board level. Currently it is only for PCIe
 * rsv_pbase: physical base to reserve.
 * rsv_sz: size to reserve. 0 means no reserve.
 */
void fmem_fixup_memory(void (*fixup_mem)(fmeminfo_t *meminfo), u32 rsv_pbase, u32 rsv_sz);

/**
 * @brief to resolve the virtual address (including direct mapping, ioremap or user space address to
 *      its real physical address.
 *
 * @parm vaddr indicates any virtual address
 *
 * @return  >= 0 for success, 0xFFFFFFFF for fail
 */
phys_addr_t fmem_lookup_pa(unsigned int vaddr);

/* @this function is a data cache operation function,
 * @parm: vaddr: any virtual address
 * @parm: dir will be:
 * DMA_BIDIRECTIONAL = 0, it means flush operation.
 * DMA_TO_DEVICE = 1, it means clean operation.
 * DMA_FROM_DEVICE = 2, it means invalidate operation.
 * DMA_NONE = 3,
 */
void fmem_dcache_sync(void *vaddr, u32 len, enum dma_data_direction dir);

/* @this function is a video buffer data cache operation function,
 *       default will ignore the cache flush operation
 * @parm: vaddr: any virtual address
 * @parm: dir will be:
 * DMA_BIDIRECTIONAL = 0, it means flush operation.
 * DMA_TO_DEVICE = 1, it means clean operation.
 * DMA_FROM_DEVICE = 2, it means invalidate operation.
 * DMA_NONE = 3,
 */
void fmem_dcache_sync_vb(void *vaddr, u32 len, enum dma_data_direction dir);

/* @this function gets memory information and size
 * @meminfo: memory related information
 * Please notice:
 *  mem_info database would be changed base on the hpg action.
 */
int fmem_get_meminfo(fmeminfo_t *meminfo);

/* @this function gets bank information and size
 * @meminfo: memory related information
 * Please notice:
 *  bank_info database would NOT be changed even the hpg action.
 */
void fmem_get_bankinfo(fmeminfo_t *meminfo);

/* @this function used to get cma device structure
 * @param: dev: which cma region.
 * return: NULL for fail, otherwise for success.
 */
void *fmem_get_cmadev(int dev);

/* @this function used to allocate memory from cma region.
 * @ 	Actually it call dma_alloc_from_contiguous() in kernel.
 * @param: cma_dev_idx: which cma resion. Currently only 0 is supported.
 * return: virtual address. NULL for fail, otherwise for success.
 * Note: It is only cachable.
 */
void *fmem_alloc_from_cma(int cma_region_idx, int page_count, phys_addr_t *paddr);

/* @this function used to release memory to cma region.
 * @ 	Actually it call dma_release_from_contiguous() in kernel
 * @param: cma_dev_idx: which cma resion. Currently only 0 is supported.
 *		   vaddr: virtual address, page_count: how many pages to release
 * return: 0 for success, < 0 for fail
 */
int fmem_release_from_cma(int cma_region_idx, void *vaddr, int page_count);

/* @this function used to release memory to dsp cma region.
 * @ 	Actually it call dma_release_from_contiguous() in kernel
 * @param:
 *	  vaddr: virtual address, page_count: how many pages to release
 *	  page_count: allocate size
 *	  index: dsp cma area index (0 or 1)
 * return: 0 for success, < 0 for fail
 */
int fmem_release_from_dspcma(void *vaddr, int page_count, unsigned int index);

/* @this function used to release memory to dsp cma region.
 * @ 	Actually it call dma_release_from_contiguous() in kernel
 * @param:
 *	   page_count: how many pages to release, paddr: returned physical address
 *	   paddr: Physical address
 *	   index: dsp cma area index (0 or 1)
 * return: 0 for success, < 0 for fail
 */
void *fmem_alloc_from_dspcma(int page_count, phys_addr_t *paddr, unsigned int index);

#endif /* __FMEM_H */
