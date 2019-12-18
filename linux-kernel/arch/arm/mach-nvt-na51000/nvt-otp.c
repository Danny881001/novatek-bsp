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

#include <linux/crypto.h>
#include <mach/efuse_protected.h>
#include <mach/dma_protected.h>
#include <linux/mutex.h>
#include <mach/rcw_macro.h>
#include <linux/vmalloc.h>
#include <asm/pgtable.h>
#include <asm/barrier.h>



#ifndef CHKPNT
#define CHKPNT    printk("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#endif

#ifndef DBGD
#define DBGD(x)   printk("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif

#ifndef DBGH
#define DBGH(x)   printk("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) printk(fmtstr, ##args)
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  printk("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  printk("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#if 0
#define DBG_IND(fmtstr, args...) printk("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif


struct mutex otp_lock;


#define otp_loc() {mutex_lock(&otp_lock); }
#define otp_unl() {mutex_unlock(&otp_lock); }

static UINT32 	OTP_PG_MALLOC = 0;
extern INT32    efuse_read_addr(UINT32 efuse_addr, UINT32 address);


static UINT32 OTP_READ[] = {
	0xE92D41F0,
	0xE1A04000,
	0xE1A06001,
	0xE30F2D66,
	0xE3E00020,
	0xE2867094,
	0xE3A0800D,
	0xE3A0501F,
	0xE0045005,
	0xE1550008,
	0xC1A0F007,
	0xE1A00802,
	0xE286703C,
	0xE3A01001,
	0xE5801020,
	0xE5901030,
	0xE7E01051,
	0xE3510001,
	0x11A0F007,
	0xE3A01000,
	0xE0811004,
	0xE3A05000,
	0xE5805000,
	0xE2867070,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE3A02000,
	0xE5802020,
	0xE5900008,
	0xE8BD81F0,
	0xE320F000,
	0xEAFFFFFE,
	0xE320F000,
};

static UINT32 OTP_OP[] = {
	0xE92D41F0,
	0xE1A04000,
	0xE1A06001,
	0xE30F2D66,
	0xE1A00802,
	0xE30F2D62,
	0xE1A03802,
	0xE2867028,
	0xE3A01001,
	0xE5801020,
	0xE5901030,
	0xE7E01051,
	0xE3510001,
	0x11A0F007,
	0xE3A01010,
	0xE0811104,
	0xE3A05001,
	0xE5835020,
	0xE286705C,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE2811001,
	0xE3A05002,
	0xE5835020,
	0xE2867094,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE2811001,
	0xE3A05004,
	0xE5835020,
	0xE28670CC,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE2811001,
	0xE3A05008,
	0xE5835020,
	0xE2867F41,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE3A02000,
	0xE5802020,
	0xE8BD81F0,
	0xE320F000,
	0xEAFFFFFE,
	0xE320F000,
};

static UINT32 OTP_PG[] = {
	0xE92D41F0,
	0xE1A04000,
	0xE1A06001,
	0xE30F2D66,
	0xE3E00020,
	0xE286709C,
	0xE3A08010,
	0xE3A0501F,
	0xE0045005,
	0xE1580005,
	0xC1A0F007,
	0xE1A00802,
	0xE286703C,
	0xE3A01001,
	0xE5801020,
	0xE5901030,
	0xE7E01051,
	0xE3510001,
	0x11A0F007,
	0xE3A01000,
	0xE0811004,
	0xE3A05004,
	0xE5805000,
	0xE2867070,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE3A02000,
	0xE5802020,
	0xE5802004,
	0xE5802000,
	0xE3A00000,
	0xE8BD81F0,
	0xE320F000,
	0xEAFFFFFE,
	0xE320F000,
};


static UINT32 DMA_USAGE_DATA[] = {
	0xE92D41F0,
	0xE1A04000,
	0xE1A06001,
	0xE2868020,
	0xE30F2D00,
	0xE1A02802,
	0xE1A03A04,
	0xE3A07A09,
	0xE1822003,
	0xE1827007,
	0xE5921054,
	0xE5970000,
	0xE1A05001,
	0xE1A06000,
	0xE3500000,
	0x01A0F008,
	0xE3A03064,
	0xE7F91051,
	0xE7F90050,
	0xE0030193,
	0xE730F013,
	0xE8BD81F0,
	0xE320F000,
	0xEAFFFFFE,
	0xE320F000,
};

static UINT32 OTP_KEY0_SET[] = {
	0xE92D41F0,
	0xE1A04000,
	0xE1A06000,
	0xE30F2D66,
	0xE1A00802,
	0xE2867020,
	0xE3A01001,
	0xE5801020,
	0xE5901030,
	0xE7E01051,
	0xE3510001,
	0x11A0F007,
	0xE3A01000,
	0xE3A01FE2,
	0xE3A05004,
	0xE5805000,
	0xE2867054,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE3A02000,
	0xE5802020,
	0xE5802004,
	0xE5802000,
	0xE8BD81F0,
	0xE320F000,
	0xEAFFFFFE,
	0xE320F000,
};

static UINT32 OTP_KEY1_SET[] = {
	0xE92D41F0,
	0xE1A04000,
	0xE1A06000,
	0xE30F2D66,
	0xE1A00802,
	0xE2867020,
	0xE3A01001,
	0xE5801020,
	0xE5901030,
	0xE7E01051,
	0xE3510001,
	0x11A0F007,
	0xE3A01000,
	0xE3A01FEA,
	0xE3A05004,
	0xE5805000,
	0xE2867054,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE3A02000,
	0xE5802020,
	0xE5802004,
	0xE5802000,
	0xE8BD81F0,
	0xE320F000,
	0xEAFFFFFE,
	0xE320F000,
};

static UINT32 OTP_KEY2_SET[] = {
	0xE92D41F0,
	0xE1A04000,
	0xE1A06000,
	0xE30F2D66,
	0xE1A00802,
	0xE2867020,
	0xE3A01001,
	0xE5801020,
	0xE5901030,
	0xE7E01051,
	0xE3510001,
	0x11A0F007,
	0xE3A01000,
	0xE3A01FF2,
	0xE3A05004,
	0xE5805000,
	0xE2867054,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE3A02000,
	0xE5802020,
	0xE5802004,
	0xE5802000,
	0xE8BD81F0,
	0xE320F000,
	0xEAFFFFFE,
	0xE320F000,
};

static UINT32 OTP_KEY3_SET[] = {
	0xE92D41F0,
	0xE1A04000,
	0xE1A06000,
	0xE30F2D66,
	0xE1A00802,
	0xE2867020,
	0xE3A01001,
	0xE5801020,
	0xE5901030,
	0xE7E01051,
	0xE3510001,
	0x11A0F007,
	0xE3A01000,
	0xE3A01FFA,
	0xE3A05004,
	0xE5805000,
	0xE2867054,
	0xE5801004,
	0xE5902020,
	0xE3822002,
	0xE5802020,
	0xE5902020,
	0xE7E020D2,
	0xE3520001,
	0x01A0F007,
	0xE5902030,
	0xE5802030,
	0xE3A02000,
	0xE5802020,
	0xE5802004,
	0xE5802000,
	0xE8BD81F0,
	0xE320F000,
	0xEAFFFFFE,
	0xE320F000,
};

static void _efuse_key_set_en_(UINT32 address)
{
	otp_loc();
	__asm__("MOV r0, %0\n\t"
			"MOV lr, pc\n\t"
			"MOV pc, %0\n\t"
			:
			: "r"(address)
			: "lr", "r0", "r1", "r2", "r3");
	otp_unl();
	return;
}

static INT32 efuse_check_read_data(UINT16 data)
{
	INT32 i_ret;

	if ((data & EFUSE_STATUS_VALID) != EFUSE_STATUS_VALID) {
		i_ret = EFUSE_INACTIVE_ERR;
	} else {
		if ((data & EFUSE_STATUS_FORCE_INVALID) == EFUSE_STATUS_FORCE_INVALID) {
			i_ret = EFUSE_INVALIDATE_ERR;
		} else {
			i_ret = EFUSE_SUCCESS;
		}
	}
	return i_ret;
}

static INT32 efuse_read_data(UINT32 efuse_addr, UINT32 address)
{
	INT32  result;

	char *p = __vmalloc(sizeof(OTP_READ), GFP_KERNEL, PAGE_KERNEL_EXEC);

	if(!p) {
		DBG_ERR("efuse read data allocate buffer error\r\n");
		return -1;
	}

	if(!pte_exec((pte_t)p)) {
		DBG_ERR("efuse read data allocate buffer not executable 0x%x\r\n", p);
	 	vfree(p);
		return -1;
	}
	/*else {
		DBG_DUMP("fource make exec 0x%x\r\n", p);
		p = pte_mkexec((pte_t)p);
	}*/

	memcpy((void *)p, (void *)address, sizeof(OTP_READ));

	otp_loc();

	__asm__("MOV r0, %1\n\t"
			"MOV r1, %2\n\t"
			"MOV lr, pc\n\t"
			"MOV pc, %2\n\t"
			"MOV %0, r0\n\t"
			: "=r"(result)
			: "r"(efuse_addr), "r"(p)
			: "cc", "lr", "r0", "r1", "r2", "r3");
	otp_unl();

	vfree((void *)p);
	isb();
	dsb();
	return result;
}

static INT32 efuse_set_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT32 address)
{

	otp_loc();
	__asm__("MOV r0, %0\n\t"
			"MOV r1, %1\n\t"
			"MOV lr, pc\n\t"
			"MOV pc, %1\n\t"
			:
			: "r"(key_set_index), "r"(address)
			: "lr", "r0", "r1", "r2", "r3");
	otp_unl();
	return E_OK;
}

static INT32 _efuse_prog_(UINT32 addr, UINT32 address)
{
	INT32  result;

	otp_loc();


	__asm__("MOV r0, %1\n\t"
			"MOV r1, %2\n\t"
			"MOV lr, pc\n\t"
			"MOV pc, %2\n\t"
			"MOV %0, r0\n\t"
			: "=r"(result)
			: "r"(addr), "r"(address)
			: "cc", "lr", "r0", "r1", "r2", "r3");
	otp_unl();

	return result;
}

static INT32 efuse_program(UINT32 rol, UINT32 col)
{
	return _efuse_prog_((rol | (col << 5)), (UINT32)OTP_PG_MALLOC);
}

static INT32 efuse_write_data(UINT32 addr, UINT32 data)
{
	UINT32  ui_bits;
	UINT32  ui_data;

	if (addr < 16) {
		return EFUSE_PARAM_ERR;
	}

	ui_data = data;

	ui_bits = 0;

	while (ui_data) {
		ui_bits = __builtin_ctz(ui_data);
		ui_data &= ~(1 << ui_bits);
		if (efuse_program(addr, ui_bits) != E_OK) {
			DBG_ERR("%s,eFuse program addr[%03lx] bit[%2d] fail\r\n", __func__, (UINT32)addr, (int)ui_bits);
			return EFUSE_OPS_ERR;
		}
	}

	return EFUSE_SUCCESS;
}

static UINT32 dma_get_utilization_data(UINT32 id, UINT32 address)
{
	UINT32  result;

	char *p = __vmalloc(sizeof(DMA_USAGE_DATA), GFP_KERNEL, PAGE_KERNEL_EXEC);

	if(!p) {
		DBG_ERR("efuse read data allocate buffer error\r\n");
		return -1;
	}

	if(!pte_exec((pte_t)p)) {
		DBG_ERR("efuse read data allocate buffer not executable 0x%x\r\n", p);
	 	vfree(p);
		return -1;
	}
	/*else {
		DBG_DUMP("fource make exec 0x%x\r\n", p);
		p = pte_mkexec((pte_t)p);
	}*/

	memcpy((void *)p, (void *)address, sizeof(DMA_USAGE_DATA));

	__asm__("MOV r0, %1\n\t"
			"MOV r1, %2\n\t"
			"MOV lr, pc\n\t"
			"MOV pc, %2\n\t"
			"MOV %0, r0\n\t"
			: "=r"(result)
			:"r"(id), "r"(p)
			: "lr", "r0", "r1", "r2", "r3");

	vfree(p);
	isb();
	dsb();
	return result;
}



#define EFUSE_READDATA(addr)    efuse_read_data(addr, (UINT32)OTP_READ)


/**
     key set enable identifier

     Program specific key set flag(represent key set were programmed or not)

     @param[in] key set     0~3
     @return Description of data returned.
         - @b  0:   Success
         - @b -1:   Fail
*/

INT32 efuse_key_set_flag(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	UINT32 key_set_addr;
	char *p;
	switch (key_set_index) {
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		p = __vmalloc(sizeof(OTP_KEY0_SET), GFP_KERNEL, PAGE_KERNEL_EXEC);
		if(!p) {
			DBG_ERR("efuse key0 data allocate buffer error\r\n");
			return -1;
		}

		if(!pte_exec((pte_t)p)) {
			DBG_ERR("efuse key0 data allocate buffer not executable 0x%x\r\n", p);
		 	vfree(p);
			return -1;
		}
		memcpy((void *)p, (void *)OTP_KEY0_SET, sizeof(OTP_KEY0_SET));

		key_set_addr = (UINT32)p;
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		p = __vmalloc(sizeof(OTP_KEY1_SET), GFP_KERNEL, PAGE_KERNEL_EXEC);
		if(!p) {
			DBG_ERR("efuse key1 data allocate buffer error\r\n");
			return -1;
		}

		if(!pte_exec((pte_t)p)) {
			DBG_ERR("efuse key1 data allocate buffer not executable 0x%x\r\n", p);
		 	vfree(p);
			return -1;
		}
		memcpy((void *)p, (void *)OTP_KEY1_SET, sizeof(OTP_KEY1_SET));

		key_set_addr = (UINT32)p;
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		p = __vmalloc(sizeof(OTP_KEY2_SET), GFP_KERNEL, PAGE_KERNEL_EXEC);
		if(!p) {
			DBG_ERR("efuse key2 data allocate buffer error\r\n");
			return -1;
		}

		if(!pte_exec((pte_t)p)) {
			DBG_ERR("efuse key2 data allocate buffer not executable 0x%x\r\n", p);
		 	vfree(p);
			return -1;
		}
		memcpy((void *)p, (void *)OTP_KEY2_SET, sizeof(OTP_KEY2_SET));

		key_set_addr = (UINT32)p;
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		p = __vmalloc(sizeof(OTP_KEY3_SET), GFP_KERNEL, PAGE_KERNEL_EXEC);
		if(!p) {
			DBG_ERR("efuse key3 data allocate buffer error\r\n");
			return -1;
		}

		if(!pte_exec((pte_t)p)) {
			DBG_ERR("efuse key3 data allocate buffer not executable 0x%x\r\n", p);
		 	vfree(p);
			return -1;
		}
		memcpy((void *)p, (void *)OTP_KEY3_SET, sizeof(OTP_KEY3_SET));

		key_set_addr = (UINT32)p;
		break;

	default:
		DBG_ERR("Unknow key set [%d] (0~3)\r\n", key_set_index);
		return -1;
		break;
	}
	_efuse_key_set_en_(key_set_addr);

	vfree((void *)p);
	isb();
	dsb();
	return 0;
}

/**
     Get key set enable identifier

     Represent specific key set was programmed or not

     @param[in] key set     0~3
     @return Description of data returned.
         - @b  0:   Not programmed
         - @b  1:   Programmed
*/
UINT32 efuse_key_get_flag(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	volatile UINT32  param;

	param = EFUSE_READDATA(EFUSE_TRIM_PARAM_FIELD);

	return (param & (1<<(EFUSE_OTP_1ST_KEY_SET_FIELD_FLAG+key_set_index)))==0?0x00000000:0x00000001;
}

/**
    efuse_write_key

    Write specific key into specific key set (0~3)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~3)
    @param[in] ucKey           key (16bytes)
    @return Description of data returned.
        - @b E_OK:   Success
        - @b E_SYS:  Fail
*/
INT32 efuse_write_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key)
{
	INT32   result = EFUSE_SUCCESS;
	UINT32  u32_key = (UINT32)(uc_key);
	UINT32  data[4];
	UINT32  key_field_start_index = 16;
	UINT32  index_cnt;
	UINT32  temp_value_UINT32;
	char 	*p = NULL;

	switch (key_set_index) {
	//Note: >>>1st Key set is dedicate for secure boot usage<<<
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_field_start_index = 16;
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_field_start_index = 20;
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_field_start_index = 24;
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		key_field_start_index = 28;
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~3\r\n", (int)key_set_index);
		result = EFUSE_OPS_ERR;
		break;
	}

	if (result != EFUSE_SUCCESS) {
		return result;
	}
	temp_value_UINT32 = *(UINT32 *)(u32_key + 12);
	data[0] = (UINT32)(((temp_value_UINT32 & 0xFF000000) >> 24) | ((temp_value_UINT32 & 0xFF0000) >> 8) | ((temp_value_UINT32 & 0xFF00) << 8) | ((temp_value_UINT32 & 0xFF) << 24));

	temp_value_UINT32 = *(UINT32 *)(u32_key + 8);
	data[1] = (UINT32)(((temp_value_UINT32 & 0xFF000000) >> 24) | ((temp_value_UINT32 & 0xFF0000) >> 8) | ((temp_value_UINT32 & 0xFF00) << 8) | ((temp_value_UINT32 & 0xFF) << 24));

	temp_value_UINT32 = *(UINT32 *)(u32_key + 4);
	data[2] = (UINT32)(((temp_value_UINT32 & 0xFF000000) >> 24) | ((temp_value_UINT32 & 0xFF0000) >> 8) | ((temp_value_UINT32 & 0xFF00) << 8) | ((temp_value_UINT32 & 0xFF) << 24));

	temp_value_UINT32 = *(UINT32 *)(u32_key + 0);
	data[3] = (UINT32)(((temp_value_UINT32 & 0xFF000000) >> 24) | ((temp_value_UINT32 & 0xFF0000) >> 8) | ((temp_value_UINT32 & 0xFF00) << 8) | ((temp_value_UINT32 & 0xFF) << 24));


	p = __vmalloc(sizeof(OTP_PG), GFP_KERNEL, PAGE_KERNEL_EXEC);

	if(!p) {
		DBG_ERR("efuse program data allocate buffer error\r\n");
		return -1;
	}

	if(!pte_exec((pte_t)p)) {
		DBG_ERR("efuse program data allocate buffer not executable 0x%x\r\n", p);
	 	vfree(p);
		return -1;
	}

	memcpy((void *)p, (void *)OTP_PG, sizeof(OTP_PG));

	OTP_PG_MALLOC = (UINT32)p;


	for (index_cnt = 0; index_cnt < EFUSE_OTP_TOTAL_KEY_SET_FIELD; index_cnt++) {
		result = efuse_write_data(key_field_start_index + index_cnt, data[index_cnt]);
		if (result < 0) {
			DBG_ERR("[%d]set key => write addr[%2d][0x%08lx] fail\r\n", (int)(((key_field_start_index - 16) / 4) + 1), (int)(key_field_start_index + index_cnt), (UINT32)data[index_cnt]);
			break;
		}
	}

	vfree(p);
	isb();
	dsb();

	efuse_key_set_flag(key_set_index);

	return result;
}


/*
     efuse_read_param_ops

     efuse read system parameter

     @note for EFUSE_PARAM_DATA

     @param[in]     param   efuse system internal data field
     @param[in]      data   data ready to programmed

     @return
    -@b EFUSE_SUCCESS       success
    -@b EFUSE_UNKNOW_PARAM_ERR  unknow system internal data field
    -@b EFUSE_INVALIDATE_ERR    system internal data field invalidate
    -@b EFUSE_OPS_ERR       efuse operation error
*/
INT32 efuse_read_param_ops(EFUSE_PARAM_DATA param, UINT16 *data)
{
	INT32   i_ret = EFUSE_SUCCESS;
	UINT16  tmp_data;
	UINT32  paramdatacnt;
	UINT32  pri_param_id_no;

	switch (param) {
	case EFUSE_DDR2P_TRIM_DATA:
	case EFUSE_USBC2_TRIM_DATA:
	case EFUSE_HDMI_TRIM_DATA:
	case EFUSE_TVDC_TRIM_DATA:
	case EFUSE_USBC_TRIM_DATA:
	case EFUSE_DDRP_TRIM_DATA:
	case EFUSE_SLVSEC_TRIM_DATA:
	case EFUSE_VX1_TRIM_DATA:
		if (EFUSE_HDMI_TRIM_DATA == param) {
			pri_param_id_no = EFUSE_TRIM_DATA_HDMI_PRI;
		} else if (EFUSE_USBC_TRIM_DATA == param) {
			pri_param_id_no = EFUSE_TRIM_DATA_USB_PRI;
		} else if (EFUSE_DDRP_TRIM_DATA == param) {
			pri_param_id_no = EFUSE_TRIM_DATA_DDR_DRIVING_PRI;
		} else if (EFUSE_TVDC_TRIM_DATA == param) {
			pri_param_id_no = EFUSE_TRIM_DATA_TVDEC_PRI;
		} else if (EFUSE_VX1_TRIM_DATA == param) {
			pri_param_id_no = EFUSE_TRIM_DATA_VX1_PRI;
		} else if (EFUSE_SLVSEC_TRIM_DATA == param) {
			pri_param_id_no = EFUSE_TRIM_DATA_SLVSEC_PRI;
		} else if (EFUSE_USBC2_TRIM_DATA == param) {
			pri_param_id_no = EFUSE_TRIM_DATA_USB2_PRI;
		} else {
			pri_param_id_no = EFUSE_TRIM_DATA_DDR2_DRIVING_PRI;
		}
		for (paramdatacnt = 0; paramdatacnt < EFUSE_PARAM_OP_CNT; paramdatacnt++) {
			if (paramdatacnt == 0) {
				tmp_data = EFUSE_READDATA(pri_param_id_no) & 0xFFFF;
			} else {
				tmp_data = (EFUSE_READDATA(pri_param_id_no) >> 16) & 0xFFFF;
			}
			if (tmp_data < 0) {
				i_ret = EFUSE_OPS_ERR;
				DBG_DUMP("^RERR:PKG UID[%d] op error\r\n", paramdatacnt);
				continue;
			} else {
				*data = (tmp_data & 0x3FFF);

				i_ret = efuse_check_read_data(tmp_data);

				if (EFUSE_UNKNOW_PARAM_ERR == i_ret) {
					DBG_DUMP("^RERR:unknow param error\r\n");
				} else if (EFUSE_INVALIDATE_ERR == i_ret) {
					if (!paramdatacnt) {
						DBG_IND("^YWRN:[PRI] param invalidate\r\n");
					} else {
						DBG_IND("^YWRN:[SEC] param invalidate\r\n");
					}
				} else if (EFUSE_INACTIVE_ERR == i_ret) {
					if (!paramdatacnt) {
						DBG_IND("^YWRN:[PRI] param inactive\r\n");
					} else {
						DBG_IND("^YWRN:[SEC] param inactive\r\n");
					}
				} else {
					DBG_IND("[%d] success = 0\r\n", paramdatacnt);
					break;
				}
			}
		}
		break;

	default:
		i_ret = EFUSE_UNKNOW_PARAM_ERR;
		break;
	}
	return i_ret;
}



INT32 efuse_otp_set_key(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	char *p = __vmalloc(sizeof(OTP_OP), GFP_KERNEL, PAGE_KERNEL_EXEC);

	if(!p) {
		DBG_ERR("efuse read data allocate buffer error\r\n");
		return -1;
	}

	if(!pte_exec((pte_t)p)) {
		DBG_ERR("efuse set key allocate buffer not executable 0x%x\r\n", p);
	 	vfree(p);
		return -1;
	}

	memcpy((void *)p, (void *)OTP_OP, sizeof(OTP_OP));

	efuse_set_key(key_set_index, (UINT32)p);

	vfree(p);
	isb();
	dsb();
	return E_OK;
}

UINT32 dma_get_utilization(DMA_ID id)
{
	return dma_get_utilization_data(id, (UINT32)&DMA_USAGE_DATA[0]);
}


static int __init nvt_init_otp_mutex(void)
{
	mutex_init(&otp_lock);
	return 0;
}

core_initcall(nvt_init_otp_mutex);
EXPORT_SYMBOL(efuse_read_param_ops);
EXPORT_SYMBOL(efuse_otp_set_key);
EXPORT_SYMBOL(dma_get_utilization);
EXPORT_SYMBOL(efuse_write_key);
EXPORT_SYMBOL(efuse_key_get_flag);
