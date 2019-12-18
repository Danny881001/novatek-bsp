/**
    SRAM Control header file
    This file will handle core communications.
    @file       nvt-sramctl.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _NT96680_NVT_SRAMCTL_H
#define _NT96680_NVT_SRAMCTL_H
#include <mach/rcw_macro.h>
#include <mach/nvt_type.h>
//#define ENUM_DUMMY4WORD(m)

/*
    SRAM ShutDown ID

    This is for pinmux_enableSramShutDown() and pinmux_disableSramShutDown().
*/
typedef enum {
	SIE_SD = 4,       //< Shut Down SIE SRAM
	SIE2_SD,          //< Shut Down SIE2 SRAM
	/*5*/

	IPE_SD = 7,       //< Shut Down IPE SRAM
	DIS_SD,           //< Shut Down DIS SRAM
	IME_SD,           //< Shut Down IME SRAM
	FDE_SD,           //< Shut Down FDE SRAM
	ISE_SD,           //< Shut Down ISE SRAM
	/*11*/

	IFE_SD = 13,      //< Shut Down IFE SRAM
	DCE_SD,           //< Shut Down DCE SRAM
	IFE2_SD,          //< Shut Down IFE2 SRAM
	IDE_SD,           //< Shut Down IDE SRAM
	IDE2_SD,          //< Shut Down IDE2 SRAM
	/*17*/

	H264_SD = 24,     //< Shut Down H264 SRAM
	JPG_SD = 26,      //< Shut Down JPG SRAM
	GRAPH_SD,         //< Shut Down Graphic SRAM
	GRAPH2_SD,        //< Shut Down Graphic2 SRAM
	/*28*/

	NAND_SD = 32,     //< Shut Down NAND SRAM
	SDIO1_SD = 34,    //< Shut Down SDIO1 SRAM
	SDIO2_SD,         //< Shut Down SDIO2 SRAM
	/*35*/

	SDIO3_SD = 46,    //< Shut Down SDIO3 SRAM
	USB_SD = 51,      //< Shut Down USB2.0 SRAM
	ETH_SD = 61,      //< Shut Down ETH SRAM
	MIPI_SD = 67,     //< Shut Down MIPI SRAM
	RDE_SD = 70,      //< Shut Down RDE SRAM
	DRE_SD = 76,      //< Shut Down DRE SRAM
	TSMUX_SD,         //< Shut Down DRE SRAM
	/*77*/

	SIE3_SD = 79,     //< Shut Down SIE3 SRAM
	SIE4_SD,          //< Shut Down SIE4 SRAM
	USB2_SD,          //< Shut Down USB3.0 SRAM
	/*81*/

	RHE_SD = 83,      //< Shut Down RHE SRAM
	HWCPY_SD = 94,    //< Shut Down HWCPY SRAM
	ROTATE_SD,        //< Shut Down ROTATE SRAM
	/*95*/

	SIE5_SD = 122,    //< Shut Down SIE5 SRAM
	SIE6_SD,          //< Shut Down SIE6 SRAM
	SIE7_SD,          //< Shut Down SIE7 SRAM
	SIE8_SD,          //< Shut Down SIE8 SRAM
	IVE_SD,           //< Shut Down IVE SRAM
	SDE_SD,           //< Shut Down SDE CNN SRAM
	SVM_SD,           //< Shut Down SVM SRAM
	/*128*/

	ISE2_SD = 130,    //< Shut Down ISE2 SRAM
	CPU2_SD = 192,    //< Shut Down CPU2 SRAM
	CPU2_L2CACHE_SD,  //< Shut Down CPU2 L2 CACHE SRAM
	CPU2_CORESIGHT_SD,//< Shut Down CPU2 CORESIGHT SRAM

	ENUM_DUMMY4WORD(SRAM_SD)
} SRAM_SD;

extern void nvt_disable_sram_shutdown(SRAM_SD id);
extern void nvt_enable_sram_shutdown(SRAM_SD id);


#endif /* _NT96680_NVT_SRAMCTL_H */
