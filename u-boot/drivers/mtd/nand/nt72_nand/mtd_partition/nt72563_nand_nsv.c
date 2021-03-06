struct mtd_partition g_nt72_nand_mtd_partitions[] = {
	[0] = {
		.name        = "xboot",
		.offset      = 0,
		.size        = SZ_128K * 4,
	},
	[1] = {
		.name        = "ddrcfg",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 4,
	},
	[2] = {
		.name        = "ddrbud",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 3,
	},
	[3] = {
		.name        = "xbootdat",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 5,
	},
	[4] = {
		.name        = "misc",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 3,
	},
	[5] = {
		.name        = "stbc",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 5,
	},
	[6] = {
		.name        = "secos",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 15,
	},
	[7] = {
		.name        = "uboot",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 7,
	},
	[8] = {
		.name        = "stbcdat",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 3,
	},
	[9] = {
		.name        = "fdt0",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 3,
	},
	[10] = {
		.name        = "fdt1",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 3,
	},
	[11] = {
		.name        = "ker0",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 40,
	},
	[12] = {
		.name        = "ker1",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 40,
	},
	[13] = {
		.name        = "fs0",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 355,
	},
	[14] = {
		.name        = "supfs",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 355,
	},
	[15] = {
		.name        = "ap0",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 4500,
	},
	[16] = {
		.name        = "ap1",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 572,
	},
	[17] = {
		.name        = "apdat0",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 89,
	},
	[18] = {
		.name        = "devcfg",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 64,
	},
	[19] = {
		.name        = "Netflix_Cache",
		.offset      = MTDPART_OFS_APPEND,
		.size        = SZ_128K * 550,
	}
};
