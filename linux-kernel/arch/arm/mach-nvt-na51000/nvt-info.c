/**
    NVT info for the platform related tools
    This file will provide the boot stage record the timestamp function
    @file       nvt-info.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/gfp.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/memblock.h>
#include <generated/nvtversion.h>

#include <mach/nvt-bootts.h>
#include <mach/hardware.h>
#include <mach/nvt-io.h>

#define NVT_BOOTTS_MAX_LEN          16
#define NVT_BOOTTS_NAME_MAX_LEN     16
#define NVT_BOOTTS_UBOOT_RESV_CNT   2
#define LOADER_TIME_OFFSET          0

#define NVT_TIMER_TM0_CNT       0x108
#define TM0_LIST_ONLY

struct bootts {
	u32 time;
	char name[NVT_BOOTTS_NAME_MAX_LEN];
	int column;
	int diff_time; //diff time to the same name
};

static struct bootts g_bootts[NVT_BOOTTS_MAX_LEN];
static int g_index = 0;
static unsigned long *watchpoint_addr_virt = NULL;
struct proc_dir_entry   *nvt_info_dir_root = NULL;
EXPORT_SYMBOL(nvt_info_dir_root);

void nvt_bootts_add_ts(char *name)
{
	unsigned char name_len = strlen(name);

	if (name_len >= NVT_BOOTTS_NAME_MAX_LEN || g_index >= NVT_BOOTTS_MAX_LEN) {
		pr_err("\n%s %s fail\n", __func__, name);
		return;
	}

	if (name != NULL)
		strncpy(g_bootts[g_index].name, name, name_len);

	g_bootts[g_index].time = nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT);
	g_index++;
}

EXPORT_SYMBOL_GPL(nvt_bootts_add_ts);

#ifdef CONFIG_PROC_FS
/* Support for	/proc/nvt_info/bootts */
/*		/proc/nvt_info/tm0 */
/*		/proc/nvt_info/memperf */

static void add_uboot_time(void)
{
	char *pstr = NULL;
	char *psep = NULL;
	char symbol;
	unsigned long uboot_ts = 0;
	int bootts_idx = 0;

	pstr = strstr(saved_command_line, "bootts=");
	if (pstr) {
		pstr += strlen("bootts=");
	}
	while (pstr) {
		psep = strpbrk(pstr, ", "); //find ',' or ' ', or '\0'
		if (NULL == psep) {
			break;
		}

		symbol = *psep;
		*psep = '\0';
		uboot_ts = 0;
		kstrtoul(pstr, 10, &uboot_ts);
		*psep = symbol;

		if (bootts_idx < NVT_BOOTTS_UBOOT_RESV_CNT) {
			strncpy(g_bootts[bootts_idx].name, "uboot", sizeof(g_bootts[bootts_idx].name)-1);
			g_bootts[bootts_idx].time = uboot_ts;
			bootts_idx++;
		}

		if (' ' == symbol) {
			break;
		}

		pstr = psep + 1;
	}
}

#ifndef TM0_LIST_ONLY
static void bootts_dump_linux_tm0(struct seq_file *m)
{
	int i;

	add_uboot_time();

	seq_puts(m, "Name\t\tDiff(us)\tTM0\n");
	for (i = 0; i < g_index; i++) {
		seq_printf(m, "%-8.8s\t%-8.8u\t%-8.8u\n",
				g_bootts[i].name,
				(i-1 < 0) ? (g_bootts[i].time - 0) :
				(g_bootts[i].time - g_bootts[i-1].time),
				g_bootts[i].time
				);
	}
	seq_printf(m, "%-8.8s\t%-8.8u\t\n",
			"Total",
			(g_bootts[g_index - 1].time - g_bootts[0].time)
			);
}
#else //TM0_LIST_ONLY
static void bootts_dump_linux(struct seq_file *m)
{
	int item_idx, item_idx2;
	int column_cur, column_max;
	int row_idx;

	if (g_index < 1) {
		seq_printf(m, "no data");
		return;
	}

	add_uboot_time();

	//reset the column to recount the newest column
	for (item_idx = 0; item_idx < g_index; item_idx++) {
		g_bootts[item_idx].column = 0;
	}

	//setup the column of each item
	column_cur = 0;
	for (item_idx = 0; item_idx < g_index; item_idx++) {
		if (0 != g_bootts[item_idx].column) {
			continue;
		}
		column_cur++;
		g_bootts[item_idx].column = column_cur;
		for (item_idx2 = item_idx; item_idx2 < g_index; item_idx2++) {
			if (!strcmp(g_bootts[item_idx2].name, g_bootts[item_idx].name)) {
				g_bootts[item_idx2].column = column_cur;
				g_bootts[item_idx2].diff_time = g_bootts[item_idx2].time - g_bootts[item_idx].time;
			}
		}
	}
	column_max = column_cur;

#if 0 //for debug
	printk("g_index %d, column_max %d\r\n", g_index, column_max);
	for (item_idx = 0; item_idx < g_index; item_idx++) {
		printk("item[%d], column %d, %u, %s\r\n",
			item_idx,
			g_bootts[item_idx].column,
			g_bootts[item_idx].time,
			g_bootts[item_idx].name);
	}
#endif

	//print column name
	seq_printf(m, "Name");
	for (column_cur = 1; column_cur <= column_max; column_cur++) {
		for (item_idx = 0; item_idx < g_index; item_idx++) {
			if (column_cur == g_bootts[item_idx].column) {
				seq_printf(m, "%12s", g_bootts[item_idx].name);
				break;
			}
		}
	}
	seq_printf(m, "\n");

	//print item data by column
	for (row_idx = 0; row_idx < g_index; row_idx++) {
		seq_printf(m, "[%02d]", row_idx);
		for (column_cur = 1; column_cur <= column_max; column_cur++) {
			if (column_cur == g_bootts[row_idx].column) {
				seq_printf(m, "%12u", g_bootts[row_idx].time + LOADER_TIME_OFFSET);
			} else {
				seq_printf(m, "%12c", '-');
			}
		}
		seq_printf(m, "\n");
	}

	//print diff time
	seq_puts(m, "-----------------------------------------------\n");
	seq_printf(m, "Diff");
	for (column_cur = 1; column_cur <= column_max; column_cur++) {
		for (item_idx = 0; item_idx < g_index; item_idx++) {
			if (column_cur == g_bootts[item_idx].column &&
				0 != g_bootts[item_idx].diff_time) {
				seq_printf(m, "%12u", g_bootts[item_idx].diff_time);
				break;
			}
		}
	}
	seq_printf(m, "\n");
}
#endif //TM0_LIST_ONLY

static int nvt_bootts_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "============== Boot time results ==============\n");
	/* seq_printf(m, "TM0 initial value: %llu\n", g_bootts.time[0]); */
#ifndef TM0_LIST_ONLY
	bootts_dump_linux_tm0(m);
#else
	bootts_dump_linux(m);
#endif
	seq_puts(m, "================================================\n");
	return 0;
}


static int nvt_bootts_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_bootts_proc_show, NULL);
}

static ssize_t nvt_bootts_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	int res = 0;

	if (!buf)
		return -ENOMEM;

	res = -EINVAL;
	if (count >= NVT_BOOTTS_NAME_MAX_LEN)
		goto out;

	res = -EFAULT;
	if (copy_from_user(buf, buffer, count))
		goto out;

	buf[count-1] = '\0';

	nvt_bootts_add_ts(buf);
	res = count;
out:
	free_page((unsigned long)buf);
	return res;
}
static const struct file_operations nvt_bootts_fops = {
	.open = nvt_bootts_open,
	.read = seq_read,
	.write = nvt_bootts_write,
};

static int nvt_timer_tm0_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%u\n", nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT));

	return 0;
}

static int nvt_timer_tm0_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_timer_tm0_proc_show, NULL);
}

static const struct file_operations nvt_timer_tm0_fops = {
	.open = nvt_timer_tm0_open,
	.read = seq_read,
	.llseek = seq_lseek,
};

static unsigned long nvt_test_memcpy_perf(size_t bufsize, unsigned long iters)
{
	unsigned int i = 0;
	char *ptr_buf1 = NULL;
	char *ptr_buf2 = NULL;
	unsigned long time_before = 0;
	unsigned long time_after = 0;

	ptr_buf1 = (char*)kzalloc(bufsize, GFP_KERNEL);
	ptr_buf2 = (char*)kzalloc(bufsize, GFP_KERNEL);

	time_before = nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT);
	for (i = 0; i < iters; ++i) {
		memcpy(ptr_buf1, ptr_buf2, bufsize);
	}
	time_after = nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT);
	kfree(ptr_buf1);
	kfree(ptr_buf2);
	return ((bufsize * iters) / (time_after - time_before));
}

static unsigned long nvt_test_memset_perf(size_t bufsize, unsigned long iters)
{
	unsigned int i = 0;
	char *ptr_buf1 = NULL;
	unsigned long time_before = 0;
	unsigned long time_after = 0;

	ptr_buf1 = (char*)kzalloc(bufsize, GFP_KERNEL);

	time_before = nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT);
	for (i = 0; i < iters; ++i) {
		memset(ptr_buf1, i, bufsize);
	}
	time_after = nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT);
	kfree(ptr_buf1);
	return ((bufsize * iters) / (time_after - time_before));
}

static unsigned long nvt_test_memcmp_perf(size_t bufsize, unsigned long iters)
{
	unsigned int i = 0;
	char *ptr_buf1 = NULL;
	char *ptr_buf2 = NULL;
	unsigned long time_before = 0;
	unsigned long time_after = 0;
	int ret = 0;
	
	ptr_buf1 = (char*)kzalloc(bufsize, GFP_KERNEL);
	ptr_buf2 = (char*)kzalloc(bufsize, GFP_KERNEL);
	memset(ptr_buf1, 0x55aa55aa, bufsize);
	memset(ptr_buf2, 0x55aa55aa, bufsize);

	time_before = nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT);
	for (i = 0; i < iters; ++i) {
		ret = memcmp(ptr_buf1, ptr_buf2, bufsize);
		if (ret != 0)
			break;
	}
	time_after = nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT);
	if (ret == 0)
		pr_info("Compare result: Same\n");
	else
		pr_info("Compare result: Different\n");

	kfree(ptr_buf1);
	kfree(ptr_buf2);
	return ((bufsize * iters) / (time_after - time_before));
}

static int nvt_memperf_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "Memory performance testing results\n");
	seq_printf(m, "memcpy: %lu MB/Sec.\n", nvt_test_memcpy_perf(2000000, 1000));
	seq_printf(m, "memset: %lu MB/Sec.\n", nvt_test_memset_perf(2000000, 1000));
	seq_printf(m, "memcmp: %lu MB/Sec.\n", nvt_test_memcmp_perf(2000000, 1000));

	return 0;
}

static int nvt_memperf_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_memperf_proc_show, NULL);
}

static const struct file_operations nvt_memperf_fops = {
	.open = nvt_memperf_open,
	.read = seq_read,
	.llseek = seq_lseek,
};

static ssize_t nvt_memhotplug_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	char* const delim = "@";
	char *token = NULL;
	unsigned long mem_address = 0;
	unsigned long mem_size = 0;
	int res = 0;

	if (!buf)
		return -ENOMEM;

	res = -EFAULT;
	if (copy_from_user(buf, buffer, count))
		goto out;

	res = -EFAULT;
	while ((token = strsep(&buf, delim)) != NULL) {
		if (mem_size == 0) {
			if (kstrtoul(token, 16, &mem_size) < 0)
				goto out;
		} else {
			if (kstrtoul(token, 16, &mem_address) < 0)
				goto out;
			break;
		}
	}

	res = memblock_add(mem_address, mem_size);
	if (res != 0) {
		pr_info("Got failures during add memory region 0x%08lx@0x%08lx\n", mem_size, mem_address);
		res = -1;
	} else {
		pr_info("Add memory region 0x%08lx@0x%08lx\n", mem_size, mem_address);
		res = count;
	}

out:
	free_page((unsigned long)buf);
	return res;
}

static int nvt_memhotplug_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "Memory hotplug usage\n");
	seq_puts(m, "\tAdd memory region:\n");
	seq_puts(m, "\t$ echo Phys_size@Phys_addr > /proc/nvt_info/memhotplug\n");
	seq_puts(m, "\tPrint memory region:\n");
	seq_puts(m, "\t$ cat /sys/kernel/debug/memblock/memory\n");

	return 0;
}

static int nvt_memhotplug_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_memhotplug_proc_show, NULL);
}

static const struct file_operations nvt_memhotplug_fops = {
	.open = nvt_memhotplug_open,
	.read = seq_read,
	.write = nvt_memhotplug_write,
};

static ssize_t nvt_watchpoint_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	char* const delim = "@";
	char *token = NULL;
	unsigned long test_address = 0;
	unsigned long test_val = 0;
	int res = 0;

	if (!buf)
		return -ENOMEM;

	res = -EFAULT;
	if (copy_from_user(buf, buffer, count))
		goto out;

	res = -EFAULT;
	while ((token = strsep(&buf, delim)) != NULL) {
		if (test_val == 0) {
			if (kstrtoul(token, 16, &test_val) < 0)
				goto out;
		} else {
			if (kstrtoul(token, 16, &test_address) < 0)
				goto out;
			break;
		}
	}

	if (watchpoint_addr_virt != NULL) {
		*watchpoint_addr_virt = test_val;
		pr_info("Write value:%lx to address 0x%p\n", *watchpoint_addr_virt, watchpoint_addr_virt);
	}

	res = count;

out:
	free_page((unsigned long)buf);
	return res;
}

static int nvt_watchpoint_proc_show(struct seq_file *m, void *v)
{
	if (watchpoint_addr_virt == NULL) {
		watchpoint_addr_virt = kzalloc(8, GFP_KERNEL);
		if (!watchpoint_addr_virt) {
			pr_err("failed to alloc memory for watchdog test\n");
			return -ENOMEM;
		}
	}

	seq_printf(m, "Watchdog test usage\n");
	seq_printf(m, "\tWrite logical address(0x%p) with value(0x55aa):\n", watchpoint_addr_virt);
	seq_printf(m, "\t$ echo 0x55aa@0x%p > /proc/nvt_info/watchpoint_test\n", watchpoint_addr_virt);
	seq_printf(m, "\t$ Current addr: %p value: %lx\n", watchpoint_addr_virt, *watchpoint_addr_virt);
	seq_printf(m, "\tTo catch value change event: $ modprobe nvt_data_breakpoint wp_addr=0x%p wp_chk_val=0x%08lx\n", watchpoint_addr_virt, *watchpoint_addr_virt);
	seq_printf(m, "\tTo catch address access event: $ modprobe nvt_data_breakpoint wp_addr=0x%p\n", watchpoint_addr_virt);

	return 0;
}

static int nvt_watchpoint_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_watchpoint_proc_show, NULL);
}

static const struct file_operations nvt_watchpoint_fops = {
	.open = nvt_watchpoint_open,
	.read = seq_read,
	.write = nvt_watchpoint_write,
};

static int nvt_version_show(struct seq_file *m, void *v)
{
	seq_printf(m, "Version: %s \n", NVT_UTS_RELEASE);
	return 0;
}

static int nvt_version_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_version_show, NULL);
}

static const struct file_operations nvt_version_fops = {
	.open = nvt_version_open,
	.read = seq_read,
};
#endif

static int nvt_bootts_init(void)
{
	memset(g_bootts, 0, sizeof(g_bootts));
	g_index = NVT_BOOTTS_UBOOT_RESV_CNT;

	nvt_bootts_add_ts("ker");

	pr_info("NVTBOOTTS: %s initial success\n", __func__);

	return 0;
}

#ifdef CONFIG_PROC_FS
static int __init nvt_bootts_proc_init(void)
{
	struct proc_dir_entry *entry = NULL;

	nvt_info_dir_root = proc_mkdir("nvt_info", NULL);
	if (!nvt_info_dir_root)
		return -ENOMEM;

	entry = proc_create("bootts", 0664, nvt_info_dir_root, &nvt_bootts_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("tm0", 0664, nvt_info_dir_root, &nvt_timer_tm0_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("memperf", 0664, nvt_info_dir_root, &nvt_memperf_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("memhotplug", 0664, nvt_info_dir_root, &nvt_memhotplug_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("watchpoint_test", 0664, nvt_info_dir_root, &nvt_watchpoint_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("version", 0664, nvt_info_dir_root, &nvt_version_fops);
	if (!entry)
		return -ENOMEM;

	nvt_bootts_init();
	pr_info("NVTBOOTTS: %s initial success\n", __func__);

	return 0;
}

core_initcall(nvt_bootts_proc_init);
#endif
