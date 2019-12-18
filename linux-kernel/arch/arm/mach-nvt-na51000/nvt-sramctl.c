/**
    NVT SRAM Contrl
    This file will Enable and disable SRAM shutdown
    @file       nvt-sramctl.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <mach/nvt-sramctl.h>
#include <linux/mutex.h>
#include <mach/ioaddress.h>

struct mutex top_lock;

#if 0
static void __iomem *remap_base = (void __iomem *)IOADDR_TOP_REG_BASE;
#define NA51000_TOP_SETREG(ofs, value)	writel((value), (remap_base+(ofs)))
#define NA51000_TOP_GETREG(ofs)		readl(remap_base+(ofs))
#endif

#define loc_cpu() {mutex_lock(&top_lock);}
#define unl_cpu() {mutex_unlock(&top_lock);}

#define SRAM_OFS 0x1000

void nvt_disable_sram_shutdown(SRAM_SD id)
{
#if 0
	u32 reg_data, reg_ofs;

	reg_ofs = (id >> 5) << 2;

	loc_cpu();

	reg_data = NA51000_TOP_GETREG(SRAM_OFS + reg_ofs);

	reg_data &= ~(1 << (id & 0x1F));

	NA51000_TOP_SETREG(SRAM_OFS + reg_ofs, reg_data);

	unl_cpu();
#endif
}

void nvt_enable_sram_shutdown(SRAM_SD id)
{
#if 0
	u32 reg_data, reg_ofs;

	reg_ofs = (id >> 5) << 2;

	loc_cpu();

	reg_data = NA51000_TOP_GETREG(SRAM_OFS + reg_ofs);

	reg_data |= (1 << (id & 0x1F));

	NA51000_TOP_SETREG(SRAM_OFS + reg_ofs, reg_data);

	unl_cpu();
#endif
}

static int __init nvt_init_sram_mutex(void)
{
	mutex_init(&top_lock);
	return 0;
}

core_initcall(nvt_init_sram_mutex);
EXPORT_SYMBOL(nvt_enable_sram_shutdown);
EXPORT_SYMBOL(nvt_disable_sram_shutdown);