/**
    NVT clock management module for NA51000 SoC
    @file na51000-clk.c

    Copyright Novatek Microelectronics Corp. 2017. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/spinlock.h>

#include "mach/ioaddress.h"
#include "mach/cc.h"
#include "mach/cg-reg.h"
#include "nvt-im-clk.h"

#define IOADDR_CG_REG_SIZE 0x12A0

#define DEFAULT_OSC_IN_CLK_FREQ 12000000
#define DEFAULT_PLL_DIV_VALUE   131072
#define DEFAULT_PLL2_RATIO  3244032 /* 0x318000 (297MHz for PLL2) */
#define DEFAULT_PLL3_RATIO  5133653 /* 0x4E5555 (470MHz for DMA) */
#define DEFAULT_PLL4_RATIO  2271914 /* 0x22AAAA (208MHz for SSPLL) */
#define DEFAULT_PLL5_RATIO  4369066 /* 0x42AAAA (400MHz for Sensor) */
#define DEFAULT_PLL6_RATIO  4150613 /* 0x3F5555 (380MHz for IPP) */
#define DEFAULT_PLL7_RATIO  5024426 /* 0x4CAAAA (460MHz for Audio) */
#define DEFAULT_PLL8_RATIO  4587520 /* 0x460000 (420MHz for CPU) */
#define DEFAULT_PLL9_RATIO  5242880 /* 0x500000 (480MHz        ) */
#define DEFAULT_PLL10_RATIO 3276800 /* 0x320000 (300MHz for MCLK) */
#define DEFAULT_PLL11_RATIO 5242880 /* 0x500000 (480MHz for DSI) */
#define DEFAULT_PLL12_RATIO 5461333 /* 0x535555 (500MHz for ETH) */
#define DEFAULT_PLL13_RATIO 4587520 /* 0x460000 (420MHz for H264) */
#define DEFAULT_PLL14_RATIO 2184533 /* 0x215555 (200MHz for DMA2) */

static void __iomem *remap_base = (void __iomem *)IOADDR_CG_REG_BASE;
static spinlock_t cg_lock;

#define NA51000_CG_SETREG(ofs, value) writel((value), (remap_base+(ofs)))
#define NA51000_CG_GETREG(ofs)        readl(remap_base+(ofs))

static struct nvt_fixed_rate_clk na51000_fixed_rate_clk[] __initdata = {
	FIXED_RATE_CONF("osc_in", 12000000),
	FIXED_RATE_CONF("fix480m", 480000000),
	FIXED_RATE_CONF("fix240m", 240000000),
	FIXED_RATE_CONF("fix192m", 192000000),
	FIXED_RATE_CONF("fix160m", 160000000),
	FIXED_RATE_CONF("fix120m", 120000000),
	FIXED_RATE_CONF("fix96m", 96000000),
	FIXED_RATE_CONF("fix80m", 80000000),
	FIXED_RATE_CONF("fix60m", 60000000),
	FIXED_RATE_CONF("fix48m", 48000000),
	FIXED_RATE_CONF("fix12.288m", 12288000),
	FIXED_RATE_CONF("fix11.2896m", 11289600),
	FIXED_RATE_CONF("fix8.192m", 8192000),
	FIXED_RATE_CONF("fix3m", 3000000),
	/* fix11718.75 is for WatchDog Timer */
	FIXED_RATE_CONF("fix11718.75", 12000000 / 1024),
	FIXED_RATE_CONF("fix16M", 16000000),
	FIXED_RATE_CONF("sie1_pxclkpad", 108000000),
	FIXED_RATE_CONF("sie2_pxclkpad", 108000000),
	FIXED_RATE_CONF("sie3_pxclkpad", 108000000),
	FIXED_RATE_CONF("sie4_pxclkpad", 108000000),
	FIXED_RATE_CONF("sie5_pxclkpad", 108000000),
	FIXED_RATE_CONF("sie6_pxclkpad", 108000000),
	FIXED_RATE_CONF("sie7_pxclkpad", 108000000),
	FIXED_RATE_CONF("sie8_pxclkpad", 108000000),
	FIXED_RATE_CONF("vx1_1x", 48000000),
	FIXED_RATE_CONF("vx1_2x", 96000000),
	FIXED_RATE_CONF("vx12_1x", 48000000),
	FIXED_RATE_CONF("vx12_2x", 96000000),
	FIXED_RATE_CONF("PAUTOGATING", 0),
};

static struct nvt_pll_clk na51000_pll[] __initdata = {
	PLL_CONF("pll2", DEFAULT_PLL2_RATIO, CG_PLL2_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT2),
	PLL_CONF("pll3", DEFAULT_PLL3_RATIO, CG_PLL3_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT3),
	PLL_CONF("pll4", DEFAULT_PLL4_RATIO, CG_PLL4_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT4),
	PLL_CONF("pll5", DEFAULT_PLL5_RATIO, CG_PLL5_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT5),
	PLL_CONF("pll6", DEFAULT_PLL6_RATIO, CG_PLL6_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT6),
	PLL_CONF("pll7", DEFAULT_PLL7_RATIO, CG_PLL7_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT7),
	PLL_CONF("pll8", DEFAULT_PLL8_RATIO, CG_PLL8_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT8),
	PLL_CONF("pll9", DEFAULT_PLL9_RATIO, CG_PLL9_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT9),
	PLL_CONF("pll10", DEFAULT_PLL10_RATIO, CG_PLL10_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT10),
	PLL_CONF("pll11", DEFAULT_PLL11_RATIO, CG_PLL11_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT11),
	PLL_CONF("pll12", DEFAULT_PLL12_RATIO, CG_PLL12_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT12),
	PLL_CONF("pll13", DEFAULT_PLL13_RATIO, CG_PLL13_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT13),
	PLL_CONF("pll14", DEFAULT_PLL14_RATIO, CG_PLL14_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT14),
};

static struct nvt_composite_gate_clk na51000_cgate_clk[] __initdata = {
	COMP_GATE_CONF("f0220000.i2c", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT4, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT4, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT22, DO_AUTOGATING),
	COMP_GATE_CONF("f0350000.i2c2", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT5, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT5, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT23, DO_AUTOGATING),
	COMP_GATE_CONF("f03a0000.i2c3", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT31, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT31, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT24, DO_AUTOGATING),
	COMP_GATE_CONF("f03b0000.i2c4", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT26, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT26, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT25, DO_AUTOGATING),
	COMP_GATE_CONF("f03c0000.i2c5", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT27, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT27, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT26, DO_AUTOGATING),
	COMP_GATE_CONF("f0050000.wdt", "fix11718.75", 11718, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT17, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT17, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT1, DO_AUTOGATING),
	COMP_GATE_CONF("f0300000.uart", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT11, DO_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT11, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT29, DO_AUTOGATING),
	COMP_GATE_CONF("f0310000.uart", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT22, DO_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT22, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT16, DO_AUTOGATING),
	COMP_GATE_CONF("f0380000.uart", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT23, DO_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT23, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT17, DO_AUTOGATING),
	COMP_GATE_CONF("f02b0000.eth", "pll12div6", 50000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT29, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT29, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG2_OFFSET, BIT20, DO_AUTOGATING),
	COMP_GATE_CONF("f0260000.adc", "fix16M", 16000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT13, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT13, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT31, DO_AUTOGATING),
	COMP_GATE_CONF("f0230000.spi", "fix192m", 24000000,
		       CG_SPI_CLK_DIV_REG0_OFFSET, BIT0, WID11,
		       CG_CLK_EN_REG1_OFFSET, BIT6, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT6, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT24, DO_AUTOGATING),
	COMP_GATE_CONF("f0320000.spi", "fix192m", 24000000,
		       CG_SPI_CLK_DIV_REG0_OFFSET, BIT16, WID11,
		       CG_CLK_EN_REG1_OFFSET, BIT7, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT7, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT25, DO_AUTOGATING),
	COMP_GATE_CONF("f0340000.spi", "fix192m", 24000000,
		       CG_SPI_CLK_DIV_REG1_OFFSET, BIT0, WID11,
		       CG_CLK_EN_REG1_OFFSET, BIT8, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT8, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT26, DO_AUTOGATING),
	COMP_GATE_CONF("f0360000.spi", "fix192m", 24000000,
		       CG_SPI_CLK_DIV_REG1_OFFSET, BIT16, WID11,
		       CG_CLK_EN_REG1_OFFSET, BIT15, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT15, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG1_OFFSET, BIT18, DO_AUTOGATING),
#ifdef CONFIG_MTD_SPINAND
	COMP_GATE_CONF("f0400000.nand", "fix480m", 80000000,
		       CG_SDIO_CLK_DIV_REG_OFFSET, BIT24, WID6,
		       CG_CLK_EN_REG1_OFFSET, BIT0, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT0, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT19, NOT_AUTOGATING),
#else
	COMP_GATE_CONF("f0400000.nor", "fix480m", 48000000,
		       CG_SDIO_CLK_DIV_REG_OFFSET, BIT24, WID6,
		       CG_CLK_EN_REG1_OFFSET, BIT0, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT0, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT19, NOT_AUTOGATING),
#endif
	COMP_GATE_CONF("f0240000.sif", "fix96m", 96000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT9, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT9, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT27, NOT_AUTOGATING),
	COMP_GATE_CONF("pwm_clk.12", "fix120m", 30000000,
		       CG_PWM_CLK_RATE_REG1_OFFSET, BIT16, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT12, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("pwm_clk.13", "fix120m", 30000000,
		       CG_PWM_CLK_RATE_REG2_OFFSET, BIT0, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT13, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("pwm_clk.14", "fix120m", 30000000,
		       CG_PWM_CLK_RATE_REG2_OFFSET, BIT16, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT14, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("pwm_clk.15", "fix120m", 30000000,
		       CG_PWM_CLK_RATE_REG3_OFFSET, BIT0, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT15, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("pwm_clk.16", "fix120m", 30000000,
		       CG_PWM_CLK_RATE_REG3_OFFSET, BIT16, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT16, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("pwm_clk.17", "fix120m", 30000000,
		       CG_PWM_CLK_RATE_REG4_OFFSET, BIT0, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT17, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("pwm_clk.18", "fix120m", 30000000,
		       CG_PWM_CLK_RATE_REG4_OFFSET, BIT16, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT18, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("pwm_clk.19", "fix120m", 30000000,
		       CG_PWM_CLK_RATE_REG5_OFFSET, BIT0, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT19, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("ccnt_clk.0", "fix3m", 3000000,
		       0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT8, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("ccnt_clk.1", "fix3m", 3000000,
		       0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT9, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("ccnt_clk.2", "fix3m", 3000000,
		       0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT10, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("ccnt_clk.3", "fix3m", 3000000,
		       0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT11, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f02a0000.vx1", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT14, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT21, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f02a0000.vx1ml", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT23, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT14, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f02a0000.vx1sbl", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT20, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT20, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f03f0000.vx1ml", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT24, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT24, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f03f0000.vx1sbl", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT22, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT22, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f02c0000.slvsec", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG4_OFFSET, BIT10, NOT_ENABLE,
		       CG_SYS_RESET_REG4_OFFSET, BIT10, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0270000.lvds", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT1, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT1, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT3, DO_AUTOGATING),
	COMP_GATE_CONF("f0370000.lvds", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT2, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT2, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT4, DO_AUTOGATING),
	COMP_GATE_CONF("f0e00000.lvds", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG3_OFFSET, BIT20, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT20, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0e10000.lvds", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG3_OFFSET, BIT21, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT21, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0e20000.lvds", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG3_OFFSET, BIT22, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT22, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0e30000.lvds", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG3_OFFSET, BIT23, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT23, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0e40000.lvds", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG3_OFFSET, BIT24, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT24, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0e50000.lvds", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG3_OFFSET, BIT25, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT25, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0600000.usb20", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT19, NOT_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT19, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1700000.u3susp", "pll12", 500000000, 0, 0, 0,
		       CG_CLK_EN_REG4_OFFSET, BIT8, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT17, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1700000.u3itp", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG4_OFFSET, BIT9, NOT_ENABLE,
		       CG_AXI_BRI_RESET_REG_OFFSET, BIT20, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0040000.timer", "fix3m", 3000000,
		       0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT18, DO_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT18, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT0, DO_AUTOGATING),
	COMP_GATE_CONF("f0820000.hdmitx", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG0_OFFSET, BIT22, NOT_ENABLE,
		       CG_SYS_RESET_REG0_OFFSET, BIT22, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT13, DO_AUTOGATING),
	COMP_GATE_CONF("f0820000.cec", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG0_OFFSET, BIT21, NOT_ENABLE,
		       CG_SYS_RESET_REG0_OFFSET, BIT21, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0830000.tv", "fix48m", 48000000, 0, 0, 0,
		       CG_CLK_EN_REG0_OFFSET, BIT18, NOT_ENABLE,
		       CG_SYS_RESET_REG0_OFFSET, BIT18, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT5, DO_AUTOGATING),
	COMP_GATE_CONF("f1430000.dsp", "dsp_clk", 0, 0, 0, 0,
		       CG_CLK_EN_REG4_OFFSET, BIT11, NOT_ENABLE,
		       0, 0, 0,
		       0, 0, 0),
	COMP_GATE_CONF("f1430000.rst", "dsp_rst", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_SYS_RESET_REG4_OFFSET, BIT11, DO_RESET,
		       0, 0, 0),		       
	COMP_GATE_CONF("f1430000.nic", "dsp_inc", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_AXI_BRI_RESET_REG_OFFSET, BIT0, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1430000.apbtx", "dsp_apbtx", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_AXI_BRI_RESET_REG_OFFSET, BIT1, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1430000.epp", "dsp_axib_epp", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_AXI_BRI_RESET_REG_OFFSET, BIT2, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1430000.edp", "dsp_axib_edp", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_AXI_BRI_RESET_REG_OFFSET, BIT3, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1430000.chipx", "dsp_chipx", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_CHIP_PART_RESET_REG_OFFSET, BIT0, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1440000.dsp", "dsp_clk", 0, 0, 0, 0,
		       CG_CLK_EN_REG4_OFFSET, BIT12, NOT_ENABLE,
		       0, 0, 0,
		       0, 0, 0),
	COMP_GATE_CONF("f1440000.rst", "dsp_rst", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_SYS_RESET_REG4_OFFSET, BIT12, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1440000.nic", "dsp_inc", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_AXI_BRI_RESET_REG_OFFSET, BIT8, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1440000.apbtx", "dsp_apbtx", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_AXI_BRI_RESET_REG_OFFSET, BIT9, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1440000.epp", "dsp_axib_epp", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_AXI_BRI_RESET_REG_OFFSET, BIT10, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1440000.edp", "dsp_axib_edp", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_AXI_BRI_RESET_REG_OFFSET, BIT11, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f1440000.chipx", "dsp_chipx", 0, 0, 0, 0,
		       0, 0, 0,
		       CG_CHIP_PART_RESET_REG_OFFSET, BIT1, DO_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("f0110000.drtc", "fix12m", 12000000, 0, 0, 0,
		       CG_CLK_EN_REG4_OFFSET, BIT22, NOT_ENABLE,
		       CG_SYS_RESET_REG4_OFFSET, BIT22, NOT_RESET,
		       0, 0, 0),
	COMP_GATE_CONF("pll4_ss", "pll4", 0,
		       CG_PLL4_SPREAD_SPECTRUM_REG0_OFFSET, BIT4, 0,
		       CG_PLL4_SPREAD_SPECTRUM_REG1_OFFSET, BIT4, 0,
		       CG_PLL4_SPREAD_SPECTRUM_REG2_OFFSET, BIT4, 0,
		       CG_PLL4_SPREAD_SPECTRUM_REG3_OFFSET, BIT4, 0),
	COMP_GATE_CONF("pll2_ss", "pll2", 0,
		       CG_PLL2_SPREAD_SPECTRUM_REG0_OFFSET, BIT2, 0,
		       CG_PLL2_SPREAD_SPECTRUM_REG1_OFFSET, BIT2, 0,
		       CG_PLL2_SPREAD_SPECTRUM_REG2_OFFSET, BIT2, 0,
		       CG_PLL2_SPREAD_SPECTRUM_REG3_OFFSET, BIT2, 0),
	COMP_GATE_CONF("pll12_ss", "pll11", 0,
		       CG_PLL12_SPREAD_SPECTRUM_REG0_OFFSET, BIT12, 0,
		       CG_PLL12_SPREAD_SPECTRUM_REG1_OFFSET, BIT12, 0,
		       CG_PLL12_SPREAD_SPECTRUM_REG2_OFFSET, BIT12, 0,
		       CG_PLL12_SPREAD_SPECTRUM_REG3_OFFSET, BIT12, 0),
	COMP_GATE_CONF("ipe_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT2, DO_AUTOGATING),
	COMP_GATE_CONF("ime_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT3, DO_AUTOGATING),
	COMP_GATE_CONF("dis_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT4, DO_AUTOGATING),
	COMP_GATE_CONF("fde_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT5, DO_AUTOGATING),
	COMP_GATE_CONF("dce_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT6, DO_AUTOGATING),
	COMP_GATE_CONF("ife_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT7, DO_AUTOGATING),
	COMP_GATE_CONF("graphic_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT8, DO_AUTOGATING),
	COMP_GATE_CONF("graphic2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT9, DO_AUTOGATING),
	COMP_GATE_CONF("ide_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT10, DO_AUTOGATING),
	COMP_GATE_CONF("ide2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT11, DO_AUTOGATING),
	COMP_GATE_CONF("ife2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT14, DO_AUTOGATING),
	COMP_GATE_CONF("jpeg_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT15, DO_AUTOGATING),
	COMP_GATE_CONF("h264_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT16, DO_AUTOGATING),
	COMP_GATE_CONF("dai_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT17, DO_AUTOGATING),
	COMP_GATE_CONF("eac_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT18, DO_AUTOGATING),
	COMP_GATE_CONF("nand_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT19, DO_AUTOGATING),
	COMP_GATE_CONF("sdio_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT20, DO_AUTOGATING),
	COMP_GATE_CONF("sdio2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT21, DO_AUTOGATING),
	COMP_GATE_CONF("spi_pclk", "PAUTOGATING", 0, 
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT24, DO_AUTOGATING),
	COMP_GATE_CONF("spi2_pclk", "PAUTOGATING", 0, 
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT25, DO_AUTOGATING),
	COMP_GATE_CONF("spi3_pclk", "PAUTOGATING", 0, 
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT26, DO_AUTOGATING),
	COMP_GATE_CONF("sif_pclk", "PAUTOGATING", 0, 
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT27, DO_AUTOGATING),
	COMP_GATE_CONF("uart_pclk", "PAUTOGATING", 0, 
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT28, DO_AUTOGATING),
	COMP_GATE_CONF("remote_pclk", "PAUTOGATING", 0, 
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT30, DO_AUTOGATING),
	COMP_GATE_CONF("mipidsi_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT6, DO_AUTOGATING),
	COMP_GATE_CONF("ise_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT8, DO_AUTOGATING),
	COMP_GATE_CONF("affine_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT10, DO_AUTOGATING),
	COMP_GATE_CONF("pwm_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT11, DO_AUTOGATING),
	COMP_GATE_CONF("dre_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT14, DO_AUTOGATING),
	COMP_GATE_CONF("sdio3_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT15, DO_AUTOGATING),
	COMP_GATE_CONF("spi4_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT18, DO_AUTOGATING),
	COMP_GATE_CONF("tge_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT20, DO_AUTOGATING),
	COMP_GATE_CONF("rhe_pclk", "PAUTOGATING", 0, 
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT23, DO_AUTOGATING),
	COMP_GATE_CONF("hwcopy_pclk", "PAUTOGATING", 0, 
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT29, DO_AUTOGATING),
	COMP_GATE_CONF("rotate_pclk", "PAUTOGATING", 0, 
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT30, DO_AUTOGATING),
	COMP_GATE_CONF("gpio_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT0, DO_AUTOGATING),
	COMP_GATE_CONF("intc_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT1, DO_AUTOGATING),
	COMP_GATE_CONF("dma_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT3, DO_AUTOGATING),
	COMP_GATE_CONF("dma2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT4, DO_AUTOGATING),
	COMP_GATE_CONF("ive_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT5, DO_AUTOGATING),
	COMP_GATE_CONF("sde_cnn_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT6, DO_AUTOGATING),
	COMP_GATE_CONF("svm_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT7, DO_AUTOGATING),
	COMP_GATE_CONF("csi3_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT9, DO_AUTOGATING),
	COMP_GATE_CONF("csi4_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT10, DO_AUTOGATING),
	COMP_GATE_CONF("csi5_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT11, DO_AUTOGATING),
	COMP_GATE_CONF("csi6_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT12, DO_AUTOGATING),
	COMP_GATE_CONF("csi7_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT13, DO_AUTOGATING),
	COMP_GATE_CONF("csi8_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT14, DO_AUTOGATING),
	COMP_GATE_CONF("eth_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT20, DO_AUTOGATING),
	COMP_GATE_CONF("tsdemux_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT21, DO_AUTOGATING),
	COMP_GATE_CONF("ise2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT22, DO_AUTOGATING),
	COMP_GATE_CONF("pwm_gating", "fix120m", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_CLK_AUTO_GATING_REG1_OFFSET, BIT11, DO_AUTOGATING),
};

static struct nvt_composite_group_pwm_clk na51000_cgpwm_clk[] __initdata = {
	COMP_GPWM_CONF("fix120m", 250000,
		       CG_PWM_CLK_RATE_REG0_OFFSET, BIT0, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT0, BIT3, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8),
	COMP_GPWM_CONF("fix120m", 250000,
		       CG_PWM_CLK_RATE_REG0_OFFSET, BIT16, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT4, BIT7, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8),
	COMP_GPWM_CONF("fix120m", 250000,
		       CG_PWM_CLK_RATE_REG1_OFFSET, BIT0, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT8, BIT11, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT8),
};

static const char *sdio_clk_parent[] __initconst = {
	"fix192m", "fix480m", "pll2", "pll4"
};

static const char *tsmux_clk_parent[] __initconst = {
	"reserved", "fix240m", "fix480m", "reserved"
};

static const char *crypto_clk_parent[] __initconst = {
	"fix240m", "fix480m"
};

static const char *sie_clk_parent[] __initconst = {
	"fix480m", "pll2", "pll5", "pll14"
};

static const char *sie2_clk_parent[] __initconst = {
	"fix480m", "pll2", "pll5", "pll14"
};

static const char *sie3_clk_parent[] __initconst = {
	"fix480m", "pll2", "pll5", "pll14"
};

static const char *sie4_clk_parent[] __initconst = {
	"fix480m", "pll2", "pll5", "pll14"
};

static const char *sie5_clk_parent[] __initconst = {
	"fix480m"
};

static const char *sie6_clk_parent[] __initconst = {
	"fix480m"
};

static const char *sie7_clk_parent[] __initconst = {
	"fix480m"
};

static const char *sie8_clk_parent[] __initconst = {
	"fix480m"
};

static const char *rhe_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *ime_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *ife2_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *ise_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *ise2_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *siemclk_clk_parent[] __initconst = {
	"fix480m", "pll4", "pll5", "pll10"
};

static const char *ife_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *cnn_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *svm_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *dis_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};
static const char *ipe_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *dce_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *ive_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *sde_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *ide_clk_parent[] __initconst = {
	"fix480m", "pll2", "pll4"
};

static const char *mi_clk_parent[] __initconst = {
	"fix192m", "fix240m"
};

static const char *csi_clk_parent[] __initconst = {
	"fix60m", "fix120m"
};

static const char *grph_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *affine_clk_parent[] __initconst = {
	"fix160m", "fix240m", "pll6", "pll13"
};

static const char *jpg_clk_parent[] __initconst = {
	"fix240m", "fix480m", "pll6", "pll13"
};

static const char *h26x_clk_parent[] __initconst = {
    "fix192m", "fix240m", "pll6", "pll13"
};

static const char *dai_clk_parent[] __initconst = {
	"fix480m", "pll7"
};

static const char *sie2_io_pxclk_clk_parent[] __initconst = {
	"sie2_pxclkpad", "sie1_pxclkpad"
};

static const char *sie4_io_pxclk_clk_parent[] __initconst = {
	"sie4_pxclkpad", "sie3_pxclkpad"
};

static const char *sie5_io_pxclk_clk_parent[] __initconst = {
	"sie5_pxclkpad", "sie6_pxclkpad"
};

static const char *sie8_io_pxclk_clk_parent[] __initconst = {
	"sie8_pxclkpad", "sie7_pxclkpad"
};

static const char *sie1_pxclk_clk_parent[] __initconst = {
	"sie1_pxclkpad", "f0c00000.siemck"
};

static const char *sie2_pxclk_clk_parent[] __initconst = {
	"sie2_io_pxclk", "f0c00000.siemk2"
};

static const char *sie3_pxclk_clk_parent[] __initconst = {
	"sie3_pxclkpad", "f0c00000.siemk2", "vx1_1x", "vx1_2x"
};

static const char *sie4_pxclk_clk_parent[] __initconst = {
	"sie4_io_pxclk", "f0c00000.siemk2", "vx12_1x", "vx12_2x"
};

static const char *sie5_pxclk_clk_parent[] __initconst = {
	"sie5_io_pxclk", "f0c00000.siemk2", "vx1_1x", "vx1_2x"
};

static const char *sie6_pxclk_clk_parent[] __initconst = {
	"sie6_pxclkpad", "f0c00000.siemk2", "vx12_1x", "vx12_2x"
};

static const char *sie7_pxclk_clk_parent[] __initconst = {
	"sie7_pxclkpad", "f0c00000.siemk2"
};

static const char *sie8_pxclk_clk_parent[] __initconst = {
	"sie8_io_pxclk", "f0c00000.siemk2"
};

static const char *tge_clk_parent[] __initconst = {
	"sie1_pxclkpad", "f0c00000.siemck", "f0c00000.siemk2"
};

static const char *special_clk_parent[] __initconst = {
	"fix480m", "pll4", "pll10", "pll14"
};

static struct nvt_composite_mux_clk na51000_cmux_clk[] __initdata = {
	COMP_MUX_CONF("f0420000.mmc", sdio_clk_parent, 312500, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT4, WID2,
		      CG_SDIO_CLK_DIV_REG_OFFSET, BIT0, WID11,
		      CG_CLK_EN_REG1_OFFSET, BIT2, NOT_ENABLE,
		      CG_SYS_RESET_REG1_OFFSET, BIT2, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT20, DO_AUTOGATING),
	COMP_MUX_CONF("f0500000.mmc", sdio_clk_parent, 312500, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT8, WID2,
		      CG_SDIO_CLK_DIV_REG_OFFSET, BIT12, WID11,
		      CG_CLK_EN_REG1_OFFSET, BIT3, NOT_ENABLE,
		      CG_SYS_RESET_REG1_OFFSET, BIT3, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT21, NOT_AUTOGATING),
	COMP_MUX_CONF("f0510000.mmc", sdio_clk_parent, 312500, 0,
		      CG_PERI_CLK_RATE_REG1_OFFSET, BIT0, WID2,
		      CG_PERI_CLK_DIV_REG1_OFFSET, BIT0, WID11,
		      CG_CLK_EN_REG1_OFFSET, BIT14, NOT_ENABLE,
		      CG_SYS_RESET_REG1_OFFSET, BIT14, DO_RESET,
		      CG_CLK_AUTO_GATING_REG1_OFFSET, BIT15, NOT_AUTOGATING),
	COMP_MUX_CONF("f0620000.crypto", crypto_clk_parent, 480000000, 1,
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT31, WID1, 0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT23, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT23, NOT_RESET,
		      0, 0, 0),
	COMP_MUX_CONF("f0650000.tsmux", tsmux_clk_parent, 240000000, 1,
		      CG_PERI_CLK_RATE_REG1_OFFSET, BIT4, WID2, 0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT13, DO_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT13, NOT_RESET,
		      CG_CLK_AUTO_GATING_REG2_OFFSET, BIT21, DO_AUTOGATING),
	COMP_MUX_CONF("f0c00000.sie", sie_clk_parent, 240000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT0, WID3,
		      CG_IPP_CLK_DIV_REG0_OFFSET, BIT16, WID8,
		      CG_CLK_EN_REG0_OFFSET, BIT4, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT4, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT0, DO_AUTOGATING),
	COMP_MUX_CONF("f0d20000.sie", sie2_clk_parent, 240000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT3, WID3,
		      CG_IPP_CLK_DIV_REG0_OFFSET, BIT24, WID8,
		      CG_CLK_EN_REG0_OFFSET, BIT5, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT5, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT9, DO_AUTOGATING),
	COMP_MUX_CONF("f0d30000.sie", sie3_clk_parent, 240000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT6, WID3,
		      CG_IPP_CLK_DIV_REG1_OFFSET, BIT0, WID8,
		      CG_CLK_EN_REG2_OFFSET, BIT15, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT15, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT21, DO_AUTOGATING),
	COMP_MUX_CONF("f0d40000.sie", sie4_clk_parent, 240000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT9, WID3,
		      CG_IPP_CLK_DIV_REG1_OFFSET, BIT8, WID8,
		      CG_CLK_EN_REG2_OFFSET, BIT16, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT16, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT22, DO_AUTOGATING),
	COMP_MUX_CONF("f0d50000.sie", sie5_clk_parent, 240000000, 0,
		      0, 0, 0,
		      CG_IPP_CLK_DIV_REG1_OFFSET, BIT16, WID4,
		      CG_CLK_EN_REG3_OFFSET, BIT26, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT5, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT15, DO_AUTOGATING),
	COMP_MUX_CONF("f0d60000.sie", sie6_clk_parent, 240000000, 0,
		      0, 0, 0,
		      CG_IPP_CLK_DIV_REG1_OFFSET, BIT20, WID4,
		      CG_CLK_EN_REG3_OFFSET, BIT27, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT5, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT16, DO_AUTOGATING),
	COMP_MUX_CONF("f0d70000.sie", sie7_clk_parent, 240000000, 0,
		      0, 0, 0,
		      CG_IPP_CLK_DIV_REG1_OFFSET, BIT24, WID4,
		      CG_CLK_EN_REG3_OFFSET, BIT28, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT5, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT17, DO_AUTOGATING),
	COMP_MUX_CONF("f0d80000.sie", sie8_clk_parent, 240000000, 0,
		      0, 0, 0,
		      CG_IPP_CLK_DIV_REG1_OFFSET, BIT28, WID4,
		      CG_CLK_EN_REG3_OFFSET, BIT29, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT29, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT18, DO_AUTOGATING),
	COMP_MUX_CONF("f0c40000.ime", ime_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT12, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT9, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT9, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT3, DO_AUTOGATING),
	COMP_MUX_CONF("f0ce0000.rhe", rhe_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG1_OFFSET, BIT12, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT19, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT19, DO_RESET,
		      0, 0, 0),
	COMP_MUX_CONF("f0d00000.ife2", ife2_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG1_OFFSET, BIT4, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT15, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT15, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT14, DO_AUTOGATING),
	COMP_MUX_CONF("f0c90000.ise", ise_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT24, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT11, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT11, DO_RESET,
		      CG_CLK_AUTO_GATING_REG1_OFFSET, BIT8, DO_AUTOGATING),
	COMP_MUX_CONF("f0dc0000.ise", ise2_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG1_OFFSET, BIT16, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG4_OFFSET, BIT2, NOT_ENABLE,
		      CG_SYS_RESET_REG4_OFFSET, BIT2, DO_RESET,
		      CG_CLK_AUTO_GATING_REG2_OFFSET, BIT22, DO_AUTOGATING),
	COMP_MUX_CONF("f0c00000.siemck", siemclk_clk_parent, 240000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT12, WID2,
		      CG_IPP_CLK_DIV_REG0_OFFSET, BIT0, WID8,
		      CG_CLK_EN_REG0_OFFSET, BIT2, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("f0c00000.siemk2", siemclk_clk_parent, 240000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT14, WID2,
		      CG_IPP_CLK_DIV_REG0_OFFSET, BIT8, WID8,
		      CG_CLK_EN_REG0_OFFSET, BIT3, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("f0c70000.ife", ife_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG1_OFFSET, BIT0, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT13, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT13, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT7, DO_AUTOGATING),
	COMP_MUX_CONF("f0cb0000.ai", cnn_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT0, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG3_OFFSET, BIT31, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT31, DO_RESET,
		      CG_CLK_AUTO_GATING_REG2_OFFSET, BIT6, DO_AUTOGATING),
	COMP_MUX_CONF("f0da0000.ai", svm_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG1_OFFSET, BIT8, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG4_OFFSET, BIT0, NOT_ENABLE,
		      CG_SYS_RESET_REG4_OFFSET, BIT0, DO_RESET,
		      CG_CLK_AUTO_GATING_REG2_OFFSET, BIT7, DO_AUTOGATING),
	COMP_MUX_CONF("f0c50000.dis", dis_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT8, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT8, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT8, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT4, DO_AUTOGATING),
	COMP_MUX_CONF("f0c20000.dce", dce_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT28, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT14, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT14, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT6, DO_AUTOGATING),
	COMP_MUX_CONF("f0d90000.ive", ive_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT20, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG3_OFFSET, BIT30, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT30, DO_RESET,
		      CG_CLK_AUTO_GATING_REG2_OFFSET, BIT5, DO_AUTOGATING),
	COMP_MUX_CONF("f0c30000.ipe", ipe_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT4, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT7, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT7, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT2, DO_AUTOGATING),
	COMP_MUX_CONF("f0800000.ide", ide_clk_parent, 297000000, 1,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT16, WID2,
		      CG_IDE_CLK_RATE_REG_OFFSET, BIT0, WID8,
		      CG_CLK_EN_REG0_OFFSET, BIT16, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT16, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT10, DO_AUTOGATING),
	COMP_MUX_CONF("f0900000.ide2", ide_clk_parent, 297000000, 1,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT18, WID2,
		      CG_IDE_CLK_RATE_REG_OFFSET, BIT8, WID8,
		      CG_CLK_EN_REG0_OFFSET, BIT17, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT17, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT11, DO_AUTOGATING),
	COMP_MUX_CONF("f0800000.ideif", ide_clk_parent, 297000000, 1,
		      0, 0, 0,
		      CG_IDE_CLK_RATE_REG_OFFSET, BIT16, WID8,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("f0900000.ide2if", ide_clk_parent, 297000000, 1,
		      0, 0, 0,
		      CG_IDE_CLK_RATE_REG_OFFSET, BIT24, WID8,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("f0840000.dsi", ide_clk_parent, 60000000, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT20, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT3, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT3, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT6, DO_AUTOGATING),
	COMP_MUX_CONF("f0280000.csi", csi_clk_parent, 60000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT0, WID1,
		      0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT1, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT1, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT3, DO_AUTOGATING),
	COMP_MUX_CONF("f0330000.csi", csi_clk_parent, 60000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT1, WID1,
		      0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT2, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT2, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT4, DO_AUTOGATING),
	COMP_MUX_CONF("f0e60000.csi", csi_clk_parent, 60000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT2, WID1,
		      0, 0, 0,
		      CG_CLK_EN_REG3_OFFSET, BIT20, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT20, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT9, DO_AUTOGATING),
	COMP_MUX_CONF("f0e70000.csi", csi_clk_parent, 60000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT3, WID1,
		      0, 0, 0,
		      CG_CLK_EN_REG3_OFFSET, BIT21, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT21, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT10, DO_AUTOGATING),
	COMP_MUX_CONF("f0e80000.csi", csi_clk_parent, 60000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT4, WID1,
		      0, 0, 0,
		      CG_CLK_EN_REG3_OFFSET, BIT22, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT22, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT11, DO_AUTOGATING),
	COMP_MUX_CONF("f0e90000.csi", csi_clk_parent, 60000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT5, WID1,
		      0, 0, 0,
		      CG_CLK_EN_REG3_OFFSET, BIT23, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT23, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT12, DO_AUTOGATING),
	COMP_MUX_CONF("f0ea0000.csi", csi_clk_parent, 60000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT6, WID1,
		      0, 0, 0,
		      CG_CLK_EN_REG3_OFFSET, BIT24, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT24, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT13, DO_AUTOGATING),
	COMP_MUX_CONF("f0eb0000.csi", csi_clk_parent, 60000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT7, WID1,
		      0, 0, 0,
		      CG_CLK_EN_REG3_OFFSET, BIT25, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT25, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT14, DO_AUTOGATING),
	COMP_MUX_CONF("f0c80000.grph", grph_clk_parent, 240000000, 0,
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT8, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT27, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT27, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT8, DO_AUTOGATING),
	COMP_MUX_CONF("f0d10000.grph", grph_clk_parent, 240000000, 0,
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT12, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT28, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT28, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT9, DO_AUTOGATING),
	COMP_MUX_CONF("f0a00000.jpg", jpg_clk_parent, 240000000, 0,
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT0, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT26, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT26, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT15, DO_AUTOGATING),
	COMP_MUX_CONF("f0a10000.h26x", h26x_clk_parent, 192000000, 0,
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT4, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT24, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT24, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT16, DO_AUTOGATING),
	COMP_MUX_CONF("f0630000.hdmado", dai_clk_parent, 12288000, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT30, WID2,
		      CG_PERI_CLK_DIV_REG0_OFFSET, BIT24, WID8,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("f0630000.dai", dai_clk_parent, 12288000, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT28, WID2,
		      CG_PERI_CLK_DIV_REG0_OFFSET, BIT16, WID8,
		      CG_CLK_EN_REG0_OFFSET, BIT29, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT29, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT17, DO_AUTOGATING),
	COMP_MUX_CONF("f0640000.eac", dai_clk_parent, 12288000, 0,
		      0, 0, 0,
		      0, 0, 0,
		      CG_CLK_EN_REG1_OFFSET, BIT20, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT30, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT18, DO_AUTOGATING),
	COMP_MUX_CONF("f0640000.eacadc", dai_clk_parent, 12288000, 0,
		      0, 0, 0,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT30, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT30, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT18, DO_AUTOGATING),
	COMP_MUX_CONF("f0640000.eacdac", dai_clk_parent, 12288000, 0,
		      0, 0, 0,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT31, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT30, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT18, DO_AUTOGATING),
	COMP_MUX_CONF("sie2_io_pxclk", sie2_io_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT25, WID2,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie4_io_pxclk", sie4_io_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT27, WID1,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie5_io_pxclk", sie5_io_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT28, WID1,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie8_io_pxclk", sie8_io_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT31, WID1,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie1_pxclk", sie1_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT16, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT4, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie2_pxclk", sie2_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT18, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT5, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie3_pxclk", sie3_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT20, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG4_OFFSET, BIT14, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie4_pxclk", sie4_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT22, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG4_OFFSET, BIT15, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie5_pxclk", sie5_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT24, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG4_OFFSET, BIT16, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie6_pxclk", sie6_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT26, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG4_OFFSET, BIT17, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie7_pxclk", sie7_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT28, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG4_OFFSET, BIT18, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("sie8_pxclk", sie8_pxclk_clk_parent, 108000000, 0,
		      CG_PERI_CLK_RATE_REG3_OFFSET, BIT30, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG4_OFFSET, BIT19, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("f0cf0000.rotate", grph_clk_parent, 240000000, 0,
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT24, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT31, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT31, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT30, DO_AUTOGATING),
	COMP_MUX_CONF("f0810000.mi", mi_clk_parent, 1920000000, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT22, WID2,
		      CG_VIDEO_CLK_DIV_REG_OFFSET, BIT16, WID8,
		      CG_CLK_EN_REG0_OFFSET, BIT20, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT20, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT12, DO_AUTOGATING),
	COMP_MUX_CONF("f0c10000.hwcopy", grph_clk_parent, 240000000, 0,
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT20, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT30, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT30, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT29, DO_AUTOGATING),
	COMP_MUX_CONF("f0ca0000.affine", affine_clk_parent, 160000000, 0,
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT16, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT25, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT25, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT10, DO_AUTOGATING),
	COMP_MUX_CONF("f0cc0000.tge", tge_clk_parent, 240000000, 1,
		      CG_IPP_CLK_RATE_REG1_OFFSET, BIT22, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG0_OFFSET, BIT6, NOT_ENABLE,
		      CG_SYS_RESET_REG0_OFFSET, BIT6, DO_RESET,
		      0, 0, 0),
	COMP_MUX_CONF("sp_clk", special_clk_parent, 25000000, 1,
		      CG_PERI_CLK_RATE_REG1_OFFSET, BIT8, WID2,
		      CG_PERI_CLK_DIV_REG0_OFFSET, BIT0, WID8,
		      CG_CLK_EN_REG0_OFFSET, BIT12, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0),
	COMP_MUX_CONF("f0db0000.sde", sde_clk_parent, 240000000, 0,
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT0, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG3_OFFSET, BIT31, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT31, DO_RESET,
		      CG_CLK_AUTO_GATING_REG2_OFFSET, BIT6, DO_AUTOGATING),
};


static struct nvt_clk_gating_init na51000_init_gating_clk[] __initdata = {
	COMP_GATING_CONF(CG_CLK_AUTO_GATING_REG0_OFFSET, 0xF7FEFFFF),
	COMP_GATING_CONF(CG_CLK_AUTO_GATING_REG1_OFFSET, 0xFFFFFFFF),
	COMP_GATING_CONF(CG_CLK_AUTO_GATING_REG2_OFFSET, 0xFF7FFFFF),
	COMP_GATING_CONF(CG_APB_CLK_AUTO_GATING_REG0_OFFSET, 0xFFFFFFFD),
	COMP_GATING_CONF(CG_APB_CLK_AUTO_GATING_REG1_OFFSET, 0x67F7CF7B),
	COMP_GATING_CONF(CG_APB_CLK_AUTO_GATING_REG2_OFFSET, 0x77FEFB),
};

static int na51000_misc_clk_register(void)
{
	struct clk *clk;
	int ret = 0;

	clk = clk_register_fixed_factor(NULL, "pll12div6", "pll12", 0, 1, 6);
	if (IS_ERR(clk)) {
		pr_err("%s: failed to register clock hardware \"pll12div6\"\n",
		       __func__);
		ret = -EPERM;
	} else {
		ret = clk_prepare_enable(clk);
		if (ret < 0)
			pr_err("pll12div6 prepare & enable failed!\n");
	}

	return ret;
}

int na51000_clock_init(void)
{
	int ret;

	pr_info("na51000_clock_init\n");

	spin_lock_init(&cg_lock);

	remap_base = ioremap_nocache(IOADDR_CG_REG_BASE, IOADDR_CG_REG_SIZE);
	if (IS_ERR((void *)remap_base)) {
		ret = -EPERM;
		pr_err("Failed to remap CKG registers!\n");
		goto exit;
	} else
		nvt_cg_base_remap(remap_base);

	nvt_init_clk_auto_gating(na51000_init_gating_clk,
				ARRAY_SIZE(na51000_init_gating_clk), &cg_lock);

	ret =
	    nvt_fixed_rate_clk_register(na51000_fixed_rate_clk,
					ARRAY_SIZE(na51000_fixed_rate_clk));
	if (ret < 0)
		goto exit;

	ret =
	    nvt_pll_clk_register(na51000_pll, ARRAY_SIZE(na51000_pll),
				 DEFAULT_PLL_DIV_VALUE, &cg_lock);
	if (ret < 0)
		goto exit;

	ret = na51000_misc_clk_register();
	if (ret < 0)
		goto exit;

	ret =
	    nvt_composite_gate_clk_register(na51000_cgate_clk,
					    ARRAY_SIZE
					    (na51000_cgate_clk), &cg_lock);
	if (ret < 0)
		goto exit;

	ret =
	    nvt_composite_group_pwm_clk_register(na51000_cgpwm_clk,
						 ARRAY_SIZE
						 (na51000_cgpwm_clk), &cg_lock);
	if (ret < 0)
		goto exit;

	ret =
	    nvt_composite_mux_clk_register(na51000_cmux_clk,
					   ARRAY_SIZE
					   (na51000_cmux_clk), &cg_lock);
	if (ret < 0)
		goto exit;

exit:
	return ret;
}

arch_initcall(na51000_clock_init);
