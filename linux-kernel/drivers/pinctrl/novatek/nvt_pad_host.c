/*
    PAD controller

    Sets the PAD control of each pin.

    @file       pad.c
    @ingroup    mIDrvSys_PAD
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#include <mach/pad.h>
#include <mach/pad_reg.h>
#include "nvt_pinmux.h"

static u32 TOP_PAD_REG_BASE;
#define PAD_GETREG(ofs)         readl((void __iomem *)(TOP_PAD_REG_BASE + ofs))
#define PAD_SETREG(ofs,value)   writel(value, (void __iomem *)(TOP_PAD_REG_BASE + ofs))

static struct semaphore pad_sem;
#define loc_cpu() down(&pad_sem);
#define unl_cpu() up(&pad_sem);

struct pad_pullupdown_pin {
	UINT16 pin;
	UINT8 pup;
	UINT8 bit_offset;
};

const struct pad_pullupdown_pin pad_pullupdown_table [] = {
	/*PUPD0 0x00*/
	{PAD_PIN_CGPIO0, 0, 0}, {PAD_PIN_CGPIO1, 0, 2}, {PAD_PIN_CGPIO2, 0, 4}, {PAD_PIN_CGPIO3, 0, 6},
	{PAD_PIN_CGPIO4, 0, 8}, {PAD_PIN_CGPIO5, 0, 10}, {PAD_PIN_CGPIO6, 0, 12}, {PAD_PIN_CGPIO7, 0, 14},
	{PAD_PIN_CGPIO8, 0, 16}, {PAD_PIN_CGPIO9, 0, 18}, {PAD_PIN_CGPIO10, 0, 20}, {PAD_PIN_CGPIO11, 0, 22},
	{PAD_PIN_CGPIO12, 0, 24}, {PAD_PIN_CGPIO13, 0, 26}, {PAD_PIN_CGPIO14, 0, 28}, {PAD_PIN_CGPIO15, 0, 30},
	/*PUPD1 0x04*/
	{PAD_PIN_CGPIO16, 1, 0}, {PAD_PIN_CGPIO17, 1, 2}, {PAD_PIN_CGPIO18, 1, 4}, {PAD_PIN_CGPIO19, 1, 6},
	{PAD_PIN_CGPIO20, 1, 8}, {PAD_PIN_CGPIO21, 1, 10}, {PAD_PIN_CGPIO22, 1, 12}, {PAD_PIN_CGPIO23, 1, 14},
	{PAD_PIN_CGPIO24, 1, 16}, {PAD_PIN_CGPIO25, 1, 18}, {PAD_PIN_CGPIO26, 1, 20}, {PAD_PIN_CGPIO27, 1, 22},
	{PAD_PIN_CGPIO28, 1, 24}, {PAD_PIN_CGPIO29, 1, 26}, {PAD_PIN_CGPIO30, 1, 28}, {PAD_PIN_CGPIO31, 1, 30},
	/*PUPD2 0x08*/
	{PAD_PIN_CGPIO32, 2, 0}, {PAD_PIN_CGPIO33, 2, 2},
	/*PUPD3 0x0C*/
	{PAD_PIN_SGPIO0, 3, 0}, {PAD_PIN_SGPIO1, 3, 2}, {PAD_PIN_SGPIO2, 3, 4}, {PAD_PIN_SGPIO3, 3, 6},
	{PAD_PIN_SGPIO4, 3, 8}, {PAD_PIN_SGPIO5, 3, 10}, {PAD_PIN_SGPIO6, 3, 12}, {PAD_PIN_SGPIO7, 3, 14},
	{PAD_PIN_SGPIO8, 3, 16}, {PAD_PIN_SGPIO9, 3, 18}, {PAD_PIN_SGPIO10, 3, 20}, {PAD_PIN_SGPIO11, 3, 22},
	/*PUPD4 0x10*/
	{PAD_PIN_PGPIO0, 4, 0}, {PAD_PIN_PGPIO1, 4, 2}, {PAD_PIN_PGPIO2, 4, 4}, {PAD_PIN_PGPIO3, 4, 6},
	{PAD_PIN_PGPIO4, 4, 8}, {PAD_PIN_PGPIO5, 4, 10}, {PAD_PIN_PGPIO6, 4, 12}, {PAD_PIN_PGPIO7, 4, 14},
	{PAD_PIN_PGPIO8, 4, 16}, {PAD_PIN_PGPIO9, 4, 18}, {PAD_PIN_PGPIO10, 4, 20}, {PAD_PIN_PGPIO11, 4, 22},
	{PAD_PIN_PGPIO12, 4, 24}, {PAD_PIN_PGPIO13, 4, 26}, {PAD_PIN_PGPIO14, 4, 28}, {PAD_PIN_PGPIO15, 4, 30},
	/*PUPD5 0x14*/
	{PAD_PIN_PGPIO16, 5, 0}, {PAD_PIN_PGPIO17, 5, 2}, {PAD_PIN_PGPIO18, 5, 4}, {PAD_PIN_PGPIO19, 5, 6},
	{PAD_PIN_PGPIO20, 5, 8}, {PAD_PIN_PGPIO21, 5, 10}, {PAD_PIN_PGPIO22, 5, 12}, {PAD_PIN_PGPIO23, 5, 14},
	{PAD_PIN_PGPIO24, 5, 16}, {PAD_PIN_PGPIO25, 5, 18}, {PAD_PIN_DGPIO8, 5, 20}, {PAD_PIN_PGPIO27, 5, 22},
	{PAD_PIN_PGPIO28, 5, 24}, {PAD_PIN_PGPIO29, 5, 26}, {PAD_PIN_PGPIO30, 5, 28}, {PAD_PIN_PGPIO31, 5, 30},
	/*PUPD6 0x18*/
	{PAD_PIN_PGPIO32, 6, 0}, {PAD_PIN_PGPIO33, 6, 2}, {PAD_PIN_PGPIO34, 6, 4}, {PAD_PIN_PGPIO35, 6, 6},
	{PAD_PIN_PGPIO36, 6, 8}, {PAD_PIN_PGPIO37, 6, 10}, {PAD_PIN_PGPIO38, 6, 12}, {PAD_PIN_PGPIO39, 6, 14},
	{PAD_PIN_PGPIO40, 6, 16}, {PAD_PIN_PGPIO41, 6, 18}, {PAD_PIN_PGPIO42, 6, 20}, {PAD_PIN_PGPIO43, 6, 22},
	{PAD_PIN_PGPIO44, 6, 24}, {PAD_PIN_PGPIO45, 6, 26}, {PAD_PIN_PGPIO46, 6, 28}, {PAD_PIN_PGPIO47, 6, 30},
	/*PUPD7 0x1C*/
	{PAD_PIN_LGPIO0, 7, 0}, {PAD_PIN_LGPIO1, 7, 2}, {PAD_PIN_LGPIO2, 7, 4}, {PAD_PIN_LGPIO3, 7, 6},
	{PAD_PIN_LGPIO4, 7, 8}, {PAD_PIN_LGPIO5, 7, 10}, {PAD_PIN_LGPIO6, 7, 12}, {PAD_PIN_LGPIO7, 7, 14},
	{PAD_PIN_LGPIO8, 7, 16}, {PAD_PIN_LGPIO9, 7, 18}, {PAD_PIN_LGPIO10, 7, 20}, {PAD_PIN_LGPIO11, 7, 22},
	{PAD_PIN_LGPIO12, 7, 24}, {PAD_PIN_LGPIO13, 7, 26}, {PAD_PIN_LGPIO14, 7, 28}, {PAD_PIN_LGPIO15, 7, 30},
	/*PUPD8 0x20*/
	{PAD_PIN_LGPIO16, 8, 0}, {PAD_PIN_LGPIO17, 8, 2}, {PAD_PIN_LGPIO18, 8, 4}, {PAD_PIN_LGPIO19, 8, 6},
	{PAD_PIN_LGPIO20, 8, 8}, {PAD_PIN_LGPIO21, 8, 10}, {PAD_PIN_LGPIO22, 8, 12}, {PAD_PIN_LGPIO23, 8, 14},
	{PAD_PIN_LGPIO24, 8, 16}, {PAD_PIN_LGPIO25, 8, 18}, {PAD_PIN_LGPIO26, 8, 20}, {PAD_PIN_LGPIO27, 8, 22},
	{PAD_PIN_LGPIO28, 8, 24}, {PAD_PIN_LGPIO29, 8, 26}, {PAD_PIN_LGPIO30, 8, 28}, {PAD_PIN_LGPIO31, 8, 30},
	/*PUPD9 0x24*/
	{PAD_PIN_LGPIO32, 9, 0},
	/*PUPD10 0x28*/
	{PAD_PIN_DGPIO0, 10, 0}, {PAD_PIN_DGPIO1, 10, 2}, {PAD_PIN_DGPIO2, 10, 4}, {PAD_PIN_DGPIO3, 10, 6},
	{PAD_PIN_DGPIO4, 10, 8}, {PAD_PIN_DGPIO5, 10, 10}, {PAD_PIN_DGPIO6, 10, 12}, {PAD_PIN_DGPIO7, 10, 14},
	{PAD_PIN_DGPIO8, 10, 16}, {PAD_PIN_DGPIO9, 10, 18}, {PAD_PIN_DGPIO10, 10, 20}, {PAD_PIN_DGPIO11, 10, 22},
	{PAD_PIN_DGPIO12, 10, 24},
};


/*
    Lookup pad alias to actual bits position.

    Lookup pad alias to actual bits position.

    @param[in]      pin Pad alias
    @param[out]     dwofs Register offset address
    @param[out]     bitOffset Bit offset position inside the register offset address

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
static ER pad_reg_lookup(UINT pin, UINT *dwofs, UINT *bit_offset)
{
	UINT i, len;

	len = sizeof(pad_pullupdown_table) / sizeof(struct pad_pullupdown_pin);
	for (i = 0; i < len; i++) {
		if (pad_pullupdown_table[i].pin == pin) {
			*dwofs = pad_pullupdown_table[i].pup;
			*bit_offset = pad_pullupdown_table[i].bit_offset;
			return E_OK;
		}
	}

	*dwofs   = 0;
	*bit_offset  = 0;

	return E_SYS;
}

/**
    Set a pin pull up/down/keeper.

    Set a pin pull up/down/keeper.

    @param[in]  pin Pin name. For example PAD_PIN_xxxx. See pad.h.
    @param[in]  pulltype PAD_NONE or PAD_PULLUP or PAD_PULLDOWN or PAD_KEEPER

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_set_pull_updown(UINT32 pin, PAD_PULL pulltype)
{
	ER err;
	UINT dw_ofs, bit_ofs;
	REGVALUE    reg_data;

	err = pad_reg_lookup(pin, &dw_ofs, &bit_ofs);
	if (err == E_OK) {
		loc_cpu();
		reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_ofs << 2));
		reg_data &= ~(3 << bit_ofs);
		reg_data |= (pulltype << bit_ofs);
		PAD_SETREG(PAD_PUPD0_REG_OFS + (dw_ofs << 2), reg_data);
		unl_cpu();
		return E_OK;
	} else
		return E_SYS;
}
EXPORT_SYMBOL(pad_set_pull_updown);
/**
    Get a pin pull up/down/keeper

    Get a pin pull up/down/keeper

    @param[in]      pin         Pin name. For example PAD_PIN_xxxx. See pad.h.
    @param[out]     pulltype    PAD_NONE or PAD_PULLUP or PAD_PULLDOWN or PAD_KEEPER

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_get_pull_updown(UINT32 pin, PAD_PULL *pulltype)
{
	ER err;
	UINT dw_ofs, bit_ofs;
	REGVALUE    reg_data;

	err = pad_reg_lookup(pin, &dw_ofs, &bit_ofs);
	if (err == E_OK) {
		reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_ofs << 2));
		*pulltype = ((reg_data >> bit_ofs) & 0x3);

		return E_OK;
	} else
		return E_SYS;
}
EXPORT_SYMBOL(pad_get_pull_updown);
/**
    Set driving/sink.

    Set driving/sink.

    @param[in] name     For example PAD_DS_xxxx. See pad.h.
    @param[in] driving  PAD_DRIVINGSINK_xxMA. See pad.h.

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_set_drivingsink(UINT32 name, PAD_DRIVINGSINK driving)
{
	UINT32      dw_ofs, bit_ofs, bitMask;
	REGVALUE    reg_data;

	if (name & PAD_DS_GROUP6_16) {
		name -= PAD_DS_GROUP6_16;
		if (driving & PAD_DRIVINGSINK_6MA) {
			driving = 0;
		} else if (driving & PAD_DRIVINGSINK_16MA) {
			driving = 1;
		} else {
			return E_PAR;
		}

		dw_ofs  = (name >> 4) << 2;
		bit_ofs = (name % 16) << 1;
		bitMask = 0x03;
	} else if (name & PAD_DS_GROUP5_40) {
		if ((name >= PAD_DS_CGPIO16) && (name <= PAD_DS_CGPIO23)) {
			dw_ofs  = PAD_DS11_REG_OFS - PAD_DS_REG_OFS;
			bit_ofs = (name - (PAD_DS_CGPIO16)) * 4;

		} else {
			dw_ofs  = PAD_DS12_REG_OFS - PAD_DS_REG_OFS;
			bit_ofs = (name - (PAD_DS_CGPIO24)) * 4;
		}

		bitMask = 0x07;

		if (driving & PAD_DRIVINGSINK_5MA) {
			driving = 0;
		} else if (driving & PAD_DRIVINGSINK_10MA) {
			driving = 1;
		} else if (driving & PAD_DRIVINGSINK_15MA) {
			driving = 2;
		} else if (driving & PAD_DRIVINGSINK_20MA) {
			driving = 3;
		} else if (driving & PAD_DRIVINGSINK_25MA) {
			driving = 4;
		} else if (driving & PAD_DRIVINGSINK_30MA) {
			driving = 5;
		} else if (driving & PAD_DRIVINGSINK_35MA) {
			driving = 6;
		} else if (driving & PAD_DRIVINGSINK_40MA) {
			driving = 7;
		} else {
			return E_PAR;
		}
	} else if (name & PAD_DS_GROUP8) {
		return E_PAR;
	} else if (name & PAD_DS_GROUP16) {
		return E_PAR;
	} else {
		if (driving & PAD_DRIVINGSINK_4MA) {
			driving = 0;
		} else if (driving & PAD_DRIVINGSINK_10MA) {
			driving = 1;
		} else {
			return E_PAR;
		}

		dw_ofs  = (name >> 4) << 2;
		bit_ofs = (name % 16) << 1;
		bitMask = 0x03;

	}

	/*race condition protect. enter critical section*/
	loc_cpu();

	reg_data = PAD_GETREG(PAD_DS_REG_OFS + dw_ofs);

	reg_data &= ~(bitMask << bit_ofs);
	reg_data |= (driving << bit_ofs);

	PAD_SETREG(PAD_DS_REG_OFS + dw_ofs, reg_data);

	/*race condition protect. leave critical section*/
	unl_cpu();

	return E_OK;
}
EXPORT_SYMBOL(pad_set_drivingsink);

/**
    Get driving/sink.

    Get driving/sink.

    @param[in]  name        For example PAD_DS_xxxx. See pad.h.
    @param[out] driving     PAD_DRIVINGSINK_xxMA. See pad.h.

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_get_drivingsink(UINT32 name, PAD_DRIVINGSINK *driving)
{
	UINT32      dw_ofs, bit_ofs, bitMask;
	REGVALUE    reg_data;

	if (name & PAD_DS_GROUP5_40) {
		if ((name >= PAD_DS_CGPIO16) && (name <= PAD_DS_CGPIO23)) {
			dw_ofs  = PAD_DS11_REG_OFS - PAD_DS_REG_OFS;
			bitMask = 0x07;
			bit_ofs = (name - (PAD_DS_CGPIO16)) * 4;
		} else if ((name >= PAD_DS_CGPIO24) && (name <= PAD_DS_CGPIO27)) {
			dw_ofs  = PAD_DS12_REG_OFS - PAD_DS_REG_OFS;
			bitMask = 0x07;
			bit_ofs = (name - (PAD_DS_CGPIO24)) * 4;
		} else {
			return E_PAR;
		}
	} else if (name & PAD_DS_GROUP6_16) {
		dw_ofs  = ((name & (~(PAD_DS_GROUP6_16))) >> 4) << 2;
		bit_ofs = ((name & (~(PAD_DS_GROUP6_16))) % 16) << 1;
		bitMask = 0x03;
	} else if (name & PAD_DS_GROUP8) {
		dw_ofs  = ((name & (~(PAD_DS_GROUP8))) >> 4) << 2;
		bit_ofs = ((name & (~(PAD_DS_GROUP8))) % 16) << 1;
		bitMask = 0x01;
	} else if (name & PAD_DS_GROUP16) {
		dw_ofs  = ((name & (~(PAD_DS_GROUP16))) >> 4) << 2;
		bit_ofs = ((name & (~(PAD_DS_GROUP16))) % 16) << 1;
		bitMask = 0x01;
	} else {
		dw_ofs  = ((name & (~(PAD_DS_GROUP4_10))) >> 4) << 2;
		bit_ofs = ((name & (~(PAD_DS_GROUP4_10))) % 16) << 1;
		bitMask = 0x03;
	}

	reg_data = PAD_GETREG(PAD_DS_REG_OFS + dw_ofs);

	*driving = (reg_data >> bit_ofs) & bitMask;

	if (name & (PAD_DS_GROUP5_40)) {
		switch ((UINT32)*driving) {
		case 0:
			*driving = PAD_DRIVINGSINK_5MA;
			break;
		case 1:
			*driving = PAD_DRIVINGSINK_10MA;
			break;
		case 2:
			*driving = PAD_DRIVINGSINK_15MA;
			break;
		case 3:
			*driving = PAD_DRIVINGSINK_20MA;
			break;
		case 4:
			*driving = PAD_DRIVINGSINK_25MA;
			break;
		case 5:
			*driving = PAD_DRIVINGSINK_30MA;
			break;
		case 6:
			*driving = PAD_DRIVINGSINK_35MA;
			break;
		case 7:
		default:
			*driving = PAD_DRIVINGSINK_40MA;
			break;
		}
	} else if (name & PAD_DS_GROUP6_16) {
		switch ((UINT32)*driving) {
		case 0:
			*driving = PAD_DRIVINGSINK_6MA;
			break;
		case 1:
		default:
			*driving = PAD_DRIVINGSINK_16MA;
			break;
		}
	} else if (name & PAD_DS_GROUP8) {
		*driving = PAD_DRIVINGSINK_8MA;
	} else if (name & PAD_DS_GROUP16) {
		*driving = PAD_DRIVINGSINK_16MA;
	} else {
		switch ((UINT32)*driving) {
		case 0:
			*driving = PAD_DRIVINGSINK_4MA;
			break;
		case 1:
		default:
			*driving = PAD_DRIVINGSINK_10MA;
			break;
		}
	}

	return E_OK;
}
EXPORT_SYMBOL(pad_get_drivingsink);
/**
    Set pad power.

    Set pad power.

    @param[in] pad_power  pointer to pad power struct

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_set_power(PAD_POWER_STRUCT *pad_power)
{
	union PAD_PWR_REG pad_power_reg = {0};
	union PAD_PWR1_REG pad_power1_reg = {0};
	union PAD_PWR2_REG pad_power2_reg = {0};

	if (pad_power == NULL) {
		return E_PAR;
	}
	if (pad_power->pad_power_id == PAD_POWERID_MC1) {
		pad_power_reg.bit.MC_POWER0 = pad_power->pad_power;
		pad_power_reg.bit.MC_SEL = pad_power->bias_current;
		pad_power_reg.bit.MC_OPSEL = pad_power->opa_gain;
		pad_power_reg.bit.MC_PULLDOWN = pad_power->pull_down;
		pad_power_reg.bit.MC_REGULATOR_EN = pad_power->enable;

		loc_cpu();
		PAD_SETREG(PAD_PWR_REG_OFS, pad_power_reg.reg);
		unl_cpu();

		return E_OK;
	} else if (pad_power->pad_power_id == PAD_POWERID_MC0) {
		pad_power1_reg.bit.MC_POWER0 = pad_power->pad_power;
		pad_power1_reg.bit.MC_SEL = pad_power->bias_current;
		pad_power1_reg.bit.MC_OPSEL = pad_power->opa_gain;
		pad_power1_reg.bit.MC_PULLDOWN = pad_power->pull_down;
		pad_power1_reg.bit.MC_REGULATOR_EN = pad_power->enable;

		loc_cpu();
		PAD_SETREG(PAD_PWR1_REG_OFS, pad_power1_reg.reg);
		unl_cpu();

		return E_OK;
	} else if (pad_power->pad_power_id == PAD_POWERID_ADC) {
		pad_power2_reg.bit.ADC_POWER0 = pad_power->pad_power;
		pad_power2_reg.bit.ADC_SEL = pad_power->bias_current;
		pad_power2_reg.bit.ADC_OPSEL = pad_power->opa_gain;
		pad_power2_reg.bit.ADC_PULLDOWN = pad_power->pull_down;
		pad_power2_reg.bit.ADC_REGULATOR_EN = pad_power->enable;
		pad_power2_reg.bit.ADC_VAD = pad_power->pad_vad;

		loc_cpu();
		PAD_SETREG(PAD_PWR2_REG_OFS, pad_power2_reg.reg);
		unl_cpu();

		return E_OK;

	} else {
		return E_PAR;
	}

}
EXPORT_SYMBOL(pad_set_power);
/**
    Get pad power.

    get pad power.

    @param[in] pad_power  pointer to pad power struct
*/
void pad_get_power(PAD_POWER_STRUCT *pad_power)
{
	union PAD_PWR_REG pad_power_reg = {0};
	union PAD_PWR1_REG pad_power1_reg = {0};
	union PAD_PWR2_REG pad_power2_reg = {0};

	if (pad_power->pad_power_id == PAD_POWERID_MC1) {
		pad_power_reg.reg = PAD_GETREG(PAD_PWR_REG_OFS);

		pad_power->pad_power = pad_power_reg.bit.MC_POWER0;
		pad_power->bias_current = pad_power_reg.bit.MC_SEL;
		pad_power->opa_gain = pad_power_reg.bit.MC_OPSEL;
		pad_power->pull_down = pad_power_reg.bit.MC_PULLDOWN;
		pad_power->enable = pad_power_reg.bit.MC_REGULATOR_EN;
	} else if (pad_power->pad_power_id == PAD_POWERID_MC0) {
		pad_power1_reg.reg = PAD_GETREG(PAD_PWR1_REG_OFS);

		pad_power->pad_power = pad_power1_reg.bit.MC_POWER0;
		pad_power->bias_current = pad_power1_reg.bit.MC_SEL;
		pad_power->opa_gain = pad_power1_reg.bit.MC_OPSEL;
		pad_power->pull_down = pad_power1_reg.bit.MC_PULLDOWN;
		pad_power->enable = pad_power1_reg.bit.MC_REGULATOR_EN;
	} else if (pad_power->pad_power_id == PAD_POWERID_ADC) {
		pad_power2_reg.reg = PAD_GETREG(PAD_PWR2_REG_OFS);

		pad_power->pad_power = pad_power2_reg.bit.ADC_POWER0;
		pad_power->bias_current = pad_power2_reg.bit.ADC_SEL;
		pad_power->opa_gain = pad_power2_reg.bit.ADC_OPSEL;
		pad_power->pull_down = pad_power2_reg.bit.ADC_PULLDOWN;
		pad_power->enable = pad_power2_reg.bit.ADC_REGULATOR_EN;
		pad_power->pad_vad = pad_power2_reg.bit.ADC_VAD;
	}

}
EXPORT_SYMBOL(pad_get_power);

ER pad_init(struct nvt_pinctrl_info *info, u32 nr_pad)
{
	int cnt, ret = 0;

	TOP_PAD_REG_BASE = (u32) info->pad_base;
	sema_init(&pad_sem, 1);

	for (cnt = 0; cnt < nr_pad; cnt++) {
		ret = pad_set_pull_updown(info->pad[cnt].pad_gpio_pin, info->pad[cnt].direction);
		if (ret) {
			pr_err("pad pin 0x%x pull failed!\n", info->pad[cnt].pad_gpio_pin);
			return ret;
		}

		ret = pad_set_drivingsink(info->pad[cnt].pad_ds_pin, info->pad[cnt].driving);
		if (ret) {
			pr_err("pad pin 0x%x driving failed!\n", info->pad[cnt].pad_ds_pin);
			return ret;
		}
	}

	return E_OK;
}
