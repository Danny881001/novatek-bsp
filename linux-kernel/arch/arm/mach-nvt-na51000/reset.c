/**
    NVT reset function
    This file will setup reset function interface
    @file       nvt-reset.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/pm.h>
#include <linux/reboot.h>
#include <linux/delay.h>

#include <asm/io.h>

#include <mach/nvt-io.h>
#include <mach/nvt-ipc.h>
#include <mach/hardware.h>
#include <mach/rtc_int.h>

static int nvt_notify_reboot(struct notifier_block *nb, unsigned long action, void *data)
{
	if (action == SYS_RESTART) {
		pr_info("restarting.....\n");
#ifdef CONFIG_RTC_DRV_NA51000
		nvt_rtc_power_control(3);
#endif
	}

	return NOTIFY_DONE;
}

static void nvt_power_off_prepare(void)
{
#ifdef CONFIG_RTC_DRV_NA51000
	nvt_rtc_power_control(0);
#endif
}

static struct notifier_block nvt_reboot_notifier = {
	.notifier_call  = nvt_notify_reboot,
	.next       = NULL,
	.priority   = INT_MAX,
};

static void nvt_power_off_done(void)
{
	pr_info(" Power off \n");
}

static int __init nvt_poweroff_setup(void)
{
	pr_info("*** %s \n", __func__);

	pm_power_off = nvt_power_off_done;
        pm_power_off_prepare = nvt_power_off_prepare;
	register_reboot_notifier(&nvt_reboot_notifier);

	return 0;
}

/* It will be executed when restart done */
void nvt_na51000_restart(enum reboot_mode mode, const char *cmd)
{
	pr_info("done\n");
	/* Waiting for core0 rtc flow ready */
	mdelay(5000);
}

arch_initcall(nvt_poweroff_setup);
