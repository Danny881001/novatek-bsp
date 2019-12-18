/*
 * drivers/mtd/spiflash/spinand.c
 *
 * Copyright © 2005 Intel Corporation
 * Copyright © 2006 Marvell International Ltd.
 * Copyright © 2016 Novatek Microelectronics Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/interrupt.h>

#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/mtd/mtd.h>
#include <linux/platform_device.h>
#include <linux/mtd/partitions.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/crc32.h>
#include <linux/sizes.h>
#include <mach/nvt-sramctl.h>
#include <mach/pad.h>

#include "../nvt_flash_spi/nvt_flash_spi_reg.h"
#include "../nvt_flash_spi/nvt_flash_spi_int.h"

#ifdef CONFIG_OF
#include <linux/of_device.h>
#endif

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <asm/div64.h>
#include "../mtdcore.h"

static int enable_erase_cnt = 0;
static u32 erase_count[4096] = {};
static u32 total_block_cnt = 0;

#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

typedef struct proc_nand {
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_static_entry;
	struct proc_dir_entry *pproc_switch_entry;
	struct proc_dir_entry *pproc_mtd_ec_info_entry;
	struct proc_dir_entry *pproc_peb_mtd_ec_info_entry;
	struct proc_dir_entry *pproc_flash_status_entry;
	struct proc_dir_entry *pproc_readback_entry;
} proc_nand_static_t;
proc_nand_static_t proc_nand;

static u32 readback = 0;
static ssize_t nvt_nand_proc_readback_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned long config = 0x0;

	/*check command length*/
	if ((!len) || (len > (MAX_CMD_LENGTH - 1))) {
		pr_err("Command length is too long or 0!\n");
		goto ERR_OUT;
	}

	/*copy command string from user space*/
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (ucargc != 1) {
		pr_err("wrong argument:%d", ucargc);
		return -EINVAL;
	}

	if (kstrtoul(argv[0], 0, &config)) {
		pr_err("invalid config:%s\n", argv[0]);
		return -EINVAL;
	} else {
		readback = config;
		return size;
	}

	
ERR_OUT:
	return -1;
}

static int nvt_nand_proc_readback_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "0x%x\n", readback);
	return 0;
}


static int nvt_nand_proc_readback_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_readback_show, NULL);
}


static struct file_operations proc_readback_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_nand_proc_readback_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
	.write   = nvt_nand_proc_readback_write
};

struct drv_nand_dev_info *ginfo = NULL;
static ssize_t nvt_nand_proc_flash_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned long config = 0x0;

	/*check command length*/
	if ((!len) || (len > (MAX_CMD_LENGTH - 1))) {
		pr_err("Command length is too long or 0!\n");
		goto ERR_OUT;
	}

	/*copy command string from user space*/
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (ucargc != 1) {
		pr_err("wrong argument:%d", ucargc);
		return -EINVAL;
	}

	if (kstrtoul(argv[0], 0, &config)) {
		pr_err("invalid config:%s\n", argv[0]);
		return -EINVAL;
	} else {
		mutex_lock(&ginfo->lock);
		nand_cmd_write_status(ginfo, NAND_SPI_STS_FEATURE_2, config);
		mutex_unlock(&ginfo->lock);
		return size;
	}

	
ERR_OUT:
	return -1;
}

static int nvt_nand_proc_flash_status_show(struct seq_file *sfile, void *v)
{
	u8 status = 0x0;

	mutex_lock(&ginfo->lock);
	status = nand_cmd_read_status(ginfo, NAND_SPI_STS_FEATURE_2);
	mutex_unlock(&ginfo->lock);

	seq_printf(sfile, "0x%x\n", status);

	return 0;
}


static int nvt_nand_proc_flash_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_flash_status_show, NULL);
}


static struct file_operations proc_flash_status_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_nand_proc_flash_status_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
	.write   = nvt_nand_proc_flash_write
};


static int nvt_nand_proc_mtd_peb_ec_info_show(struct seq_file *sfile, void *v)
{
	struct mtd_info *mtd;
	int i = 0;
	u64 block_cnt = 0, block_end = 0, block_begin = 0;

	mtd_for_each_device(mtd) {
		seq_printf(sfile, "mtd%d: %8.8llx %8.8llx \"%s\"\n",
			   mtd->index, block_begin, \
			   (unsigned long long)mtd->size + block_begin, mtd->name);

		block_begin += mtd->size;
		block_cnt = mtd->size;
		do_div(block_cnt, mtd->erasesize);

		block_end += block_cnt;

		for (;i < block_end; i++) {
			if (((i % 10) == 0) && (i != 0))
				seq_printf(sfile, "\n");

			seq_printf(sfile, "%-4d: %-6d", i, erase_count[i]);
		}
		seq_printf(sfile, "\n");
	}

	return 0;
}


static int nvt_nand_proc_mtd_peb_ec_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_mtd_peb_ec_info_show, NULL);
}


static struct file_operations proc_mtd_peb_ec_info_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_nand_proc_mtd_peb_ec_info_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};


static int nvt_nand_proc_mtd_ec_info_show(struct seq_file *sfile, void *v)
{
	struct mtd_info *mtd;
	int i = 0;
	u64 block_cnt = 0, block_end = 0, block_begin = 0;
	u32 total_cnt = 0;

	mtd_for_each_device(mtd) {
		seq_printf(sfile, "mtd%d: %8.8llx %8.8llx \"%s\"\n",
			   mtd->index, block_begin, \
			   (unsigned long long)mtd->size + block_begin, mtd->name);

		block_begin += mtd->size;
		block_cnt = mtd->size;
		do_div(block_cnt, mtd->erasesize);

		block_end += block_cnt;

		for (;i < block_end; i++) {
			total_cnt += erase_count[i];
		}
		seq_printf(sfile, "Total erase count %d\n", total_cnt);
		total_cnt = 0;
	}

	return 0;
}


static int nvt_nand_proc_mtd_ec_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_mtd_ec_info_show, NULL);
}


static struct file_operations proc_mtd_ec_info_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_nand_proc_mtd_ec_info_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};


static int nvt_nand_proc_static_show(struct seq_file *sfile, void *v)
{
	int i;

	for (i = 0; i < total_block_cnt; i++) {
		if ((i % 10) == 0)
			seq_printf(sfile, "\n");
		seq_printf(sfile, "%-4d: %-6d", i, erase_count[i]);
	}
	seq_printf(sfile, "\n");
	return 0;
}


static int nvt_nand_proc_static_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_static_show, NULL);
}


static struct file_operations proc_static_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_nand_proc_static_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};

static int nvt_nand_proc_switch_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\nUsage\n");
	seq_printf(sfile, "\necho $command > enable_static\n\n");
	seq_printf(sfile, "enable           : enable erase static\n");
	seq_printf(sfile, "disable          : disable erase static\n");
	seq_printf(sfile, "clean            : clean static data\n");
	return 0;
}


static int nvt_nand_proc_switch_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_switch_show, NULL);
}

static ssize_t nvt_nand_proc_switch_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

	/*check command length*/
	if ((!len) || (len > (MAX_CMD_LENGTH - 1))) {
		pr_err("Command length is too long or 0!\n");
		goto ERR_OUT;
	}

	/*copy command string from user space*/
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (!strcmp(argv[0], "enable")) {
		enable_erase_cnt = 1;
		return size;
	} else if (!strcmp(argv[0], "disable")) {
		enable_erase_cnt = 0;
		return size;
	} else if (!strcmp(argv[0], "clean")) {
		int i;
		for (i = 0; i < 4096; i++)
			erase_count[i] = 0x0;

		return size;
	}

ERR_OUT:
	return -1;
}

static struct file_operations proc_switch_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_nand_proc_switch_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
	.write   = nvt_nand_proc_switch_write
};

int nvt_nand_proc_init(void)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("nvt_info/nvt_flash", NULL);
	if (pmodule_root == NULL) {
		pr_err("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_module_root = pmodule_root;

	pentry = proc_create("static_info", S_IRUGO | S_IXUGO, pmodule_root, &proc_static_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc static!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_static_entry = pentry;

	pentry = proc_create("enable_static", S_IRUGO | S_IXUGO, pmodule_root, &proc_switch_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc static!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_switch_entry = pentry;

	pentry = proc_create("mtd_ec_info", S_IRUGO | S_IXUGO, pmodule_root, &proc_mtd_ec_info_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc static!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_mtd_ec_info_entry = pentry;

	pentry = proc_create("mtd_peb_ec_info", S_IRUGO | S_IXUGO, pmodule_root, &proc_mtd_peb_ec_info_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc static!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_peb_mtd_ec_info_entry = pentry;

	pentry = proc_create("flash_status", S_IRUGO | S_IXUGO, pmodule_root, &proc_flash_status_fops);
	if (pentry == NULL) {
		pr_err("failed to create flash status!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_flash_status_entry = pentry;

	pentry = proc_create("readback", S_IRUGO | S_IXUGO, pmodule_root, &proc_readback_fops);
	if (pentry == NULL) {
		pr_err("failed to create readback!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_readback_entry = pentry;

remove_proc:
	return ret;
}


/* error code and state */
enum {
	ERR_NONE	= 0,
	ERR_DMABUSERR	= -1,
	ERR_SENDCMD	= -2,
	ERR_DBERR	= -3,
	ERR_BBERR	= -4,
	ERR_ECC_FAIL	= -5,
	ERR_ECC_UNCLEAN = -6,
};

enum {
	STATE_READY = 0,
	STATE_CMD_HANDLE,
	STATE_DMA_READING,
	STATE_DMA_WRITING,
	STATE_DMA_DONE,
	STATE_PIO_READING,
	STATE_PIO_WRITING,
};

static struct nand_flash_dev spinand_flash_ids[] = {
/*
* Some incompatible NAND chips share device ID's and so must be
* listed by full ID. We list them first so that we can easily identify
* the most specific match.
*/
	// Micron
	{"MT29F1G01AAADD 1G 3.3V",
		{ .id = {MFG_ID_MICRON, 0x12} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// Micron
	{"MT29F1G01ABAF 1G 3.3V",
		{ .id = {MFG_ID_MICRON, 0x14} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// Winbond
	{"W25N01GV 1G 3.3V",
		{ .id = {MFG_ID_WINBOND, 0xAA, 0x21} },
		SZ_2K, SZ_128, SZ_128K, 0, 3, 64 },
	// ESMT
	{"F250L512M 512M 3.3V",
		{ .id = {MFG_ID_ESMT, 0x20} },
		SZ_2K, SZ_64, SZ_128K, 0, 2, 64 },
	// ESMT
	{"F250L512M 1GiB 3.3V",
		{ .id = {MFG_ID_ESMT, 0x01} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 128 },
	// GigaDevice
	{"GD5F1GQ4UBYIG 1GiB 3.3V",
		{ .id = {MFG_ID_GD, 0xD1} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 128 },
	// GigaDevice 2G
	{"GD5F2GQ4UB9IG 2GiB 3.3V",
		{ .id = {MFG_ID_GD, 0xD2} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 64 },
	// GigaDevice 2G
	{"GD5F2GQ5UEYIG 2GiB 3.3V",
		{ .id = {MFG_ID_GD, 0x52} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 64 },
	// MXIC
	{"MX35LF1GE4AB 1GiB 3.3V",
		{ .id = {MFG_ID_MXIC, 0x12} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// MXIC
	{"MX35LF2G14AC 2GiB 3.3V",
		{ .id = {MFG_ID_MXIC, 0x20} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 64 },
	// ETRON
	{"EM73C044 1GiB 3.3V",
		{ .id = {MFG_ID_ETRON, 0x11} },
		SZ_2K, SZ_128, SZ_64K, 0, 2, 64 },
	// ATO
	{"ATO25D1GA 1GiB 3.3V",
		{ .id = {MFG_ID_ATO, 0x12} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// TOSHIBA
	{"TC58CVG0S3Hx 1GiB 3.3V",
		{ .id = {MFG_ID_TOSHIBA, 0xC2} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// TOSHIBA
	{"TC58CVG1S3Hx 2GiB 3.3V",
		{ .id = {MFG_ID_TOSHIBA, 0xEB, 0x40} },
		SZ_2K, SZ_256, SZ_128K, 0, 3, 64 },
	// Doscilicon
	{"DS35Q1GA 1GiB 3.3V",
		{ .id = {MFG_ID_DOSILICON, 0x71} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
};

#ifdef CONFIG_MTD_SPINAND_HWECC
static struct nand_ecclayout spinand_oob_64 = {
	.eccbytes = 32,
	.eccpos = {
		8, 9, 10, 11, 12, 13, 14, 15,
		24, 25, 26, 27, 28, 29, 30, 31,
		40, 41, 42, 43, 44, 45, 46, 47,
		56, 57, 58, 59, 60, 61, 62, 63},
	.oobavail = 12,
	.oobfree = {
		{.offset = 16,
			.length = 4},
		{.offset = 32,
			.length = 4},
		{.offset = 48,
			.length = 4},
	}
};

static struct nand_ecclayout spinand_oob_etron = {
        .eccbytes = 60,
        .eccpos = {
		4, 5, 6, 7,
                8, 9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23,
                24, 25, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39,
                40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
                56, 57, 58, 59, 60, 61, 62, 63},
        .oobavail = 2,
        .oobfree = {
                {.offset = 2,
                        .length = 2},
        }
};

static struct nand_ecclayout spinand_oob_gd = {
#ifdef CONFIG_JFFS2_FS
	.eccbytes = 64,
	.eccpos = {
		64, 65, 66, 67, 68, 69, 70, 71,
		72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 87,
		88, 89, 90, 91, 92, 93, 94, 95,
		96, 97, 98, 99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119,
		120, 121, 122, 123, 124, 125, 126, 127},
	.oobavail = 12,
	.oobfree = {
		{.offset = 16, .length = 4},
		{.offset = 32, .length = 4},
		{.offset = 48, .length = 4},
	}
#else
	.oobavail = 62,
	.oobfree = {
		{.offset = 2, .length = 62},
	}
#endif
};
#endif


static int use_dma = 1;

static int drv_nand_reset(struct drv_nand_dev_info *info, struct platform_device *pdev)
{
	int ret = 0;

	info->clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (!IS_ERR(info->clk)) {
		clk_prepare(info->clk);
		clk_enable(info->clk);
	} else {
		pr_err("%s: %s not found\n", __func__, dev_name(&pdev->dev));
		return -ENXIO;
	}

	NAND_SETREG(info, NAND_TIME0_REG_OFS, 0x06002222);
	NAND_SETREG(info, NAND_TIME1_REG_OFS, 0x7f0f);

	clk_set_rate(info->clk, info->flash_freq);

	/* Release SRAM */
	nvt_disable_sram_shutdown(NAND_SD);

	pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_10MA);
	pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_10MA);
	pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_10MA);
	pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_10MA);
	pad_set_drivingsink(PAD_DS_CGPIO8, PAD_DRIVINGSINK_10MA);
	pad_set_drivingsink(PAD_DS_CGPIO9, PAD_DRIVINGSINK_16MA);

	nand_host_set_nand_type(info, NANDCTRL_SPI_NAND_TYPE);

	nand_host_settiming2(info, 0x9F51);

	ret = nand_cmd_reset(info);

	return ret;
}

static int spinand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)

{
	struct drv_nand_dev_info *info = mtd->priv;
	int eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;
	u8 status = 0, chip_id = info->flash_info->chip_id & 0xFF;
	int ret = 0;

	mutex_lock(&info->lock);

	chip->read_buf(mtd, buf, eccsize * eccsteps);
	if (oob_required)
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	if (info->retcode == ERR_ECC_UNCLEAN) {
		mtd->ecc_stats.failed++;
	} else if (info->retcode == ECC_CORRECTED) {
		if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC) {
				mtd->ecc_stats.corrected += \
					nand_cmd_read_ecc_corrected(info);

				ret = status;
		} else {
			if (chip_id == MFG_ID_MXIC) {
				ret = nand_cmd_read_flash_ecc_corrected(info);
				mtd->ecc_stats.corrected += ret;
			} else if (chip_id == MFG_ID_TOSHIBA) {
				status = nand_cmd_read_status(info, \
					NAND_SPI_STS_FEATURE_4);

				mtd->ecc_stats.corrected += status;
				ret = status;
			} else {
				mtd->ecc_stats.corrected++;
				ret = 1;
			}
		}
	}

	mutex_unlock(&info->lock);

	return ret;

}

static int spinand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required)
{
	int eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;

	chip->write_buf(mtd, buf, eccsize * eccsteps);

	return 0;
}


static int drv_nand_read_id(struct drv_nand_dev_info *info, uint32_t *id)
{
	uint8_t  card_id[4];
	uint8_t  id_count, i;
	int ret = E_OK;

	memset(card_id, 0, sizeof(card_id));

	mutex_lock(&info->lock);

	if (nand_cmd_read_id(card_id, info) != E_OK) {
		pr_err("NAND cmd timeout\r\n");
		ret = -EIO;
	} else {
		pr_debug("id =  0x%02x 0x%02x 0x%02x 0x%02x\n",
			card_id[0], card_id[1], card_id[2], card_id[3]);

		for (i = 0; i < sizeof(spinand_flash_ids); i++) {
			if ((card_id[0] == spinand_flash_ids[i].mfr_id) && \
			(card_id[1] == spinand_flash_ids[i].dev_id)) {
				id_count = spinand_flash_ids[i].id_len;
				memset(card_id + id_count, 0, 4 - id_count);
				break;
			}
		}
		*id = card_id[0] | (card_id[1] << 8) | (card_id[2] << 16) | \
			(card_id[3] << 24);

		ret = E_OK;
	}

	mutex_unlock(&info->lock);

	return ret;
}


static int drv_nand_readpage(struct drv_nand_dev_info *info,
				int column, int page_addr)
{
	return nand_cmd_read_operation(info, info->data_buff,
			page_addr * info->flash_info->page_size, 1);
}

static int drv_nand_write_page(struct drv_nand_dev_info *info,
				int column, int page_addr)
{
	if (column != info->flash_info->page_size)
		nand_cmd_write_spare_sram(info);

	return nand_cmd_write_operation_single(info, \
			page_addr * info->flash_info->page_size, column);
}

static irqreturn_t spinand_irq(int irq, void *devid)
{
	struct drv_nand_dev_info *info = devid;

	info->nand_int_status = NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);

	if (info->nand_int_status) {
		NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, info->nand_int_status);
		complete(&info->cmd_complete);
		return IRQ_HANDLED;
	} else
		return IRQ_NONE;


}

static inline int is_buf_blank(uint8_t *buf, size_t len)
{
	for (; len > 0; len--)
		if (*buf++ != 0xff)
			return 0;
	return 1;
}

static void print_data(unsigned int *buf, size_t count)
{
	int i = 0;

	for (i=0 ; i<(count/4) ; i+=10)
	{
		pr_info("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n", buf[i], buf[i+1], buf[i+2], buf[i+3], buf[i+4], buf[i+5], buf[i+6], buf[i+7], buf[i+8], buf[i+9]);
	}
}

static void spinand_cmdfunc(struct mtd_info *mtd, unsigned command,
		int column, int page_addr)
{
	struct drv_nand_dev_info *info = mtd->priv;
	int ret;
	uint32_t crc_w, crc_r, crc_r2;
	uint32_t hw_crc_w, hw_crc_r, hw_crc_r2;
	uint8_t *ptr;

	info->use_dma = (use_dma) ? 1 : 0;
	info->data_size = 0;
	info->state = STATE_READY;
	info->retcode = ERR_NONE;

	switch (command) {
	case NAND_CMD_READOOB:
		info->buf_count = mtd->writesize + mtd->oobsize;
		info->buf_start = mtd->writesize + column;

		ptr = info->data_buff + info->buf_start;
		if (info->buf_start != info->flash_info->page_size) {
			dev_err(&info->pdev->dev,
			"info->buf_start = %d, != 0\n", info->buf_start);
		}
		mutex_lock(&info->lock);
		nand_cmd_read_page_spare_data(info, ptr,
				info->flash_info->page_size * page_addr);
		mutex_unlock(&info->lock);
		/* We only are OOB, so if the data has error, does not matter */
	break;

	case NAND_CMD_READ0:
	case NAND_CMD_READ1:
		if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
			dev_err(&info->pdev->dev,
				"NAND_CMD_READ0 : is not Cache_Line_Size alignment!\n");

		info->buf_start = column;
		info->buf_count = mtd->writesize + mtd->oobsize;
		mutex_lock(&info->lock);
		ret = drv_nand_readpage(info, column, page_addr);
		if (readback) {
			crc_r =	crc32(~0, info->data_buff, mtd->writesize);
			hw_crc_r = hwcopy_chksum((u32)info->data_buff, mtd->writesize);
			memset(info->data_buff_bak, 0x0, SZ_4K);
			memcpy(info->data_buff_bak, info->data_buff, info->data_buff_size);
			ret = drv_nand_readpage(info, column, page_addr);
			crc_r2 = crc32(~0, info->data_buff, mtd->writesize);
			hw_crc_r2 = hwcopy_chksum((u32)info->data_buff, mtd->writesize);
		}
		mutex_unlock(&info->lock);

		if (readback) {
			if ((crc_r != crc_r2) || (hw_crc_r != hw_crc_r2)) {
				pr_info("FLASH read compare error =====================>>>> \n");
				pr_info("Dump 1: crc_r2 %x crc_r %x hw_crc_r2 %x hw_crc_r %x\n", crc_r2, crc_r, hw_crc_r2, hw_crc_r);
				crc_r2 = crc32(~0, info->data_buff, mtd->writesize);
				pr_info("Read buf:\n");
				print_data((unsigned int*)info->data_buff_bak, info->data_buff_size);
				pr_info("Read buf2:\n");
				print_data((unsigned int*)info->data_buff, info->data_buff_size);
				pr_info("Dump 2: crc_r2 %x crc_r %x hw_crc_r2 %x hw_crc_r %x\n", crc_r2, crc_r, hw_crc_r2, hw_crc_r);
				crc_r2 = crc32(~0, info->data_buff, mtd->writesize);
				pr_info("Read buf:\n");
				print_data((unsigned int*)info->data_buff_bak, info->data_buff_size);
				pr_info("Read buf2:\n");
				print_data((unsigned int*)info->data_buff, info->data_buff_size);
				pr_info("Dump 3: crc_r2 %x crc_r %x hw_crc_r2 %x hw_crc_r %x\n", crc_r2, crc_r, hw_crc_r2, hw_crc_r);
				crc_r2 = crc32(~0, info->data_buff, mtd->writesize);
				pr_info("Read buf:\n");
				print_data((unsigned int*)info->data_buff_bak, info->data_buff_size);
				pr_info("Read buf2:\n");
				print_data((unsigned int*)info->data_buff, info->data_buff_size);
				panic("FLASH read compare error =====================>>>> \n");
			}
		}

		if (ret == E_CTX)
			info->retcode = ERR_ECC_UNCLEAN;
		else if (ret < 0)
			info->retcode = ERR_SENDCMD;
		else if (ret == ECC_CORRECTED)
			info->retcode = ECC_CORRECTED;
		else
			info->retcode = ERR_NONE;
	break;

	case NAND_CMD_SEQIN:
		info->buf_start = column;
		info->buf_count = mtd->writesize + mtd->oobsize;
		memset(info->data_buff, 0xff, info->buf_count);

		/* save column/page_addr for next CMD_PAGEPROG */
		info->seqin_column = column;
		info->seqin_page_addr = page_addr;
	break;

	case NAND_CMD_PAGEPROG:
		if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
			dev_err(&info->pdev->dev,
				"not CACHE_LINE_SIZE alignment!\n");

		mutex_lock(&info->lock);
		if (readback) {
			crc_w =	crc32(~0, info->data_buff, mtd->writesize);
			memset(info->data_buff_bak, 0x0, SZ_4K);
			memcpy(info->data_buff_bak, info->data_buff, info->data_buff_size);
			ret = drv_nand_write_page(info, info->seqin_column, info->seqin_page_addr);
			hw_crc_w = hwcopy_chksum((u32)info->data_buff, mtd->writesize);
		} else
			ret = drv_nand_write_page(info, info->seqin_column, info->seqin_page_addr);
		mutex_unlock(&info->lock);

		if (ret)
			info->retcode = ERR_SENDCMD;

		if (readback) {
			info->buf_start = info->seqin_column;
			info->buf_count = mtd->writesize + mtd->oobsize;
			mutex_lock(&info->lock);
			drv_nand_readpage(info, info->seqin_column, info->seqin_page_addr);
			crc_r =	crc32(~0, info->data_buff, mtd->writesize);
			hw_crc_r = hwcopy_chksum((u32)info->data_buff, mtd->writesize);
			mutex_unlock(&info->lock);

			if ((crc_w != crc_r) || (hw_crc_w != hw_crc_r)) {
				pr_info("FLASH write compare error =====================>>>> \n");
				pr_info("Dump 1: crc_w %x crc_r %x hw_crc_w %x hw_crc_r %x\n", crc_w, crc_r, hw_crc_w, hw_crc_r);
				crc_r = crc32(~0, info->data_buff, info->data_buff_size);
				pr_info("Write buf:\n");
				print_data((unsigned int*)info->data_buff_bak, info->data_buff_size);
				pr_info("Read buf:\n");
				print_data((unsigned int*)info->data_buff, info->data_buff_size);
				pr_info("Dump 2: crc_w %x crc_r %x hw_crc_w %x hw_crc_r %x\n", crc_w, crc_r, hw_crc_w, hw_crc_r);
				crc_r = crc32(~0, info->data_buff, info->data_buff_size);
				pr_info("Write buf:\n");
				print_data((unsigned int*)info->data_buff_bak, info->data_buff_size);
				pr_info("Read buf:\n");
				print_data((unsigned int*)info->data_buff, info->data_buff_size);
				pr_info("Dump 3: crc_w %x crc_r %x hw_crc_w %x hw_crc_r %x\n", crc_w, crc_r, hw_crc_w, hw_crc_r);
				crc_r = crc32(~0, info->data_buff, info->data_buff_size);
				pr_info("Write buf:\n");
				print_data((unsigned int*)info->data_buff_bak, info->data_buff_size);
				pr_info("Read buf:\n");
				print_data((unsigned int*)info->data_buff, info->data_buff_size);
				panic("FLASH write compare error\n");
			}
		}
	break;

	case NAND_CMD_ERASE1:
		mutex_lock(&info->lock);
		ret = nand_cmd_erase_block(info, page_addr);
		mutex_unlock(&info->lock);

		if (ret)
			info->retcode = ERR_BBERR;

		if (enable_erase_cnt) {
			u32 block_number = page_addr/info->flash_info->page_per_block;
			erase_count[block_number] += 1;
		}
	break;

	case NAND_CMD_ERASE2:
	break;

	case NAND_CMD_READID:
		info->buf_start = 0;
		info->buf_count = 16;
		if(info->flash_info->chip_id)
			memcpy((uint32_t *)info->data_buff, &info->flash_info->chip_id, 16);
		else {
			ret = drv_nand_read_id(info, (uint32_t *)info->data_buff);
			if (ret)
				info->retcode = ERR_SENDCMD;
		}
	break;
	case NAND_CMD_STATUS:
		info->buf_start = 0;
		info->buf_count = 1;
		mutex_lock(&info->lock);
		nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_2);
		mutex_unlock(&info->lock);
		if (!(info->data_buff[0] & 0x80))
			info->data_buff[0] = 0x80;
	break;

	case NAND_CMD_RESET:
	break;

	default:
		pr_debug(KERN_ERR "non-supported command.\n");
	break;
	}
}

static void spinand_select_chip(struct mtd_info *mtd, int dev)
{

}

static uint8_t spinand_read_byte(struct mtd_info *mtd)
{
	struct drv_nand_dev_info *info = mtd->priv;
	char retval = 0xFF;

	if (info->buf_start < info->buf_count)
		/* Has just send a new command? */
		retval = info->data_buff[info->buf_start++];

	return retval;
}

static void spinand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct drv_nand_dev_info *info = mtd->priv;

	memcpy(buf, info->data_buff + info->buf_start, len);

	info->buf_start += len;
}

static void spinand_write_buf(struct mtd_info *mtd,
	const uint8_t *buf, int len)
{
	struct drv_nand_dev_info *info = mtd->priv;

	memcpy(info->data_buff + info->buf_start, buf, len);

	info->buf_start += len;
}


static int spinand_wait(struct mtd_info *mtd, struct nand_chip *this)
{
	struct drv_nand_dev_info *info = mtd->priv;
	int ret = info->retcode;

	info->retcode = ERR_NONE;

	if (ret < 0)
		return NAND_STATUS_FAIL;
	else
		return E_OK;
}

#ifdef CONFIG_MTD_SPINAND_HWECC
static void drv_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
{
	struct drv_nand_dev_info *info = mtd->priv;
	dev_info(&info->pdev->dev, "%s retcode\n", __func__);
	return;
}

static int drv_nand_ecc_calculate(struct mtd_info *mtd,
	const uint8_t *dat, uint8_t *ecc_code)
{
	struct drv_nand_dev_info *info = mtd->priv;
	dev_info(&info->pdev->dev, "%s retcode\n", __func__);
	return 0;
}

static int drv_nand_ecc_correct(struct mtd_info *mtd,
	uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc)
{
	struct drv_nand_dev_info *info = mtd->priv;
	dev_info(&info->pdev->dev, "%s retcode\n", __func__);
	return 0;
}
#endif

static int drv_nand_identify(struct drv_nand_dev_info *info)
{
	int maf_id, dev_id;
	uint32_t id;
	uint32_t reg;
	u8 status;

	if(drv_nand_read_id(info, &id) != E_OK)
		return -EIO;

	maf_id = id & 0xFF;
	dev_id = (id >> 8) & 0xFF;

	info->flash_info->page_per_block = 64;
	info->flash_info->page_size = 2048;
	info->flash_info->oob_size = 64;
	info->flash_info->subpage_size = 512;
	info->flash_info->flash_width = 8;
	info->flash_info->nfc_width = 8;
	info->flash_info->num_blocks = 2048;
	info->flash_info->chip_id = id;
	info->flash_info->phy_page_ratio= 4;
	info->flash_info->device_size = 128;
	info->flash_info->chip_sel = 0;
	info->flash_info->block_size = 0x20000;
	info->flash_info->module_config = NAND_PAGE2K | NAND_2COL_ADDR;

	total_block_cnt = info->flash_info->num_blocks;

	if (info->flash_info->chip_id == MXIC_MX35LF2G14AC)
		info->flash_info->plane_flags = 1;
	else
		info->flash_info->plane_flags = 0;

	mutex_lock(&info->lock);
	status = nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_2);

	if ((status & SPINAND_CONFIG_ECCEN) || (info->flash_info->plane_flags)) {
		info->use_ecc = NANDCTRL_SPIFLASH_USE_ONDIE_ECC;
		info->flash_info->module_config |= NAND_PRI_ECC_SPI_ON_DIE;
		NAND_SETREG(info, NAND_INTMASK_REG_OFS, NAND_COMP_INTREN | \
			NAND_STS_FAIL_INTREN | NAND_TOUT_INTREN);
	} else {
		info->use_ecc = NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC;
		info->flash_info->module_config |= NAND_PRI_ECC_RS;
		NAND_SETREG(info, NAND_INTMASK_REG_OFS, NAND_COMP_INTREN | \
			NAND_PRI_ECC_INTREN | NAND_STS_FAIL_INTREN | NAND_TOUT_INTREN);
	}

	nand_host_setup_page_size(info, NAND_PAGE_SIZE_2048);

	reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);
	reg &= ~(0x7FFFF);
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, reg | info->flash_info->module_config);
	mutex_unlock(&info->lock);

	flash_copy_info(info);

	return 0;
}


/* the maximum possible buffer size for large page with OOB data
 * is: 2048 + 64 = 2112 bytes, allocate a page here for both the
 * data buffer and the DMA descriptor
 */
#define MAX_BUFF_SIZE	PAGE_SIZE

static int drv_nand_init_buff(struct drv_nand_dev_info *info)
{
	struct platform_device *pdev = info->pdev;

	if (MAX_BUFF_SIZE >= (2048 + 64)) {
		info->data_buff = kzalloc(MAX_BUFF_SIZE, GFP_KERNEL);

		if (info->data_buff == NULL) {
			dev_err(&pdev->dev, "failed to allocate dma buffer\n");
			kfree(info->data_buff);
			return -ENOMEM;
		}
		info->data_buff_size = MAX_BUFF_SIZE;
	} else {
		info->data_buff = kzalloc((2048+64), GFP_KERNEL);

		if (info->data_buff == NULL) {
			dev_err(&pdev->dev, "failed to allocate dma buffer\n");
			kfree(info->data_buff);
			return -ENOMEM;
		}
		info->data_buff_size = (2048+64);
	}

	if ((unsigned long)info->data_buff % CACHE_LINE_SIZE)
		info->data_buff = (uint8_t *)((((unsigned long)info->data_buff + CACHE_LINE_SIZE - 1)) & 0xFFFFFFC0);

	return E_OK;
}

static void spinand_init_mtd(struct mtd_info *mtd)
{
	struct drv_nand_dev_info *info = mtd->priv;
	struct nand_chip *chip = &info->nand_chip;
#ifdef CONFIG_MTD_SPINAND_HWECC
	const struct nvt_nand_flash *f = info->flash_info;
	u8 chip_id = f->chip_id & 0xFF;
	chip->ecc.mode	= NAND_ECC_HW;
	chip->ecc.size	= 0x200;
	chip->ecc.bytes	= 0x8;
	chip->ecc.steps	= 0x4;

	chip->ecc.strength = 1;
	chip->ecc.total	= chip->ecc.steps * chip->ecc.bytes;
	if (chip_id == MFG_ID_ETRON)
		chip->ecc.layout = &spinand_oob_etron;
	else if (chip_id == MFG_ID_GD)
		chip->ecc.layout = &spinand_oob_gd;
	else
		chip->ecc.layout = &spinand_oob_64;
	chip->ecc.read_page = spinand_read_page_hwecc;
	chip->ecc.write_page = spinand_write_page_hwecc;

	chip->ecc.hwctl = drv_nand_ecc_hwctl;
	chip->ecc.calculate = drv_nand_ecc_calculate;
	chip->ecc.correct = drv_nand_ecc_correct;
	chip->options |= NAND_NO_SUBPAGE_WRITE;
#else
	chip->ecc.mode	= NAND_ECC_SOFT;
	if (spi_nand_unlock_bp(info) < 0)
		pr_err("%s: disable ecc failed!\n", __func__);
#endif

	chip->priv	= info;
	chip->read_buf	= spinand_read_buf;
	chip->write_buf	= spinand_write_buf;
	chip->read_byte	= spinand_read_byte;
	chip->cmdfunc	= spinand_cmdfunc;
	chip->waitfunc	= spinand_wait;
	chip->select_chip = spinand_select_chip;
	info->flash_info->spi_nand_status.block_unlocked = FALSE;
	info->flash_info->spi_nand_status.quad_en = FALSE;
	info->flash_info->spi_nand_status.quad_program = FALSE;
	info->flash_info->spi_nand_status.timer_record = FALSE;
}

static int spinand_probe(struct platform_device *pdev)
{
	struct drv_nand_dev_info *info;
	struct mtd_part_parser_data ppdata;
	struct platform_nand_data *pplatdata;
	struct nand_chip *this;
	struct mtd_info *mtd;
	struct resource *res;
	int ret = 0;
	u8 chip_id;

	mtd = kmalloc(sizeof(struct mtd_info) +
		sizeof(struct drv_nand_dev_info), GFP_KERNEL | __GFP_ZERO);
	if (!mtd) {
		dev_err(&pdev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}
	info = (struct drv_nand_dev_info *)(&mtd[1]);
	info->pdev = pdev;

	info->flash_info = kmalloc(sizeof(struct nvt_nand_flash), GFP_KERNEL);
	if (!info->flash_info) {
		dev_err(&pdev->dev, "failed to allocate nvt_nand_flash\n");
		return -ENOMEM;
	}

	info->data_buff_bak = kmalloc(SZ_4K , GFP_KERNEL);

	pplatdata = NULL;
	if (pdev->dev.platform_data != NULL)
	{
		pplatdata = pdev->dev.platform_data;
	}

	this = &info->nand_chip;
	mtd->priv = info;

	/* Get mtd device resource of base address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (!res) {
		dev_err(&pdev->dev, "failed to get resource\n");
		goto fail_free_mtd;
	}

	info->mmio_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(info->mmio_base)) {
		PTR_ERR(info->mmio_base);
		goto fail_free_mtd;
	}

	info->irq = platform_get_irq(pdev, 0);
	if (info->irq < 0) {
		dev_err(&pdev->dev, "no IRQ resource defined\n");
		ret = -ENXIO;
		goto fail_free_mtd;
	}

	ret = drv_nand_init_buff(info);
	if (ret) {
		dev_err(&pdev->dev, "failed to allocate buffer\n");
		goto fail_free_buf;
	}

	ret = request_irq(info->irq, spinand_irq, IRQF_SHARED, pdev->name, info);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request IRQ\n");
		goto fail_free_buf;
	}

	of_property_read_u32(pdev->dev.of_node, "clock-frequency",
				&info->flash_freq);

	of_property_read_u32(pdev->dev.of_node, "readback_check",
				&readback);

	mutex_init(&info->lock);

	init_completion(&info->cmd_complete);

	mutex_lock(&info->lock);
	drv_nand_reset(info, pdev);
	nand_dll_reset(info);
	mutex_unlock(&info->lock);
	/*Delay 1ms for spinand characteristic*/
	mdelay(1);

	ret = drv_nand_identify(info);

	if (ret) {
		dev_err(&pdev->dev, "failed to identify flash\n");
		ret = -ENODEV;
		goto fail_free_irq;
	}

	spinand_init_mtd(mtd);
	mtd->priv = this;
#ifdef CONFIG_OF
	of_property_read_string_index(pdev->dev.of_node, "nvt-devname", 0, &mtd->name);
#else
	mtd->name = dev_name(&pdev->dev);
#endif
	mtd->owner = THIS_MODULE;
	mtd->oobsize = 64;

	chip_id = info->flash_info->chip_id & 0xFF;
	if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC) {
			info->nand_chip.ecc.strength = 4;
			mtd->bitflip_threshold = 2;
	} else {
		if (chip_id == MFG_ID_WINBOND)
			mtd->bitflip_threshold = 1;
		else if (chip_id == MFG_ID_MXIC) {
			info->nand_chip.ecc.strength = 4;
			mtd->bitflip_threshold = 3;
		} else if (chip_id == MFG_ID_TOSHIBA) {
			info->nand_chip.ecc.strength = 8;
			mtd->bitflip_threshold = 6;
		} else
			mtd->bitflip_threshold = 8;
	}

	platform_set_drvdata(pdev, mtd);
	/* Scan for card properties */

	if (info->flash_info->chip_id != TOSHIBA_TC58CVG)
		info->flash_info->spi_nand_status.quad_program = TRUE;

	ret = nand_scan_ident(mtd, 1, spinand_flash_ids);
	if (ret) {
		dev_err(&pdev->dev, "Identify nand fail\n");
		goto fail_free_irq;
	}

	ret = nand_scan_tail(mtd);
	if (ret) {
		dev_err(&pdev->dev, "nand scan tail fail\n");
		goto fail_free_irq;
	}


	ret = nvt_nand_proc_init();
	if (ret) {
		dev_err(&pdev->dev, "create nand static proc failed\n");
	}

	ginfo = info;

	ppdata.of_node = pdev->dev.of_node;
	if (pplatdata == NULL)
		return mtd_device_parse_register(mtd, NULL, &ppdata, NULL, 0);
	else
		return mtd_device_parse_register(mtd, NULL, &ppdata,
				pplatdata->chip.partitions,
				pplatdata->chip.nr_partitions);

fail_free_irq:
	free_irq(info->irq, info);

fail_free_buf:
	kfree(info->data_buff);

fail_free_mtd:
	kfree(mtd);
	return ret;
}

static int spinand_remove(struct platform_device *pdev)
{
	struct mtd_info *mtd = platform_get_drvdata(pdev);
	struct drv_nand_dev_info *info = mtd->priv;

	platform_set_drvdata(pdev, NULL);

	nand_release(mtd);

	free_irq(info->irq, info);

	kfree(info->data_buff);

	kfree(mtd);
	return E_OK;
}

#ifdef CONFIG_PM
static int spinand_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mtd_info *mtd = platform_get_drvdata(pdev);
	struct drv_nand_dev_info *info = mtd->priv;

	if (!IS_ERR(info->clk))
		clk_disable(info->clk);

	return 0;
}
static int spinand_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mtd_info *mtd = platform_get_drvdata(pdev);
	struct drv_nand_dev_info *info = mtd->priv;

	clk_enable(info->clk);

	mdelay(10);

	drv_nand_reset(info, pdev);

	nand_dll_reset(info);

	return 0;
}
static const struct dev_pm_ops spinand_pmops = {
	.suspend	= spinand_suspend,
	.resume		= spinand_resume,
};
#define SPINAND_PMOPS &spinand_pmops
#else
#define SPINAND_PMOPS NULL
#endif /* CONFIG_PM */

#ifdef CONFIG_OF
static const struct of_device_id nvt_spinand_of_dt_ids[] = {
    { .compatible = "nvt,nvt_spinand" },
    {},
};

MODULE_DEVICE_TABLE(of, nvt_spinand_of_dt_ids);
#endif

static struct platform_driver nvt_nand_driver = {
	.driver = {
		.name = "spi_nand",
		.owner = THIS_MODULE,
		.pm = SPINAND_PMOPS,
#ifdef CONFIG_OF
		.of_match_table = nvt_spinand_of_dt_ids,
#endif
	},
	.probe	    = spinand_probe,
	.remove     = spinand_remove,
};

static int __init spinand_init(void)
{
	return platform_driver_register(&nvt_nand_driver);
}

static void __exit spinand_exit(void)
{
	platform_driver_unregister(&nvt_nand_driver);
}

module_init(spinand_init);
module_exit(spinand_exit);

MODULE_DESCRIPTION("SPI NAND driver");
MODULE_AUTHOR("Howard Chang");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.04.080");

