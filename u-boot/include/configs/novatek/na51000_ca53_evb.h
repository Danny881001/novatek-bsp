/* NVT board configuration */
#define CONFIG_MEM_SIZE				_BOARD_DRAM_SIZE_
/* 25MB for kernel image */
#define CONFIG_NVT_UIMAGE_SIZE			(SZ_1M * 25)
#define CONFIG_RAMDISK_SDRAM_SIZE		(SZ_1M * 30)		/* To define maximum ramdisk size */
/* temp buffer for all-in-one T.bin boot requirement */
#define CONFIG_UBOOT_SDRAM_BASE			_BOARD_UBOOT_ADDR_
#define CONFIG_UBOOT_SDRAM_SIZE			_BOARD_UBOOT_SIZE_
#define CONFIG_LINUX_SDRAM_BASE			_BOARD_LINUXTMP_ADDR_
#define CONFIG_LINUX_SDRAM_SIZE			_BOARD_LINUXTMP_SIZE_
#define CONFIG_LINUX_SDRAM_START		(CONFIG_UBOOT_SDRAM_BASE - CONFIG_NVT_UIMAGE_SIZE)
#define CONFIG_NVT_RUNFW_SDRAM_BASE		(CONFIG_LINUX_SDRAM_START - (SZ_1M * 30))

#define CONFIG_SMEM_SDRAM_BASE			_BOARD_SHMEM_ADDR_
#define CONFIG_SMEM_SDRAM_SIZE			_BOARD_SHMEM_SIZE_

#if defined(CONFIG_NVT_SPI_NAND)
#define CONFIG_MODELEXT_FLASH_BASE		0x40000
#elif defined(CONFIG_NVT_IVOT_EMMC)
#define CONFIG_MODELEXT_FLASH_BASE		(0x40000/512)
#else
#define CONFIG_MODELEXT_FLASH_BASE		0x10000
#endif

/* FLASH        FileSystem */
#ifdef CONFIG_NVT_SPI_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE              1
#define CONFIG_SYS_NAND_MAX_CHIPS               1
#endif

#define CONFIG_SYS_NAND_BASE                    0xF0400000
#define CONFIG_SYS_HZ_CLOCK                     12000000