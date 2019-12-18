/*
	Drvdump header
	Copyright Novatek Microelectronics Corp. 2013.  All rights reserved.
*/

#ifndef __DRVDUMP_H
#define __DRVDUMP_H

#if defined(CONFIG_MTD_SPINAND) || defined(CONFIG_MTD_SPINOR)
void nand_drvdump(void);
#else
inline void nand_drvdump(void) {};
#endif

#if defined(CONFIG_MMC_NA51000)
void sdio_drvdump(void);
#else
inline void sdio_drvdump(void) {};
#endif
#endif

