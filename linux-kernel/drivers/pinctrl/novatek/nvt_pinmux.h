#ifndef __PINMUX_DRV_H__
#define __PINMUX_DRV_H__

#include <mach/top_reg.h>
#include <mach/hardware.h>
#include <mach/top.h>
#include <mach/nvt-gpio.h>
#include <linux/semaphore.h>

typedef enum
{
	BOOT_SRC_NAND_RS,                   //< Boot from NAND (Reed Solomon)
	BOOT_SRC_SDIO,                      //< Boot from SDIO
	BOOT_SRC_EMMC,                      //< Boot from eMMC (SDIO3 2nd pinmux)
	BOOT_SRC_USB_FS,                    //< Boot from USB (Full Speed)
	BOOT_SRC_SPI,                       //< Boot from SPI
	BOOT_SRC_USB_HS,                    //< Boot from USB (High Speed)
	BOOT_SRC_SPI_NAND,                  //< Boot from SPI_NAND
	BOOT_SRC_BMC,                       //< Boot from BMC
	BOOT_SRC_ETH,

	ENUM_DUMMY4WORD(BOOT_SRC_ENUM)
} BOOT_SRC_ENUM;


/*
	EJTAG_ENUM
*/
typedef enum
{
	EJTAG_GPIO,                         //< Disable EJTAG (ICE interface)
	EJTAG_EN,                           //< Enable EJTAG

	ENUM_DUMMY4WORD(EJTAG_ENUM)
} EJTAG_ENUM;

/*
	MPLL_CLK_ENUM
*/
typedef enum
{
	MPLL_CLK_NORMAL,                    //< PLL2~13 from internal MPLL
	MPLL_CLK_BYPASS,                    //< Bypass MPLL (From external pad)

	ENUM_DUMMY4WORD(MPLL_CLK_ENUM)
} MPLL_CLK_ENUM;

/*
	NAND_ACT_ENUM
*/
typedef enum
{
	NAND_ACT_DIS,                       //< De-activate NAND ACT
	NAND_ACT_EN,                        //< Activate NAND ACT

	ENUM_DUMMY4WORD(NAND_ACT_ENUM)
} NAND_ACT_ENUM;

/*
	FSPI_ACT_ENUM
*/
typedef enum
{
	FSPI_ACT_DIS,                        //< De-activate FSPI ACT
	FSPI_ACT_EN,                         //< Activate FSPI ACT

	ENUM_DUMMY4WORD(FSPI_ACT_ENUM)
} FSPI_ACT_ENUM;

/*
	SDIO2_ACT_ENUM
*/
typedef enum
{
	SDIO2_ACT_DIS,                      //< De-activate SDIO2 ACT
	SDIO2_ACT_EN,                       //< Activate SDIO2 ACT

	ENUM_DUMMY4WORD(SDIO2_ACT_ENUM)
} SDIO2_ACT_ENUM;

/*
	NAND_EXIST_ENUM
*/
typedef enum
{
	NAND_EXIST_DIS,                     //< De-activate NAND EXIST
	NAND_EXIST_EN,                      //< Activate NAND EXIST

	ENUM_DUMMY4WORD(NAND_EXIST_ENUM)
} NAND_EXIST_ENUM;

/*
	FSPI_EXIST_ENUM
*/
typedef enum
{
	FSPI_EXIST_DIS,                      //< De-activate FSPI EXIST
	FSPI_EXIST_EN,                       //< Activate FSPI EXIST

	ENUM_DUMMY4WORD(FSPI_EXIST_ENUM)
} FSPI_EXIST_ENUM;

/*
	SDIO_EXIST_ENUM
*/
typedef enum
{
	SDIO_EXIST_DIS,                     //< De-activate SDIO EXIST
	SDIO_EXIST_EN,                      //< Activate SDIO EXIST

	ENUM_DUMMY4WORD(SDIO_EXIST_ENUM)
} SDIO_EXIST_ENUM;

/*
	SDIO2_EXIST_ENUM
*/
typedef enum
{
	SDIO2_EXIST_DIS,                    //< De-activate SDIO2 EXIST
	SDIO2_EXIST_EN,                     //< Activate SDIO2 EXIST

	ENUM_DUMMY4WORD(SDIO2_EXIST_ENUM)
} SDIO2_EXIST_ENUM;

/*
	SDIO3_EXIST_ENUM
*/
typedef enum
{
	SDIO3_EXIST_DIS,                    //< De-activate SDIO3 EXIST
	SDIO3_EXIST_EN,                     //< Activate SDIO3 EXIST

	ENUM_DUMMY4WORD(SDIO3_EXIST_ENUM)
} SDIO3_EXIST_ENUM;

/*
	EXTROM_EXIST_ENUM
*/
typedef enum
{
	EXTROM_EXIST_DIS,                   //< De-activate EXTROM ACT
	EXTROM_EXIST_EN,                    //< Activate EXTROM ACT

	ENUM_DUMMY4WORD(EXTROM_EXIST_ENUM)
} EXTROM_EXIST_ENUM;

/*
	SDIO3_MUX_ENUM
*/
typedef enum
{
	SDIO3_MUX_1ST,                      //< 1st pinmux pad
	SDIO3_MUX_2ND,                      //< 2nd pinmux pad

	ENUM_DUMMY4WORD(SDIO3_MUX_ENUM)
} SDIO3_MUX_ENUM;

/*
	SPI_MUX_ENUM
*/
typedef enum
{
	SPI_MUX_GPIO,                       //< GPIO
	SPI_MUX_1ST,                        //< 1st pinmux pad
	SPI_MUX_2ND,                        //< 2nd pinmux pad
	SPI_MUX_3RD,                        //< 3rd pinmux pad

	ENUM_DUMMY4WORD(SPI_MUX_ENUM)
} SPI_MUX_ENUM;

/*
	ROM_SRC_ENUM
*/
typedef enum
{
	ROM_SRC_INTERNAL,                   //< Internal ROM
	ROM_SRC_EXTERNAL,                   //< External ROM

	ENUM_DUMMY4WORD(ROM_SRC_ENUM)
} ROM_SRC_ENUM;

/*
	NAND_CS_NUM_ENUM
*/
typedef enum
{
	NAND_CS_NUM_1CS,                    //< 1 CS Pin
	NAND_CS_NUM_2CS,                    //< 2 CS Pin

	ENUM_DUMMY4WORD(NAND_CS_NUM_ENUM)
} NAND_CS_NUM_ENUM;

/*
	SPI_BUS_WIDTH_ENUM
*/
typedef enum
{
	SPI_BUS_WIDTH_2BITS,                //< SPI is 2 bits
	SPI_BUS_WIDTH_4BITS,                //< SPI is 4 bits

	ENUM_DUMMY4WORD(SPI_BUS_WIDTH_ENUM)
} SPI_BUS_WIDTH_ENUM;

/*
	SDIO2_BUS_WIDTH_ENUM
*/
typedef enum
{
	SDIO2_BUS_WIDTH_4BITS,              //< SDIO2 is 4 bits
	SDIO2_BUS_WIDTH_8BITS,              //< SDIO2 is 8 bits (no use, Backward compatible)

	ENUM_DUMMY4WORD(SDIO2_BUS_WIDTH_ENUM)
} SDIO2_BUS_WIDTH_ENUM;

/*
	SDIO3_BUS_WIDTH_ENUM
*/
typedef enum
{
	SDIO3_BUS_WIDTH_4BITS,              //< SDIO3 is 4 bits
	SDIO3_BUS_WIDTH_8BITS,              //< SDIO3 is 8 bits

	ENUM_DUMMY4WORD(SDIO3_BUS_WIDTH_ENUM)
} SDIO3_BUS_WIDTH_ENUM;


/*
	LCDTYPE_ENUM
*/
typedef enum
{
	LCDTYPE_ENUM_GPIO,                  //< GPIO
	LCDTYPE_ENUM_CCIR656,               //< CCIR656
	LCDTYPE_ENUM_CCIR601,               //< CCIR601
	LCDTYPE_ENUM_PARALLEL_LCD,          //< Parallel LCD (RGB565, RGB666, RGB888)
	LCDTYPE_ENUM_SerialRGB_8BITS,       //< Serial RGB 8 bits
	LCDTYPE_ENUM_SerialRGB_6BITS,       //< Serial RGB 6 bits
	LCDTYPE_ENUM_SerialYCbCr_8BITS,     //< Serial YCbCr 8 bits
	LCDTYPE_ENUM_RGB_16BITS,            //< RGB 16 bits
	LCDTYPE_ENUM_MIPI_DSI,              //< MIPI DSI

	ENUM_DUMMY4WORD(LCDTYPE_ENUM)
} LCDTYPE_ENUM;

/*
	PLCD_DATA_WIDTH_ENUM
*/
typedef enum
{
	PLCD_DATA_WIDTH_RGB888,             //< RGB888
	PLCD_DATA_WIDTH_RGB666,             //< RGB666
	PLCD_DATA_WIDTH_RGB565,             //< RGB565

	ENUM_DUMMY4WORD(PLCD_DATA_WIDTH_ENUM)
} PLCD_DATA_WIDTH_ENUM;

/*
	CCIR_DATA_WIDTH_ENUM
*/
typedef enum
{
	CCIR_DATA_WIDTH_8BITS,              //< CCIR 8 bits
	CCIR_DATA_WIDTH_16BITS,             //< CCIR 16 bits

	ENUM_DUMMY4WORD(CCIR_DATA_WIDTH_ENUM)
} CCIR_DATA_WIDTH_ENUM;

/*
	MEMIF_TYPE_ENUM
*/
typedef enum
{
	MEMIF_TYPE_GPIO,                    //< GPIO
	MEMIF_TYPE_SERIAL,                  //< Select serial MI
	MEMIF_TYPE_PARALLEL,                //< Select parallel MI

	ENUM_DUMMY4WORD(MEMIF_TYPE_ENUM)
} MEMIF_TYPE_ENUM;

/*
	PMEM_SEL_ENUM
*/
typedef enum
{
	PMEM_SEL_ENUM_1ST_PINMUX,           //< Primary Parallel MI pinmux (up to 18 bits)
	PMEM_SEL_ENUM_2ND_PINMUX,           //< Secondary Parallel MI pinmux (up to 9 bits)

	ENUM_DUMMY4WORD(PMEM_SEL_ENUM)
} PMEM_SEL_ENUM;

/*
	PMEMIF_DATA_WIDTH_ENUM
*/
typedef enum
{
	PMEMIF_DATA_WIDTH_8BITS,            //< Parallel MI bus width is 8 bits
	PMEMIF_DATA_WIDTH_9BITS,            //< Parallel MI bus width is 9 bits
	PMEMIF_DATA_WIDTH_16BITS,           //< Parallel MI bus width is 16 bits
	PMEMIF_DATA_WIDTH_18BITS,           //< Parallel MI bus width is 18 bits

	ENUM_DUMMY4WORD(PMEMIF_DATA_WIDTH_ENUM)
} PMEMIF_DATA_WIDTH_ENUM;

/*
	SMEMIF_DATA_WIDTH_ENUM
*/
typedef enum
{
	SMEMIF_DATA_WIDTH_1BITS,            //< Serial MI bus width is 1 bit
	SMEMIF_DATA_WIDTH_2BITS,            //< Serial MI bus width is 2 bits

	ENUM_DUMMY4WORD(SMEMIF_DATA_WIDTH_ENUM)
} SMEMIF_DATA_WIDTH_ENUM;

/*
	PLCD_DE_ENUM
*/
typedef enum
{
	PLCD_DE_ENUM_GPIO,                  //< GPIO
	PLCD_DE_ENUM_DE,                    //< DE

	ENUM_DUMMY4WORD(PLCD_DE_ENUM)
} PLCD_DE_ENUM;

/*
	CCIR_HVLD_VVLD_ENUM
*/
typedef enum
{
	CCIR_HVLD_VVLD_GPIO,                //< GPIO
	CCIR_HVLD_VVLD_EN,                  //< HVLD/VVLD enable

	ENUM_DUMMY4WORD(CCIR_HVLD_VVLD_ENUM)
} CCIR_HVLD_VVLD_ENUM;

/*
	CCIR_FIELD_ENUM
*/
typedef enum
{
	CCIR_FIELD_GPIO,                    //< GPIO
	CCIR_FIELD_EN,                      //< FIELD enable

	ENUM_DUMMY4WORD(CCIR_FIELD_ENUM)
} CCIR_FIELD_ENUM;

/*
	TE_SEL_ENUM
*/
typedef enum
{
	TE_SEL_GPIO,                        //< GPIO
	TE_SEL_EN,                          //< TE enable

	ENUM_DUMMY4WORD(TE_SEL_ENUM)
} TE_SEL_ENUM;

/*
	LCDTYPE2_ENUM
*/
typedef enum
{
	LCDTYPE2_ENUM_GPIO,                 //< GPIO
	LCDTYPE2_ENUM_CCIR656,              //< CCIR656
	LCDTYPE2_ENUM_CCIR601,              //< CCIR601
	LCDTYPE2_ENUM_SerialRGB_8BITS=4,    //< Serial RGB 8 bits
	LCDTYPE2_ENUM_SerialRGB_6BITS,      //< Serial RGB 6 bits
	LCDTYPE2_ENUM_SerialYCbCr_8BITS,    //< Serial YCbCr 8 bits
	LCDTYPE2_ENUM_ParallelMI,           //< Parallel MI
	LCDTYPE2_ENUM_SerialMI,             //< Serial MI

	ENUM_DUMMY4WORD(LCDTYPE2_ENUM)
} LCDTYPE2_ENUM;

/*
	PMEMIF2_DATA_WIDTH_ENUM
*/
typedef enum
{
	PMEMIF2_DATA_WIDTH_8BITS,           //< Parallel MI 2 bus width is 8 bits
	PMEMIF2_DATA_WIDTH_9BITS,           //< Parallel MI 2 bus width is 9 bits

	ENUM_DUMMY4WORD(PMEMIF2_DATA_WIDTH_ENUM)
} PMEMIF2_DATA_WIDTH_ENUM;

/*
	SMEMIF2_DATA_WIDTH_ENUM
*/
typedef enum
{
	SMEMIF2_DATA_WIDTH_1BITS,           //< Serial MI 2 bus width is 1 bit
	SMEMIF2_DATA_WIDTH_2BITS,           //< Serial MI 2 bus width is 2 bits

	ENUM_DUMMY4WORD(SMEMIF2_DATA_WIDTH_ENUM)
} SMEMIF2_DATA_WIDTH_ENUM;

/*
	CEC_ENUM
*/
typedef enum
{
	CEC_ENUM_GPIO,                      //< GPIO
	CEC_ENUM_CEC,                       //< CEC

	ENUM_DUMMY4WORD(CEC_ENUM)
} CEC_ENUM;

/*
	DDC_ENUM
*/
typedef enum
{
	DDC_ENUM_GPIO,                      //< GPIO
	DDC_ENUM_DDC,                       //< DDC

	ENUM_DUMMY4WORD(DDC_ENUM)
} DDC_ENUM;

/*
	HOTPLUG_ENUM
*/
typedef enum
{
	HOTPLUG_ENUM_GPIO,                  //< GPIO
	HOTPLUG_HOTPLUG,                    //< HOTPLUG

	ENUM_DUMMY4WORD(HOTPLUG_ENUM)
} HOTPLUG_ENUM;

/*
	SENSOR_ENUM
*/
typedef enum
{
	SENSOR_ENUM_GPIO,                   //< GPIO
	SENSOR_ENUM_8BITS,                  //< 8 bits sensor
	SENSOR_ENUM_10BITS,                 //< 10 bits sensor
	SENSOR_ENUM_12BITS,                 //< 12 bits sensor
	SENSOR_ENUM_16BITS,                 //< 16 bits sensor

	ENUM_DUMMY4WORD(SENSOR_ENUM)
} SENSOR_ENUM;

/*
	SENSOR2_ENUM
*/
typedef enum
{
	SENSOR2_ENUM_GPIO,                   //< GPIO
	SENSOR2_ENUM_CCIR8BITS,              //< CCIR 8 bits sensor
	SENSOR2_ENUM_10BITS,                 //< 10 bit sensor
	SENSOR2_ENUM_CCIR16BITS,             //< CCIR 16 bits sensor

	ENUM_DUMMY4WORD(SENSOR2_ENUM)
} SENSOR2_ENUM;

/*
	SENSOR3_ENUM
*/
typedef enum
{
	SENSOR3_ENUM_GPIO,                   //< GPIO
	SENSOR3_ENUM_8BITS,                  //< 8 bits sensor

	ENUM_DUMMY4WORD(SENSOR3_ENUM)
} SENSOR3_ENUM;

/*
	SENSOR4_ENUM
*/
typedef enum
{
	SENSOR4_ENUM_GPIO,                   //< GPIO
	SENSOR4_ENUM_8BITS,                  //< 8 bits sensor

	ENUM_DUMMY4WORD(SENSOR4_ENUM)
} SENSOR4_ENUM;

/*
	SENSOR6_ENUM
*/
typedef enum
{
	SENSOR6_ENUM_GPIO,                   //< GPIO
	SENSOR6_ENUM_8BITS,                  //< 8 bits sensor
	SENSOR6_ENUM_16BITS,                 //< 16 bits sensor

	ENUM_DUMMY4WORD(SENSOR6_ENUM)
} SENSOR6_ENUM;

/*
	SENSOR7_ENUM
*/
typedef enum
{
	SENSOR7_ENUM_GPIO,                   //< GPIO
	SENSOR7_ENUM_8BITS,                  //< 8 bits sensor

	ENUM_DUMMY4WORD(SENSOR7_ENUM)
} SENSOR7_ENUM;

/*
	SENSOR8_ENUM
*/
typedef enum
{
	SENSOR8_ENUM_GPIO,                   //< GPIO
	SENSOR8_ENUM_8BITS,                  //< 8 bits sensor

	ENUM_DUMMY4WORD(SENSOR8_ENUM)
} SENSOR8_ENUM;

/*
	PXCLK_SEL_ENUM
*/
typedef enum
{
	PXCLK_SEL_ENUM_GPIO,                //< Select as GPIO
	PXCLK_SEL_ENUM_PXCLK,               //< Select as PXCLK

	ENUM_DUMMY4WORD(PXCLK_SEL_ENUM)
} PXCLK_SEL_ENUM;

/*
	VD_HD_SEL_ENUM
*/
typedef enum
{
	VD_HD_SEL_ENUM_GPIO,                //< Select as GPIO
	VD_HD_SEL_ENUM_SIE_VDHD,            //< Mux VD/HD to SIE
	VD_HD_SEL_ENUM_LVDS_VDHD,           //< Mux VD/HD to LVDS

	ENUM_DUMMY4WORD(VD_HD_SEL_ENUM)
} VD_HD_SEL_ENUM;

/*
	XVS_XHS_ENUM
*/
typedef enum {
	XVS_XHS_SEL_ENUM_GPIO,                //< Select as GPIO
	XVS_XHS_SEL_ENUM_XVS_XHS,             //< Select as XVS_XHS

	ENUM_DUMMY4WORD(XVS_XHS_SEL_ENUM)
} XVS_XHS_SEL_ENUM;

/*
	XVS_XHS_PINMUX
*/
typedef enum
{
	XVS_XHS_1ST_PINMUX,                 //< Select 1st pinmux pad
	XVS_XHS_2ND_PINMUX,                 //< Select 2nd pinmux pad

	ENUM_DUMMY4WORD(XVS_XHS_PINMUX)
} XVS_XHS_PINMUX;

/*
	SENMCLK_SEL_ENUM
*/
typedef enum
{
	SENMCLK_SEL_ENUM_GPIO,              //< Select as GPIO
	SENMCLK_SEL_ENUM_MCLK,              //< Select as MCLK

	ENUM_DUMMY4WORD(SENMCLK_SEL_ENUM)
} SENMCLK_SEL_ENUM;

/*
	MES_SEL_ENUM
*/
typedef enum
{
	MES_SEL_ENUM_GPIO,                  //< Select as GPIO
	MES_SEL_ENUM_1ST_PINMUX,            //< Select as MES at 1st pinmux pad
	MES_SEL_ENUM_2ND_PINMUX,            //< Select as MES at 2nd pinmux pad

	ENUM_DUMMY4WORD(MES_SEL_ENUM)
} MES_SEL_ENUM;

/*
	FLCTR_SEL_ENUM
*/
typedef enum
{
	FLCTR_SEL_ENUM_GPIO,                //< Select as GPIO
	FLCTR_SEL_ENUM_FLCTR,               //< Select as FLCTR

	ENUM_DUMMY4WORD(FLCTR_SEL_ENUM)
} FLCTR_SEL_ENUM;

/*
	SP_CLK_SEL_ENUM
*/
typedef enum
{
	SP_CLK_SEL_ENUM_GPIO,               //< Select as GPIO
	SP_CLK_SEL_ENUM_SPCLK,              //< Select as SP_CLK
	SP_CLK_SEL_ENUM_SP_2_CLK,           //< Select as SP_2_CLK
	SP_CLK_SEL_ENUM_SP_3_CLK,           //< Select as SP_3_CLK

	ENUM_DUMMY4WORD(SP_CLK_SEL_ENUM)
} SP_CLK_SEL_ENUM;

/*
	SP2_CLK_SEL_ENUM
*/
typedef enum
{
	SP2_CLK_SEL_ENUM_GPIO,              //< Select as GPIO
	SP2_CLK_SEL_ENUM_SP2CLK,            //< Select as SP2_CLK
	SP2_CLK_SEL_ENUM_SP2_2_CLK,         //< Select as SP2_2_CLK

	ENUM_DUMMY4WORD(SP2_CLK_SEL_ENUM)
} SP2_CLK_SEL_ENUM;

/*
	SHUTTER_ENUM
*/
typedef enum
{
	SHUTTER_ENUM_GPIO,                  //< Select as GPIO
	SHUTTER_ENUM_SHUTTER,               //< Select as SHUTTER

	ENUM_DUMMY4WORD(SHUTTER_ENUM)
} SHUTTER_ENUM;

/*
	STROBE_ENUM
*/
typedef enum
{
	STROBE_ENUM_GPIO,                   //< Select as GPIO
	STROBE_ENUM_STROBE,                 //< Select as STROBE

	ENUM_DUMMY4WORD(STROBE_ENUM)
} STROBE_ENUM;

/*
	MIPI_LVDS_CLK_ENUM
*/
typedef enum
{
	MIPI_LVDS_CLK_ENUM_GPI,             //< Select as GPI
	MIPI_LVDS_CLK_ENUM_CLK,             //< Select as Clock Lane

	ENUM_DUMMY4WORD(MIPI_LVDS_CLK_ENUM)
} MIPI_LVDS_CLK_ENUM;

/*
	MIPI_LVDS_DATA_ENUM
*/
typedef enum
{
	MIPI_LVDS_DATA_ENUM_GPI,            //< Select as GPI
	MIPI_LVDS_DATA_ENUM_DAT,            //< Select as Data Lane

	ENUM_DUMMY4WORD(MIPI_LVDS_DATA_ENUM)
} MIPI_LVDS_DATA_ENUM;

/*
	I2C_ENUM
*/
typedef enum
{
	I2C_ENUM_GPIO,                      //< Select as GPIO
	I2C_ENUM_I2C,                       //< Select as I2C

	ENUM_DUMMY4WORD(I2C_ENUM)
} I2C_ENUM;

/*
	I2C_2_ENUM
*/
typedef enum
{
	I2C_2_ENUM_GPIO,                    //< Select as GPIO
	I2C_2_ENUM_I2C,                     //< Select as I2C

	ENUM_DUMMY4WORD(I2C_2_ENUM)
} I2C_2_ENUM;

/*
	SPI_ENUM
*/
typedef enum
{
	SPI_DISABLE,                       //< Select as GPIO
	SPI_1ST_PINMUX,                    //< Select SPI on 1st pinmux pad
	SPI_2ND_PINMUX,                    //< Select SPI on 2nd pinmux pad

	ENUM_DUMMY4WORD(SPI_ENUM)
} SPI_ENUM;


/*
	SPI2_ENUM
*/
typedef enum
{
	SPI2_DISABLE,                       //< Select as GPIO
	SPI2_1ST_PINMUX,                    //< Select SPI2 on 1st pinmux pad
	SPI2_2ND_PINMUX,                    //< Select SPI2 on 2nd pinmux pad
	SPI2_3RD_PINMUX,                    //< Select SPI2 on 3rd pinmux pad

	ENUM_DUMMY4WORD(SPI2_ENUM)
} SPI2_ENUM;

/*
	SPI3_ENUM
*/
typedef enum
{
	SPI3_DISABLE,                       //< Select as GPIO
	SPI3_1ST_PINMUX,                    //< Select SPI3 on 1st pinmux pad
	SPI3_2ND_PINMUX,                    //< Select SPI3 on 2nd pinmux pad
	SPI3_3RD_PINMUX,                    //< Select SPI3 on 3rd pinmux pad

	ENUM_DUMMY4WORD(SPI3_ENUM)
} SPI3_ENUM;

/*
	SPI4_ENUM
*/
typedef enum
{
	SPI4_DISABLE,                       //< Select as GPIO
	SPI4_1ST_PINMUX,                    //< Select SPI4 on 1st pinmux pad
	SPI4_2ND_PINMUX,                    //< Select SPI4 on 2nd pinmux pad
	SPI4_3RD_PINMUX,                    //< Select SPI4 on 3rd pinmux pad

	ENUM_DUMMY4WORD(SPI4_ENUM)
} SPI4_ENUM;

/*
	SPI_RDY_ENUM
*/
typedef enum
{
	SPI_RDY_DISABLE,                   //< Select as GPIO
	SPI_RDY_1ST_PINMUX,                //< Select SPI on 1st pinmux pad
	SPI_RDY_2ND_PINMUX,                //< Select SPI on 2nd pinmux pad

	ENUM_DUMMY4WORD(SPI_RDY_ENUM)
} SPI_RDY_ENUM;

/*
	SPI5_ENUM
*/
typedef enum
{
	SPI5_DISABLE,                       //< Select as GPIO
	SPI5_1ST_PINMUX,                    //< Select SPI5 on 1st pinmux pad
	SPI5_2ND_PINMUX,                    //< Select SPI5 on 2nd pinmux pad

	ENUM_DUMMY4WORD(SPI5_ENUM)
} SPI5_ENUM;

/*
	SPI_DAT_ENUM
*/
typedef enum
{
	SPI_DAT_ENUM_1Bit,                  //< Select SPI data is 1 bit
	SPI_DAT_ENUM_2Bit,                  //< Select SPI data is 2 bits
	SPI_DAT_ENUM_4Bit,                  //< Select SPI data is 4 bits

	ENUM_DUMMY4WORD(SPI_DAT_ENUM)
} SPI_DAT_ENUM;

/*
	UART_ENUM
*/
typedef enum
{
	UART_ENUM_GPIO,                     //< Select as GPIO
	UART_ENUM_UART,                     //< Select as UART

	ENUM_DUMMY4WORD(UART_ENUM)
} UART_ENUM;

/*
	UART2_ENUM

	@note For UART2
*/
typedef enum
{
	UART2_ENUM_GPIO,                    //< Select as GPIO
	UART2_ENUM_1ST_PINMUX,              //< Select as UART2_1
	UART2_ENUM_2ND_PINMUX,              //< Select as UART2_2

	ENUM_DUMMY4WORD(UART2_ENUM)
} UART2_ENUM;

/*
	UART3_ENUM

	@note For UART3
*/
typedef enum
{
	UART3_ENUM_GPIO,                    //< Select as GPIO
	UART3_ENUM_1ST_PINMUX,              //< Select as UART3 1ST
	UART3_ENUM_2ND_PINMUX,              //< Select as UART3 2ND

	ENUM_DUMMY4WORD(UART3_ENUM)
} UART3_ENUM;


/*
	UART4_ENUM

	@note For UART4
*/
typedef enum
{
	UART4_ENUM_GPIO,                    //< Select as GPIO
	UART4_ENUM_1ST_PINMUX,              //< Select as UART4 1ST
	UART4_ENUM_2ND_PINMUX,              //< Select as UART4 2ND

	ENUM_DUMMY4WORD(UART4_ENUM)
} UART4_ENUM;


/*
	UART_CTSRTS_ENUM

	@note for UART2~4
*/
typedef enum
{
	UART_CTSRTS_GPIO,                   //< Select as UART without flow control
	UART_CTSRTS_PINMUX,                 //< Select as UART with flow control

	ENUM_DUMMY4WORD(UART_CTSRTS_ENUM)
} UART_CTSRTS_ENUM;

/*
	SIFCH_ENUM

	@note For SIF CH0, 1, 2~5
*/
typedef enum
{
	SIFCH_ENUM_GPIO,                    //< Select as GPIO
	SIFCH_ENUM_SIF,                     //< Select as SIF
	SIFCH_ENUM_SIF_2ND_PINMUX,          //< Select as SIF 2nd
	SIFCH_ENUM_SIF_3RD_PINMUX,          //< Select as SIF 3rd

	ENUM_DUMMY4WORD(SIFCH_ENUM)
} SIFCH_ENUM;

/*
	AUDIO_ENUM
*/
typedef enum
{
	AUDIO_ENUM_GPIO,                    //< Select as GPIO
	AUDIO_ENUM_I2S,                     //< Select as I2S (on UART2_TX/UART2_RX/UART2_RTS/UART2_CTS)
	AUDIO_ENUM_I2S_2ND_PINMUX,          //< Backward compatible

	ENUM_DUMMY4WORD(AUDIO_ENUM)
} AUDIO_ENUM;

/*
	AUDIO_MCLK_ENUM
*/
typedef enum
{
	AUDIO_MCLK_GPIO,                    //< Select as GPIO
	AUDIO_MCLK_MCLK,                    //< Select as audio MCLK
	AUDIO_MCLK_MCLK_2ND_PINMUX,         //< Backward compatible

	ENUM_DUMMY4WORD(AUDIO_MCLK_ENUM)
} AUDIO_MCLK_ENUM;

/*
	REMOTE_ENUM
*/
typedef enum
{
	REMOTE_ENUM_GPIO,                   //< Select as GPIO
	REMOTE_ENUM_REMOTE,                 //< Select as REMOTE

	ENUM_DUMMY4WORD(REMOTE_ENUM)
} REMOTE_ENUM;


/*
	EJTAG_SEL_ENUM
*/
typedef enum
{
	EJTAG_SEL_ENUM_EJTAG_CPU1,          //< Select as EJTAG only CPU1
	EJTAG_SEL_ENUM_EJTAG_DAISYCHAIN,    //< Select as EJTAG Daisy chain

	ENUM_DUMMY4WORD(EJTAG_SEL_ENUM)
} EJTAG_SEL_ENUM;

/*
	EJTAG_DSP_SEL_ENUM
*/
typedef enum
{
	EJTAG_DSP_SEL_ENUM_1ST_PINMUX,     //< Select as DSP EJTAG as 1'st pinmux
	EJTAG_DSP_SEL_ENUM_2ND_PINMUX,     //< Select as DSP EJTAG as 2'nd pinmux

	ENUM_DUMMY4WORD(EJTAG_DSP_SEL_ENUM)
} EJTAG_DSP_SEL_ENUM;


/*
	PWM_ENUM
*/
typedef enum
{
	PWM_ENUM_GPIO,                      //< Select as GPIO
	PWM_ENUM_PWM,                       //< Select as PWM

	ENUM_DUMMY4WORD(PWM_ENUM)
} PWM_ENUM;

/*
	PWM_SEL_ENUM
*/
typedef enum
{
	PWM_SEL_ENUM_1ST_PINMUX,            //< Select as PWMx_1
	PWM_SEL_ENUM_2ND_PINMUX,            //< Select as PWMx_2

	ENUM_DUMMY4WORD(PWM_SEL_ENUM)
} PWM_SEL_ENUM;

/*
	PICNT_ENUM
*/
typedef enum
{
	PICNT_ENUM_GPIO,                    //< Select as GPIO
	PICNT_ENUM_PICNT,                   //< Select as PI_CNT
	PICNT_ENUM_PICNT2,                  //< Select as PI_CNT2

	ENUM_DUMMY4WORD(PICNT_ENUM)
} PICNT_ENUM;

/*
	DAC_CLK_ENUM
*/
typedef enum
{
	DAC_CLK_ENUM_GPIO,                  //< Select as GPIO
	DAC_CLK_ENUM_DAC_CLK,               //< Select as DAC_CLK

	ENUM_DUMMY4WORD(DAC_CLK_ENUM)
} DAC_CLK_ENUM;

/*
	USB_VBUSI_ENUM
*/
typedef enum
{
	USB_VBUSI_ENUM_INACTIVE,            //< Inactive (VBUSI is low)
	USB_VBUSI_ENUM_ACTIVE,              //< Active (VBUSI is high)

	ENUM_DUMMY4WORD(USB_VBUSI_ENUM)
} USB_VBUSI_ENUM;

/*
	USB_ID_ENUM
*/
typedef enum
{
	USB_ID_ENUM_HOST,                   //< VBUS ID is host
	USB_ID_ENUM_DEVICE,                 //< VBUS ID is device

	ENUM_DUMMY4WORD(USB_ID_ENUM)
} USB_ID_ENUM;

/*
	ETH_ID_ENUM
*/
typedef enum
{
	ETH_ID_ENUM_GPIO,
	ETH_ID_ENUM_MII,
	ETH_ID_ENUM_RMII,
	ETH_ID_ENUM_GMII,
	ETH_ID_ENUM_RGMII,
	ETH_ID_ENUM_REVMII_10_100,
	ETH_ID_ENUM_REVMII_10_1000,
	ENUM_DUMMY4WORD(ETH_ID_ENUM)
} ETH_ID_ENUM;

/*
	LOCKN_ENUM
*/
typedef enum
{
	LOCKN_ENUM_GPIO,                   //< GPIO
	LOCKN_ENUM_LOCKN,                  //< LOCKN

	ENUM_DUMMY4WORD(LOCKN_ENUM)
} LOCKN_ENUM;

/*
	CANBUS_ENUM
*/
typedef enum
{
	CANBUS_ENUM_GPIO,                   //< GPIO
	CANBUS_ENUM_CANBUS,                 //< CANBUS

	ENUM_DUMMY4WORD(CANBUS_ENUM)
} CANBUS_ENUM;

/*
	TV_TEST_CLK_ENUM
*/
typedef enum
{
	TV_TEST_CLK_GPIO,                   //< GPIO
	TV_TEST_CLK_TV_CLK,                 //< TV TEST CLK

	ENUM_DUMMY4WORD(TV_TEST_CLK_ENUM)
} TV_TEST_CLK_ENUM;

/*
	TRACE
*/
typedef enum
{
	TRACE_ENUM_GPIO,                    //< GPIO
	TRACE_ENUM_TRACE,                   //< TRACE

	ENUM_DUMMY4WORD(TRACE_ENUM)
} TRACE_ENUM;

/*
	GPIO_ID_ENUM
*/
typedef enum
{
	GPIO_ID_EMUM_FUNC,                  //< pinmux is mapping to function
	GPIO_ID_EMUM_GPIO,                  //< pinmux is mapping to gpio

	ENUM_DUMMY4WORD(GPIO_ID_ENUM)
} GPIO_ID_ENUM;

/**
	NAND configuration identifier

	@note For nand_setConfig()
*/
typedef enum
{
	NAND_CONFIG_ID_FREQ,                    ///< NAND module clock (Unit: MHz), MUST config before storage object open, active after open
					        ///< Context can be one of the following:
					        ///< - @b 48    : 48 MHz (Default value)
					        ///< - @b 60    : 60 MHz
					        ///< - @b 96    : 96 MHz
	NAND_CONFIG_ID_TIMING0,                 ///< NAND controller timing0 register, MUST config before storage object open and need config timing1 at the same time
                                                ///< Context is UINT32 value
	NAND_CONFIG_ID_TIMING1,                 ///< NAND controller timing1 register, MUST config before storage object open and need config timing0 at the same time
                                                ///< Context is UINT32 value

	NAND_CONFIG_ID_DELAY_LATCH,             ///< Context can be one of the following:
                                                ///< - @b TRUE  : 0.5T delay latch
                                                ///< - @b FALSE : 0.0T delay latch
                                                ///<   @note: TRUE equal to 0.5 tRP

	NAND_CONFIG_ID_SET_INTEN,               ///< Context can be one of the following:
                                                ///< - @b TRUE  : Enable interrupt mode
                                                ///< - @b FALSE : Disable interrupt mode
                                                ///<   @note: Need config before any access of storage object hook on NAND device open\n
                                                ///<          such as STRG_SET_PARTITION_SIZE, STRG_SET_PARTITION_RSV_SIZE, \n
                                                ///<          STRG_GET_BEST_ACCESS_SIZE, STRG_GET_SECTOR_SIZE, STRG_GET_DEVICE_PHY_SIZE,\n
                                                ///<          STRG_GET_MEMORY_SIZE_REQ
                                                ///< @code
                                                ///     if(nand_setConfig(NAND_CONFIG_ID_SET_INTEN, TRUE) != E_OK)
                                                ///     {
                                                ///         ERROR;
                                                ///     }
                                                ///     //Enable Interrupt mode
                                                //      if(nand_setConfig(NAND_CONFIG_ID_SET_INTEN, FALSE) != E_OK)
                                                ///     {
                                                ///         ERROR;
                                                ///     }
                                                ///     //Disable Interrupt mode
                                                ///  }
                                                ///  @endcode

	NAND_CONFIG_ID_AUTOPINMUX,              ///< Context can be one of the following:
                                                ///< - @b TRUE  : disable pinmux when NAND driver close
                                                ///< - @b FALSE : not disable pinmux when NAND driver close

	NAND_CONFIG_ID_NAND_TYPE,               ///< Context can be one of the following:
                                                ///< - @b NANDCTRL_ONFI_NAND_TYPE   : ONFI NAND via NAND controller
                                                ///< - @b NANDCTRL_SPI_NAND_TYPE    :  SPI NAND via NAND controller
                                                ///< - @b NANDCTRL_SPI_NOR_TYPE     :  SPI NOR  via NAND controller
                                                ///< - @b SPICTRL_SPI_NAND_TYPE     :  SPI NAND via  SPI controller

	NAND_CONFIG_ID_SPI_ECC_TYPE,            ///< Context can be one of the following(only available when device is SPI NAND flash):
                                                ///< - @b NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC : SPI NAND use nand controller reedsolomon ecc
                                                ///< - @b NANDCTRL_SPIFLASH_USE_ONDIE_ECC       : SPI NAND use nand on die ecc(depend on each spi nand flash)

	NAND_CONFIG_ID_SPI_SEC_ECC_EN,          ///< Context can be one of the following(only available when device is SPI NAND flash):
                                                ///< - @b TRUE  : Secondary ECC enable(only available when NAND_CONFIG_ID_SPI_ECC_TYPE = NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC)
                                                ///< - @b FALSE : Secondary ECC disable

	NAND_CONFIG_ID_SPI_OPERATION_BIT_MODE,  ///< Context can be one of the following(only available when device is SPI NAND flash):
                                                ///< - @b NANDCTRL_SPIFLASH_USE_1_BIT : SPI NAND 1 bit operation mode
                                                ///< - @b NANDCTRL_SPIFLASH_USE_2_BIT : SPI NAND 2 bit operation mode(dual mode)
                                                ///< - @b NANDCTRL_SPIFLASH_USE_4_BIT : SPI NAND 4 bit operation mode(quad mode)

	ENUM_DUMMY4WORD(NAND_CONFIG_ID)
} NAND_CONFIG_ID;


typedef enum
{
	NANDCTRL_ONFI_NAND_TYPE = 0x0,
	NANDCTRL_SPI_NAND_TYPE,
	NANDCTRL_SPI_NOR_TYPE,
	SPICTRL_SPI_NAND_TYPE,

	NAND_TYPE_NUM,
	ENUM_DUMMY4WORD(NAND_TYPE_SEL)
} NAND_TYPE_SEL;

#define TOP_SETREG(info, ofs,value)	   OUTW(info->top_base +(ofs),(value))
#define TOP_GETREG(info, ofs)		   INW(info->top_base+(ofs))

#define TGE_REG_ADDR(ofs)       (NVT_TGE_BASE_VIRT+(ofs))
#define TGE_GETREG(ofs)         INW(TGE_REG_ADDR(ofs))
#define TGE_SETREG(ofs,value)   OUTW(TGE_REG_ADDR(ofs), (value))
#define TGE_CONTROL_OFS 0x0

#define MAX_PAD_NUM  236

#define GPIO_SETREG(info, ofs,value)	   OUTW(info->gpio_base +(ofs),(value))
#define GPIO_GETREG(info, ofs)		   INW(info->gpio_base+(ofs))

struct nvt_pad_info {
	u32 pad_ds_pin;
	u32 driving;
	u32 pad_gpio_pin;
	u32 direction;
};

struct nvt_gpio_info {
	u32 gpio_pin;
	u32 direction;
};

struct nvt_pinctrl_info {
	void __iomem *top_base;
	void __iomem *pad_base;
	void __iomem *gpio_base;
	PIN_GROUP_CONFIG top_pinmux[PIN_FUNC_MAX];
	struct nvt_pad_info pad[MAX_PAD_NUM];
};

#define MAX_MODULE_NAME 9

ER pinmux_init(struct nvt_pinctrl_info *info);
ER pad_init(struct nvt_pinctrl_info *info, u32 nr_pad);
void pinmux_parsing(struct nvt_pinctrl_info *info);
int nvt_pinmux_proc_init(void);
void pinmux_preset(struct nvt_pinctrl_info *info);
int pinmux_set_host(struct nvt_pinctrl_info *info, PINMUX_FUNC_ID id, u32 pinmux);
void pinmux_gpio_parsing(struct nvt_pinctrl_info *info);
#endif

