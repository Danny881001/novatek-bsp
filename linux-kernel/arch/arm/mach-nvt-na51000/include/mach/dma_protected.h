/*
    Novatek protected header file of NT96660's driver.

    The header file for Novatek protected APIs of NT96660's driver.

    @file       efuse_protected.h
    @ingroup    mIDriver
    @note       For Novatek internal reference, don't export to agent or customer.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _NVT_DMA_PROTECTED_H
#define _NVT_DMA_PROTECTED_H

#include "nvt_type.h"

/**
    DMA controller ID

*/
typedef enum {
	DMA_ID_1,                           ///< DMA Controller
	DMA_ID_2,                           ///< DMA Controller 2

	DMA_ID_COUNT,                       //< DMA controller count

	ENUM_DUMMY4WORD(DMA_ID)
} DMA_ID;

/*
    Get DRAM utilization

    Get DRAM bandwidth utilization

    @return DRAM utilization X  %
*/
extern UINT32   dma_get_utilization(DMA_ID id);

#endif

