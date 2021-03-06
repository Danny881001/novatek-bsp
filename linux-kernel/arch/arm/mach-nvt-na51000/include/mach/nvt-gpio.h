/**
    GPIO header file
    This file will setup gpio related base address
    @file       nvt-gpio.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef __ASM_ARCH_NA51000_NVT_GPIO_H
#define __ASM_ARCH_NA51000_NVT_GPIO_H

#include <linux/types.h>
#include <linux/irq.h>

#define NVT_GPIO_BASE 0xF0070000

#ifdef CONFIG_NT96680_FPGA_EMULATION_S1
#define NVT_GPIO_STG_DATA_0		(NVT_GPIO_BASE + 0)
#define NVT_GPIO_STG_DATA_1		(NVT_GPIO_BASE + 0x4)
#define NVT_GPIO_PER_DATA_0		(NVT_GPIO_BASE + 0x8)
#define NVT_GPIO_PER_DATA_1		(NVT_GPIO_BASE + 0xc)
#define NVT_GPIO_SEN_DATA		(NVT_GPIO_BASE + 0x10)
#define NVT_GPIO_LCD_DATA		(NVT_GPIO_BASE + 0x14)
#define NVT_GPIO_GPI_HSI_DATA		(NVT_GPIO_BASE + 0x18)
#define NVT_DGPIO_DATA			(NVT_GPIO_BASE + 0x1c)
#define NVT_GPIO_STG_DIR_0		(NVT_GPIO_BASE + 0x20)
#define NVT_GPIO_STG_DIR_1		(NVT_GPIO_BASE + 0x24)
#define NVT_GPIO_PER_DIR_0		(NVT_GPIO_BASE + 0x28)
#define NVT_GPIO_PER_DIR_1		(NVT_GPIO_BASE + 0x2c)
#define NVT_GPIO_SEN_DIR		(NVT_GPIO_BASE + 0x30)
#define NVT_GPIO_LCD_DIR		(NVT_GPIO_BASE + 0x34)
#define NVT_DGPIO_DIR			(NVT_GPIO_BASE + 0x3c)
#define NVT_GPIO_STG_SET_0		(NVT_GPIO_BASE + 0x40)
#define NVT_GPIO_STG_SET_1		(NVT_GPIO_BASE + 0x44)
#define NVT_GPIO_PER_SET_0		(NVT_GPIO_BASE + 0x48)
#define NVT_GPIO_PER_SET_1		(NVT_GPIO_BASE + 0x4c)
#define NVT_GPIO_SEN_SET		(NVT_GPIO_BASE + 0x50)
#define NVT_GPIO_LCD_SET		(NVT_GPIO_BASE + 0x54)
#define NVT_DGPIO_SET			(NVT_GPIO_BASE + 0x5c)
#define NVT_GPIO_STG_CLR_0		(NVT_GPIO_BASE + 0x60)
#define NVT_GPIO_STG_CLR_1		(NVT_GPIO_BASE + 0x64)
#define NVT_GPIO_PER_CLR_0		(NVT_GPIO_BASE + 0x68)
#define NVT_GPIO_PER_CLR_1		(NVT_GPIO_BASE + 0x6c)
#define NVT_GPIO_SEN_CLR		(NVT_GPIO_BASE + 0x70)
#define NVT_GPIO_LCD_CLR		(NVT_GPIO_BASE + 0x74)
#define NVT_DGPIO_CLR			(NVT_GPIO_BASE + 0x7c)
#define NVT_GPIO_STS_CPU1		(NVT_GPIO_BASE + 0x80)
#define NVT_GPIO_STS_CPU2		(NVT_GPIO_BASE + 0x84)
#define NVT_GPIO_STS_DSP		(NVT_GPIO_BASE + 0x88)
#define NVT_GPIO_INTEN_CPU1		(NVT_GPIO_BASE + 0x90)
#define NVT_GPIO_INTEN_CPU2		(NVT_GPIO_BASE + 0x94)
#define NVT_GPIO_INTEN_DSP		(NVT_GPIO_BASE + 0x98)
#define NVT_GPIO_INT_TYPE		(NVT_GPIO_BASE + 0xA0)
#define NVT_GPIO_INT_POL		(NVT_GPIO_BASE + 0xA4)
#define NVT_GPIO_EDGE_TYPE		(NVT_GPIO_BASE + 0xA8)
#define NVT_GPIO_TO_CPU1		(NVT_GPIO_BASE + 0xB0)
#define NVT_GPIO_TO_CPU2		(NVT_GPIO_BASE + 0xB4)
#define NVT_GPIO_TO_DSP			(NVT_GPIO_BASE + 0xB8)
#define NVT_DGPIO_STS_CPU1		(NVT_GPIO_BASE + 0xC0)
#define NVT_DGPIO_STS_CPU2		(NVT_GPIO_BASE + 0xC4)
#define NVT_DGPIO_STS_DSP		(NVT_GPIO_BASE + 0xC8)
#define NVT_DGPIO_INTEN_CPU1		(NVT_GPIO_BASE + 0xD0)
#define NVT_DGPIO_INTEN_CPU2		(NVT_GPIO_BASE + 0xD4)
#define NVT_DGPIO_INTEN_DSP		(NVT_GPIO_BASE + 0xD8)
#define NVT_DGPIO_INT_TYPE		(NVT_GPIO_BASE + 0xE0)
#define NVT_DGPIO_INT_POL		(NVT_GPIO_BASE + 0xE4)
#define NVT_DGPIO_EDGE_TYPE		(NVT_GPIO_BASE + 0xE8)
#define NVT_DGPIO_TO_CPU1		(NVT_GPIO_BASE + 0xF0)
#define NVT_DGPIO_TO_CPU2		(NVT_GPIO_BASE + 0xF4)
#define NVT_DGPIO_TO_DSP		(NVT_GPIO_BASE + 0xF8)
#else /* !CONFIG_NT96680_FPGA_EMULATION_S1 */
#define NVT_GPIO_STG_DATA_0		(0)
#define NVT_GPIO_STG_DATA_1		(0x4)
#define NVT_GPIO_PER_DATA_0		(0x8)
#define NVT_GPIO_PER_DATA_1		(0xc)
#define NVT_GPIO_SEN_DATA		(0x10)
#define NVT_GPIO_LCD_DATA_0		(0x14)
#define NVT_GPIO_LCD_DATA_1		(0x18)
#define NVT_GPIO_DGPIO_DATA		(0x1c)
#define NVT_GPIO_GPI_HSI_DATA		(0x20)
#define NVT_GPIO_STG_DIR_0		(0x30)
#define NVT_GPIO_STG_DIR_1		(0x34)
#define NVT_GPIO_PER_DIR_0		(0x38)
#define NVT_GPIO_PER_DIR_1		(0x3c)
#define NVT_GPIO_SEN_DIR		(0x40)
#define NVT_GPIO_LCD_DIR_0		(0x44)
#define NVT_GPIO_LCD_DIR_1		(0x48)
#define NVT_DGPIO_DIR			(0x4c)
#define NVT_GPIO_STG_SET_0		(0x50)
#define NVT_GPIO_STG_SET_1		(0x54)
#define NVT_GPIO_PER_SET_0		(0x58)
#define NVT_GPIO_PER_SET_1		(0x5c)
#define NVT_GPIO_SEN_SET		(0x60)
#define NVT_GPIO_LCD_SET_0		(0x64)
#define NVT_GPIO_LCD_SET_1		(0x68)
#define NVT_DGPIO_SET			(0x6c)
#define NVT_GPIO_STG_CLR_0		(0x70)
#define NVT_GPIO_STG_CLR_1		(0x74)
#define NVT_GPIO_PER_CLR_0		(0x78)
#define NVT_GPIO_PER_CLR_1		(0x7c)
#define NVT_GPIO_SEN_CLR		(0x80)
#define NVT_GPIO_LCD_CLR_0		(0x84)
#define NVT_GPIO_LCD_CLR_1		(0x88)
#define NVT_DGPIO_CLR			(0x8c)
#define NVT_GPIO_STS_CPU1		(0x90)
#define NVT_GPIO_STS_CPU2		(0x94)
#define NVT_GPIO_STS_DSP		(0x98)
#define NVT_GPIO_STS_DSP_2		(0x9c)
#define NVT_GPIO_INTEN_CPU1		(0xA0)
#define NVT_GPIO_INTEN_CPU2		(0xA4)
#define NVT_GPIO_INTEN_DSP		(0xA8)
#define NVT_GPIO_INTEN_DSP2		(0xAc)
#define NVT_GPIO_INT_TYPE		(0xB0)
#define NVT_GPIO_INT_POL		(0xB4)
#define NVT_GPIO_EDGE_TYPE		(0xB8)
#define NVT_GPIO_TO_CPU1		(0xC0)
#define NVT_GPIO_TO_CPU2		(0xC4)
#define NVT_GPIO_TO_DSP			(0xC8)
#define NVT_GPIO_TO_DSP2		(0xCC)
#define NVT_DGPIO_STS_CPU1		(0xD0)
#define NVT_DGPIO_STS_CPU2		(0xD4)
#define NVT_DGPIO_STS_DSP		(0xD8)
#define NVT_DGPIO_STS_DSP2		(0xDC)
#define NVT_DGPIO_INTEN_CPU1		(0xE0)
#define NVT_DGPIO_INTEN_CPU2		(0xE4)
#define NVT_DGPIO_INTEN_DSP		(0xE8)
#define NVT_DGPIO_INTEN_DSP2		(0xEC)
#define NVT_DGPIO_INT_TYPE		(0xF0)
#define NVT_DGPIO_INT_POL		(0xF4)
#define NVT_DGPIO_EDGE_TYPE		(0xF8)
#define NVT_DGPIO_TO_CPU1		(0x100)
#define NVT_DGPIO_TO_CPU2		(0x104)
#define NVT_DGPIO_TO_DSP		(0x108)
#define NVT_DGPIO_TO_DSP2		(0x10C)
#endif /* CONFIG_NT96680_FPGA_EMULATION_S1 */

#define NVT_IRQ_GPIO_EXT_START		(IRQ_SPI_END)

/* GPIO pin number translation  */
#define C_GPIO(pin)			(pin)
#define P_GPIO(pin)			(pin + 0x40)
#define S_GPIO(pin)			(pin + 0x80)
#define L_GPIO(pin)			(pin + 0xA0)
#define H_GPIO(pin)			(pin + 0xC0)
#define D_GPIO(pin)			(pin + 0xE0)

#define C_GPIO_NUM			34
#define P_GPIO_NUM			48
#define S_GPIO_NUM			12
#define L_GPIO_NUM			33
#define H_GPIO_NUM			32
#define D_GPIO_NUM			13

/* GPIO total pin number */
#define NVT_GPIO_NUMBER			(256)
#endif /* __ASM_ARCH_NA51000_NVT_GPIO_H */
