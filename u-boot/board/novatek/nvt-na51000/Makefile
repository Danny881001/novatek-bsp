#
# (C) Copyright 2000, 2001, 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
BUILD_DATE = 20`date +'%y%m%d'`
ccflags-y += -D_BUILD_DATE_=$(BUILD_DATE)

obj-y := na51000evb.o
obj-y += na51000_utils.o nvt_common.o nvtpack.o nvt_headinfo.o
obj-$(CONFIG_NVT_LINUX_AUTOLOAD) += nvt_fw_update.o
