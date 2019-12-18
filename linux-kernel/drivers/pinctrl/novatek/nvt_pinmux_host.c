/*
	Pinmux module driver.

	This file is the driver of Piumux module.

	@file		nvt_pinmux_host.c
	@ingroup
	@note		Nothing.

	Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#include "nvt_pinmux.h"

static struct semaphore top_sem;
#define loc_cpu() down(&top_sem);
#define unl_cpu() up(&top_sem);

static uint32_t mipi_lvds_sensor[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static int mclk2_sensor[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static int mclk_sensor = 0;
typedef int (*PINMUX_CONFIG_HDL)(uint32_t);
static uint32_t mi_display = 0;/*record MI is linked to LCD or LCD2*/
static uint32_t ui_sie6_16_12 = 0;

static int pinmux_config_sdio(uint32_t config);
static int pinmux_config_sdio2(uint32_t config);
static int pinmux_config_sdio3(uint32_t config);
static int pinmux_config_nand(uint32_t config);
static int pinmux_config_sensor(uint32_t config);
static int pinmux_config_sensor2(uint32_t config);
static int pinmux_config_sensor3(uint32_t config);
static int pinmux_config_sensor4(uint32_t config);
static int pinmux_config_sensor5(uint32_t config);
static int pinmux_config_sensor6(uint32_t config);
static int pinmux_config_sensor7(uint32_t config);
static int pinmux_config_sensor8(uint32_t config);
static int pinmux_config_mipi_lvds(uint32_t config);
static int pinmux_config_i2c(uint32_t config);
static int pinmux_config_sif(uint32_t config);
static int pinmux_config_uart(uint32_t config);
static int pinmux_config_spi(uint32_t config);
static int pinmux_config_remote(uint32_t config);
static int pinmux_config_pwm(uint32_t config);
static int pinmux_config_audio(uint32_t config);
static int pinmux_config_lcd(uint32_t config);
static int pinmux_config_lcd2(uint32_t config);
static int pinmux_config_tv(uint32_t config);
static int pinmux_config_hdmi(uint32_t config);
static int pinmux_config_eth(uint32_t config);
static int pinmux_config_canbus(uint32_t config);

union TOP_REG0 top_reg0;
union TOP_REG1 top_reg1;
union TOP_REG2 top_reg2;
union TOP_REG3 top_reg3;
union TOP_REG4 top_reg4;
union TOP_REG5 top_reg5;
union TOP_REG6 top_reg6;
union TOP_REG7 top_reg7;
union TOP_REG8 top_reg8;
union TOP_REG9 top_reg9;
union TOP_REG10 top_reg10;
union TOP_REG11 top_reg11;
union TOP_REG12 top_reg12;
union TOP_REGCGPIO0 top_reg_cgpio0;
union TOP_REGCGPIO1 top_reg_cgpio1;
union TOP_REGPGPIO0 top_reg_pgpio0;
union TOP_REGPGPIO1 top_reg_pgpio1;
union TOP_REGSGPIO0 top_reg_sgpio0;
union TOP_REGLGPIO0 top_reg_lgpio0;
union TOP_REGLGPIO1 top_reg_lgpio1;
union TOP_REGDGPIO0 top_reg_dgpio0;
union TOP_REGHGPI0  top_reg_hgpi0;

static uint32_t disp_pinmux_config[] =
{
	PINMUX_DISPMUX_SEL_NONE,            // LCD
	PINMUX_DISPMUX_SEL_NONE,            // LCD2
	PINMUX_TV_HDMI_CFG_NORMAL,          // TV
	PINMUX_TV_HDMI_CFG_NORMAL           // HDMI
};

static PINMUX_CONFIG_HDL pinmux_config_hdl[] =
{
	pinmux_config_sdio,
	pinmux_config_sdio2,
	pinmux_config_sdio3,
	pinmux_config_nand,
	pinmux_config_sensor,
	pinmux_config_sensor2,
	pinmux_config_sensor3,
	pinmux_config_sensor4,
	pinmux_config_sensor5,
	pinmux_config_sensor6,
	pinmux_config_sensor7,
	pinmux_config_sensor8,
	pinmux_config_mipi_lvds,
	pinmux_config_i2c,
	pinmux_config_sif,
	pinmux_config_uart,
	pinmux_config_spi,
	pinmux_config_remote,
	pinmux_config_pwm,
	pinmux_config_audio,
	pinmux_config_lcd,
	pinmux_config_lcd2,
	pinmux_config_tv,
	pinmux_config_hdmi,
	pinmux_config_eth,
	pinmux_config_canbus
};

static void gpio_info_show(struct nvt_pinctrl_info *info, u32 gpio_number, u32 start_offset)
{
	int i = 0, j = 0;
	u32 reg_value;
	char* gpio_name[] = {"C_GPIO", "P_GPIO", "S_GPIO", "L_GPIO", "D_GPIO", "H_GPIO"};
	char name[5];

	if (start_offset == TOP_REGCGPIO0_OFS)
		strcpy(name, gpio_name[0]);
	else if (start_offset == TOP_REGPGPIO0_OFS)
		strcpy(name, gpio_name[1]);
	else if (start_offset == TOP_REGSGPIO0_OFS)
		strcpy(name, gpio_name[2]);
	else if (start_offset == TOP_REGLGPIO0_OFS)
		strcpy(name, gpio_name[3]);
	else if (start_offset == TOP_REGDGPIO0_OFS)
		strcpy(name, gpio_name[4]);
	else if (start_offset == TOP_REGHGPI0_OFS)
		strcpy(name, gpio_name[5]);


	if (gpio_number > 0x20) {
		reg_value = TOP_GETREG(info, start_offset);

		for (i = 0; i < 0x20; i++) {
			if (reg_value & (1 << i))
				pr_info("%s%-2d       GPIO\n", name, i);
			else
				pr_info("%s%-2d     FUNCTION\n", name, i);
		}

		reg_value = TOP_GETREG(info, start_offset + 0x4);

		for (j = 0; j < (gpio_number - 0x20); j++) {
			if (reg_value & (1 << j))
				pr_info("%s%-2d       GPIO\n", name, i);
			else
				pr_info("%s%-2d     FUNCTION\n", name, i);
			i++;
		}
	} else {
		reg_value = TOP_GETREG(info, start_offset);
		for (i = 0; i < gpio_number; i++) {
			if (reg_value & (1 << i))
				pr_info("%s%-2d       GPIO\n", name, i);
			else
				pr_info("%s%-2d     FUNCTION\n", name, i);
		}
	}

}

void pinmux_gpio_parsing(struct nvt_pinctrl_info *info)
{
	pr_info("\n  PIN         STATUS\n");

	gpio_info_show(info, C_GPIO_NUM, TOP_REGCGPIO0_OFS);
	gpio_info_show(info, P_GPIO_NUM, TOP_REGPGPIO0_OFS);
	gpio_info_show(info, S_GPIO_NUM, TOP_REGSGPIO0_OFS);
	gpio_info_show(info, L_GPIO_NUM, TOP_REGLGPIO0_OFS);
	gpio_info_show(info, D_GPIO_NUM, TOP_REGDGPIO0_OFS);
	gpio_info_show(info, H_GPIO_NUM, TOP_REGHGPI0_OFS);
}

void pinmux_preset(struct nvt_pinctrl_info *info)
{
	sema_init(&top_sem, 1);
}

/**
	Get Display PINMUX setting

	Get Display PINMUX setting.
	Display driver (LCD/TV/HDMI) can get mode setting from pinmux_init()

	@param[in] id   LCD ID
			- @b PINMUX_DISP_ID_LCD: 1st LCD
			- @b PINMUX_DISP_ID_LCD2: 2nd LCD
			- @b PINMUX_DISP_ID_TV: TV
			- @b PINMUX_DISP_ID_HDMI: HDMI

	@return LCD pinmux setting
*/
PINMUX_LCDINIT pinmux_get_dispmode(PINMUX_FUNC_ID id)
{
	if (id <= PINMUX_FUNC_ID_LCD2) {
		return disp_pinmux_config[id] & ~(PINMUX_DISPMUX_SEL_MASK | PINMUX_PMI_CFG_MASK | PINMUX_LCD_SEL_FEATURE_MSK);
	} else if (id <= PINMUX_FUNC_ID_HDMI) {
		return disp_pinmux_config[id] & ~PINMUX_HDMI_CFG_MASK;
	}

	return 0;
}
EXPORT_SYMBOL(pinmux_get_dispmode);

/**
	Read pinmux data from controller base

	Read pinmux data from controller base

	@param[in] info	nvt_pinctrl_info
*/

void pinmux_parsing(struct nvt_pinctrl_info *info)
{
	u32 value;
	union TOP_REG0 local_top_reg0;
	union TOP_REG1 local_top_reg1;
	union TOP_REG2 local_top_reg2;
	union TOP_REG3 local_top_reg3;
	union TOP_REG4 local_top_reg4;
	union TOP_REG5 local_top_reg5;
	union TOP_REG6 local_top_reg6;
	union TOP_REG7 local_top_reg7;
	union TOP_REG8 local_top_reg8;
	union TOP_REG9 local_top_reg9;
	union TOP_REG10 local_top_reg10;
	union TOP_REG11 local_top_reg11;
	union TOP_REG12 local_top_reg12;
	union TOP_REGSGPIO0 local_top_reg_sgpio0;
	union TOP_REGHGPI0 local_top_reg_hgpi0;

	down(&top_sem);

	local_top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	local_top_reg1.reg = TOP_GETREG(info, TOP_REG1_OFS);
	local_top_reg2.reg = TOP_GETREG(info, TOP_REG2_OFS);
	local_top_reg3.reg = TOP_GETREG(info, TOP_REG3_OFS);
	local_top_reg4.reg = TOP_GETREG(info, TOP_REG4_OFS);
	local_top_reg5.reg = TOP_GETREG(info, TOP_REG5_OFS);
	local_top_reg6.reg = TOP_GETREG(info, TOP_REG6_OFS);
	local_top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);
	local_top_reg8.reg = TOP_GETREG(info, TOP_REG8_OFS);
	local_top_reg9.reg = TOP_GETREG(info, TOP_REG9_OFS);
	local_top_reg10.reg = TOP_GETREG(info, TOP_REG10_OFS);
	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);
	local_top_reg12.reg = TOP_GETREG(info, TOP_REG12_OFS);
	local_top_reg_sgpio0.reg = TOP_GETREG(info, TOP_REGSGPIO0_OFS);
	local_top_reg_hgpi0.reg = TOP_GETREG(info, TOP_REGHGPI0_OFS);


	/*Parsing SDIO1*/
	value = (local_top_reg1.bit.SDIO_EXIST ? PIN_SDIO_CFG_4BITS : 0);
	info->top_pinmux[PIN_FUNC_SDIO].config = value;
	info->top_pinmux[PIN_FUNC_SDIO].pin_function = PIN_FUNC_SDIO;

	/*Parsing SDIO2*/
	value = (local_top_reg1.bit.SDIO2_EXIST ? PIN_SDIO_CFG_4BITS : 0);
	info->top_pinmux[PIN_FUNC_SDIO2].config = value;
	info->top_pinmux[PIN_FUNC_SDIO2].pin_function = PIN_FUNC_SDIO2;

	/*Parsing SDIO3*/
	if (local_top_reg1.bit.SDIO3_EXIST) {
		if (local_top_reg1.bit.SDIO3_MUX_SEL)
			value = (local_top_reg1.bit.SDIO3_BUS_WIDTH ? PIN_SDIO_CFG_8BITS : PIN_SDIO_CFG_4BITS) | \
				PIN_SDIO_CFG_2ND_PINMUX;
		else
			value = PIN_SDIO_CFG_4BITS | PIN_SDIO_CFG_1ST_PINMUX;
	} else
		value = 0;

	info->top_pinmux[PIN_FUNC_SDIO3].config = value;
	info->top_pinmux[PIN_FUNC_SDIO3].pin_function = PIN_FUNC_SDIO3;

	/*Parsing NAND*/
	if (local_top_reg1.bit.FSPI_EXIST) {
		value = (local_top_reg1.bit.NAND_CS_NUM ? PIN_NAND_CFG_2CS : PIN_NAND_CFG_1CS);
#ifdef CONFIG_MTD_SPINAND
		value |= PIN_NAND_CFG_SPI_NAND;
#elif defined(CONFIG_MTD_SPINOR)
		value |= PIN_NAND_CFG_SPI_NOR;
#endif
	} else
		value = 0;

	info->top_pinmux[PIN_FUNC_NAND].config = value;
	info->top_pinmux[PIN_FUNC_NAND].pin_function = PIN_FUNC_NAND;

	/*Parsing SENSOR*/
	value = mipi_lvds_sensor[0];

	switch (top_reg3.bit.SENSOR) {
	case SENSOR_ENUM_8BITS:
		value |= PIN_SENSOR_CFG_8BITS;
		break;
	case SENSOR_ENUM_10BITS:
		value |= PIN_SENSOR_CFG_10BITS;
		break;
	case SENSOR_ENUM_12BITS:
		value |= PIN_SENSOR_CFG_12BITS;
		break;
	default:
		break;
	}

	if (mipi_lvds_sensor[0] && (local_top_reg4.bit.XVS_XHS_EN == XVS_XHS_SEL_ENUM_XVS_XHS))
		value |= PIN_SENSOR_CFG_LVDS_VDHD;

	if (local_top_reg4.bit.SHUTTER_SEL == SHUTTER_ENUM_SHUTTER)
		value |= PIN_SENSOR_CFG_SHUTTER;

	if ((local_top_reg_sgpio0.bit.SGPIO_0 == GPIO_ID_EMUM_FUNC) && mclk_sensor)
		value |= PIN_SENSOR_CFG_MCLK;

	if ((local_top_reg_sgpio0.bit.SGPIO_5 == GPIO_ID_EMUM_FUNC) && mclk2_sensor[0])
		value |= PIN_SENSOR_CFG_MCLK2;

	if (local_top_reg4.bit.MES_SH0_SEL == MES_SEL_ENUM_1ST_PINMUX)
		value |= PIN_SENSOR_CFG_MES0;

	if (local_top_reg4.bit.MES_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX)
		value |= PIN_SENSOR_CFG_MES0_2ND;

	if (local_top_reg4.bit.MES_SH1_SEL == MES_SEL_ENUM_1ST_PINMUX)
		value |= PIN_SENSOR_CFG_MES1;

	if (local_top_reg4.bit.MES_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX)
		value |= PIN_SENSOR_CFG_MES1_2ND;

	if (local_top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_1ST_PINMUX)
		value |= PIN_SENSOR_CFG_MES2;

	if (local_top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX)
		value |= PIN_SENSOR_CFG_MES2_2ND;

	if (local_top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_1ST_PINMUX)
		value |= PIN_SENSOR_CFG_MES3;

	if (local_top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX)
		value |= PIN_SENSOR_CFG_MES3_2ND;

	if (local_top_reg4.bit.FLCTR == FLCTR_SEL_ENUM_FLCTR)
		value |= PIN_SENSOR_CFG_FLCTR;

	if (local_top_reg4.bit.STROBE_SEL == STROBE_ENUM_STROBE)
		value |= PIN_SENSOR_CFG_STROBE;

	if (local_top_reg6.bit.SP_CLK == SP_CLK_SEL_ENUM_SPCLK)
		value |= PIN_SENSOR_CFG_SPCLK;

	if (local_top_reg6.bit.SP_CLK == SP_CLK_SEL_ENUM_SP_2_CLK)
		value |= PIN_SENSOR_CFG_SPCLK_2ND;

	if (local_top_reg6.bit.SP2_CLK == SP2_CLK_SEL_ENUM_SP2CLK)
		value |= PIN_SENSOR_CFG_SP2CLK;

	if (local_top_reg6.bit.SP2_CLK == SP2_CLK_SEL_ENUM_SP2_2_CLK)
		value |= PIN_SENSOR_CFG_SP2CLK_2ND;

	if (local_top_reg4.bit.LOCKN == LOCKN_ENUM_LOCKN)
		value |= PIN_SENSOR_CFG_LOCKN;

	if (local_top_reg4.bit.LOCKN2 == LOCKN_ENUM_LOCKN)
		value |= PIN_SENSOR_CFG_LOCKN2;

	info->top_pinmux[PIN_FUNC_SENSOR].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR].pin_function = PIN_FUNC_SENSOR;

	/*Parsing SENSOR2*/
	value = mipi_lvds_sensor[1];

	if ((mipi_lvds_sensor[1] != 0) && (local_top_reg4.bit.XVS_XHS2_EN == XVS_XHS_SEL_ENUM_XVS_XHS))
		value |= PIN_SENSOR2_CFG_LVDS_VDHD;

	if ((local_top_reg_sgpio0.bit.SGPIO_5 == GPIO_ID_EMUM_FUNC) && mclk2_sensor[1])
		value |= PIN_SENSOR2_CFG_MCLK2;

	if (local_top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_1ST_PINMUX)
		value |= PIN_SENSOR2_CFG_MES2;

	if (local_top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX)
		value |= PIN_SENSOR2_CFG_MES2_2ND;

	if (local_top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_1ST_PINMUX)
		value |= PIN_SENSOR2_CFG_MES3;

	if (local_top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX)
		value |= PIN_SENSOR2_CFG_MES3_2ND;

	info->top_pinmux[PIN_FUNC_SENSOR2].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR2].pin_function = PIN_FUNC_SENSOR2;

	/*Parsing SENSOR3*/
	value = 0x0;

	switch (local_top_reg3.bit.SENSOR3) {
	case SENSOR_ENUM_8BITS:
		value |= PIN_SENSOR3_CFG_8BITS;
		break;
	case SENSOR_ENUM_10BITS:
		value |= PIN_SENSOR3_CFG_10BITS;
		break;
	case SENSOR_ENUM_12BITS:
		value |= PIN_SENSOR3_CFG_12BITS;
		break;
	case SENSOR_ENUM_16BITS:
		value |= PIN_SENSOR3_CFG_16BITS;
		break;
	default:
		break;
	}

	if (mipi_lvds_sensor[2] == PIN_SENSOR_CFG_MIPI)
		value |= PIN_SENSOR3_CFG_MIPI;

	if (mipi_lvds_sensor[2] == PIN_SENSOR_CFG_LVDS)
		value |= PIN_SENSOR3_CFG_LVDS;

	if ((mipi_lvds_sensor[2] != 0) && (local_top_reg4.bit.XVS_XHS3_SEL == XVS_XHS_1ST_PINMUX) && \
		(local_top_reg4.bit.XVS_XHS3_EN == XVS_XHS_SEL_ENUM_XVS_XHS))
		value |= PIN_SENSOR3_CFG_LVDS_VDHD;

	if ((mipi_lvds_sensor[2] != 0) && (local_top_reg4.bit.XVS_XHS3_SEL == XVS_XHS_2ND_PINMUX) && \
		(local_top_reg4.bit.XVS_XHS3_EN == XVS_XHS_SEL_ENUM_XVS_XHS))
		value |= PIN_SENSOR3_CFG_LVDS_VDHD_2ND;

	if ((local_top_reg_sgpio0.bit.SGPIO_5 == GPIO_ID_EMUM_FUNC) && mclk2_sensor[2])
		value |= PIN_SENSOR3_CFG_MCLK2;

	info->top_pinmux[PIN_FUNC_SENSOR3].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR3].pin_function = PIN_FUNC_SENSOR3;

	/*Parsing SENSOR4*/
	value = mipi_lvds_sensor[3];

	if ((mipi_lvds_sensor[3] != 0) && (local_top_reg4.bit.XVS_XHS4_EN == XVS_XHS_SEL_ENUM_XVS_XHS))
		value |= PIN_SENSOR4_CFG_LVDS_VDHD;

	if ((local_top_reg_sgpio0.bit.SGPIO_5 == GPIO_ID_EMUM_FUNC) && mclk2_sensor[3])
		value |= PIN_SENSOR4_CFG_MCLK2;

	info->top_pinmux[PIN_FUNC_SENSOR4].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR4].pin_function = PIN_FUNC_SENSOR4;

	/*Parsing SENSOR5*/
	value = mipi_lvds_sensor[4];

	if ((mipi_lvds_sensor[4] != 0) && (local_top_reg4.bit.XVS_XHS5_EN == XVS_XHS_SEL_ENUM_XVS_XHS))
		value |= PIN_SENSOR5_CFG_LVDS_VDHD;

	if ((local_top_reg_sgpio0.bit.SGPIO_5 == GPIO_ID_EMUM_FUNC) && mclk2_sensor[4])
		value |= PIN_SENSOR5_CFG_MCLK2;

	info->top_pinmux[PIN_FUNC_SENSOR5].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR5].pin_function = PIN_FUNC_SENSOR5;

	/*Parsing SENSOR6*/
	value = mipi_lvds_sensor[5];

	switch (local_top_reg3.bit.SENSOR6) {
	case SENSOR6_ENUM_8BITS:
		value |= PIN_SENSOR6_CFG_8BITS;
		break;
	case SENSOR6_ENUM_16BITS:
		if (ui_sie6_16_12)
			value |= PIN_SENSOR6_CFG_12BITS;
		else
			value |= PIN_SENSOR6_CFG_16BITS;
		break;
	default:
		break;
	}

	if ((mipi_lvds_sensor[5] != 0) && (local_top_reg4.bit.XVS_XHS6_EN == XVS_XHS_SEL_ENUM_XVS_XHS))
		value |= PIN_SENSOR6_CFG_LVDS_VDHD;

	if ((local_top_reg_sgpio0.bit.SGPIO_5 == GPIO_ID_EMUM_FUNC) && mclk2_sensor[5])
		value |= PIN_SENSOR6_CFG_MCLK2;

	info->top_pinmux[PIN_FUNC_SENSOR6].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR6].pin_function = PIN_FUNC_SENSOR6;

	/*Parsing SENSOR7*/
	value = mipi_lvds_sensor[6];

	if (local_top_reg3.bit.SENSOR7 == SENSOR7_ENUM_8BITS)
		value |= PIN_SENSOR7_CFG_8BITS;

	if ((mipi_lvds_sensor[6] != 0) && (local_top_reg4.bit.XVS_XHS7_EN == XVS_XHS_SEL_ENUM_XVS_XHS))
		value |= PIN_SENSOR7_CFG_LVDS_VDHD;

	if ((local_top_reg_sgpio0.bit.SGPIO_5 == GPIO_ID_EMUM_FUNC) && mclk2_sensor[6])
		value |= PIN_SENSOR7_CFG_MCLK2;

	info->top_pinmux[PIN_FUNC_SENSOR7].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR7].pin_function = PIN_FUNC_SENSOR7;

	/*Parsing SENSOR8*/
	value = mipi_lvds_sensor[7];

	if (local_top_reg3.bit.SENSOR8 == SENSOR8_ENUM_8BITS)
		value |= PIN_SENSOR8_CFG_8BITS;

	if ((mipi_lvds_sensor[7] != 0) && (local_top_reg4.bit.XVS_XHS8_EN == XVS_XHS_SEL_ENUM_XVS_XHS))
		value |= PIN_SENSOR8_CFG_LVDS_VDHD;

	if ((local_top_reg_sgpio0.bit.SGPIO_5 == GPIO_ID_EMUM_FUNC) && mclk2_sensor[7])
		value |= PIN_SENSOR8_CFG_MCLK2;

	info->top_pinmux[PIN_FUNC_SENSOR8].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR8].pin_function = PIN_FUNC_SENSOR8;

	/*Parsing MIPI_LVDS*/
	value = 0x0;

	if ((local_top_reg_hgpi0.bit.HSIGPI_18 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_19 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_CLK0;

	if ((local_top_reg_hgpi0.bit.HSIGPI_22 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_23 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_CLK1;

	if ((local_top_reg_hgpi0.bit.HSIGPI_26 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_27 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_CLK2;

	if ((local_top_reg_hgpi0.bit.HSIGPI_30 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_31 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_CLK3;

	if ((local_top_reg_hgpi0.bit.HSIGPI_2 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_3 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_CLK4;

	if ((local_top_reg_hgpi0.bit.HSIGPI_6 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_7 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_CLK5;

	if ((local_top_reg_hgpi0.bit.HSIGPI_10 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_11 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_CLK6;

	if ((local_top_reg_hgpi0.bit.HSIGPI_14 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_15 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_CLK7;

	if ((local_top_reg_hgpi0.bit.HSIGPI_16 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_17 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_DAT0;

	if ((local_top_reg_hgpi0.bit.HSIGPI_20 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_21 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_DAT1;

	if ((local_top_reg_hgpi0.bit.HSIGPI_24 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_25 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_DAT2;

	if ((local_top_reg_hgpi0.bit.HSIGPI_28 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_29 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_DAT3;

	if ((local_top_reg_hgpi0.bit.HSIGPI_0 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_1 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_DAT4;

	if ((local_top_reg_hgpi0.bit.HSIGPI_4 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_5 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_DAT5;

	if ((local_top_reg_hgpi0.bit.HSIGPI_8 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_8 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_DAT6;

	if ((local_top_reg_hgpi0.bit.HSIGPI_12 == GPIO_ID_EMUM_FUNC) && \
		(local_top_reg_hgpi0.bit.HSIGPI_13 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_LVDS_CFG_DAT7;

	info->top_pinmux[PIN_FUNC_MIPI_LVDS].config = value;
	info->top_pinmux[PIN_FUNC_MIPI_LVDS].pin_function = PIN_FUNC_MIPI_LVDS;

	/*Parsing I2C*/
	value = 0x0;

	if (local_top_reg10.bit.I2C == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH1;

	if (local_top_reg10.bit.I2C_2 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH1_2ND_PINMUX;

	if (local_top_reg10.bit.I2C2 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH2;

	if (local_top_reg10.bit.I2C2_2 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH2_2ND_PINMUX;

	if (local_top_reg10.bit.I2C3 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH3;

	if (local_top_reg10.bit.I2C3_2 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH3_2ND_PINMUX;

	if (local_top_reg10.bit.I2C3_3 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH3_3RD_PINMUX;

	if (local_top_reg10.bit.I2C4 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH4;

	if (local_top_reg10.bit.I2C4_2 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH4_2ND_PINMUX;

	if (local_top_reg10.bit.I2C4_3 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH4_3RD_PINMUX;

	if (local_top_reg10.bit.I2C4_4 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH4_4TH_PINMUX;

	if (local_top_reg10.bit.I2C5 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH5;

	if (local_top_reg10.bit.I2C5_2 == I2C_ENUM_I2C)
		value |= PIN_I2C_CFG_CH5_2ND_PINMUX;

	info->top_pinmux[PIN_FUNC_I2C].config = value;
	info->top_pinmux[PIN_FUNC_I2C].pin_function = PIN_FUNC_I2C;

	/*Parsing SIF*/
	value = 0x0;

	switch (local_top_reg5.bit.SIFCH0) {
	case SIFCH_ENUM_SIF:
		value |= PIN_SIF_CFG_CH0;
		break;
	case SIFCH_ENUM_SIF_2ND_PINMUX:
		value |= PIN_SIF_CFG_CH0_2ND_PINMUX;
		break;
	case SIFCH_ENUM_SIF_3RD_PINMUX:
		value |= PIN_SIF_CFG_CH0_3RD_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg5.bit.SIFCH1 == SIFCH_ENUM_SIF)
		value |= PIN_SIF_CFG_CH1;

	switch (local_top_reg5.bit.SIFCH2) {
	case SIFCH_ENUM_SIF:
		value |= PIN_SIF_CFG_CH2;
		break;
	case SIFCH_ENUM_SIF_2ND_PINMUX:
		value |= PIN_SIF_CFG_CH2_2ND_PINMUX;
		break;
	default:
		break;
	}

	switch (local_top_reg5.bit.SIFCH3) {
	case SIFCH_ENUM_SIF:
		value |= PIN_SIF_CFG_CH3;
		break;
	case SIFCH_ENUM_SIF_2ND_PINMUX:
		value |= PIN_SIF_CFG_CH3_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg5.bit.SIFCH4 == SIFCH_ENUM_SIF)
		value |= PIN_SIF_CFG_CH4;

	if (local_top_reg5.bit.SIFCH5 == SIFCH_ENUM_SIF)
		value |= PIN_SIF_CFG_CH5;

	if (local_top_reg5.bit.SIFCH6 == SIFCH_ENUM_SIF)
		value |= PIN_SIF_CFG_CH6;

	if (local_top_reg5.bit.SIFCH7 == SIFCH_ENUM_SIF)
		value |= PIN_SIF_CFG_CH7;

	info->top_pinmux[PIN_FUNC_SIF].config = value;
	info->top_pinmux[PIN_FUNC_SIF].pin_function = PIN_FUNC_SIF;

	/*Parsing UART*/
	value = 0x0;

	if (local_top_reg9.bit.UART == UART_ENUM_UART)
		value |= PIN_UART_CFG_CH1;

	switch (local_top_reg9.bit.UART2) {
	case UART2_ENUM_1ST_PINMUX:
		value |= PIN_UART_CFG_CH2;
		break;
	case UART2_ENUM_2ND_PINMUX:
		value |= PIN_UART_CFG_CH2 | PIN_UART_CFG_CH2_2ND;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.UART2_CTSRTS == UART_CTSRTS_PINMUX)
		value |= PIN_UART_CFG_CH2_CTSRTS;

	switch (local_top_reg9.bit.UART3) {
	case UART3_ENUM_1ST_PINMUX:
		value |= PIN_UART_CFG_CH3;
		break;
	case UART3_ENUM_2ND_PINMUX:
		value |= PIN_UART_CFG_CH3 | PIN_UART_CFG_CH3_2ND;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.UART3_CTSRTS == UART_CTSRTS_PINMUX)
		value |= PIN_UART_CFG_CH3_CTSRTS;

	if (local_top_reg9.bit.UART2_CTSRTS == UART_CTSRTS_PINMUX)
		value |= PIN_UART_CFG_CH2_CTSRTS;

	switch (local_top_reg9.bit.UART4) {
	case UART4_ENUM_1ST_PINMUX:
		value |= PIN_UART_CFG_CH4;
		break;
	case UART4_ENUM_2ND_PINMUX:
		value |= PIN_UART_CFG_CH4 | PIN_UART_CFG_CH4_2ND;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.UART4_CTSRTS == UART_CTSRTS_PINMUX)
		value |= PIN_UART_CFG_CH4_CTSRTS;

	info->top_pinmux[PIN_FUNC_UART].config = value;
	info->top_pinmux[PIN_FUNC_UART].pin_function = PIN_FUNC_UART;

	/*Parsing SPI*/
	value = 0x0;

	switch (local_top_reg5.bit.SPI) {
	case SPI_1ST_PINMUX:
		value |= PIN_SPI_CFG_CH1;
		break;
	case SPI_2ND_PINMUX:
		value |= PIN_SPI_CFG_CH1 | PIN_SPI_CFG_CH1_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg5.bit.SPI_DAT == SPI_DAT_ENUM_2Bit)
		value |= PIN_SPI_CFG_CH1_2BITS;

	switch (local_top_reg5.bit.SPI2) {
	case SPI2_1ST_PINMUX:
		value |= PIN_SPI_CFG_CH2;
		break;
	case SPI2_2ND_PINMUX:
		value |= PIN_SPI_CFG_CH2 | PIN_SPI_CFG_CH2_2ND_PINMUX;
		break;
	case SPI2_3RD_PINMUX:
		value |= PIN_SPI_CFG_CH2 | PIN_SPI_CFG_CH2_3RD_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg5.bit.SPI2_DAT == SPI_DAT_ENUM_2Bit)
		value |= PIN_SPI_CFG_CH2_2BITS;

	switch (local_top_reg5.bit.SPI3) {
	case SPI3_1ST_PINMUX:
		value |= PIN_SPI_CFG_CH3;
		break;
	case SPI3_2ND_PINMUX:
		value |= PIN_SPI_CFG_CH3 | PIN_SPI_CFG_CH3_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg5.bit.SPI3_DAT == SPI_DAT_ENUM_2Bit)
		value |= PIN_SPI_CFG_CH3_2BITS;

	switch (local_top_reg5.bit.SPI3_RDY) {
	case SPI_RDY_1ST_PINMUX:
		value |= PIN_SPI_CFG_CH3_RDY;
		break;
	case SPI_RDY_2ND_PINMUX:
		value |= PIN_SPI_CFG_CH3_RDY | PIN_SPI_CFG_CH3_RDY_2ND_PINMUX;
		break;
	default:
		break;
	}

	switch (local_top_reg5.bit.SPI4) {
	case SPI4_1ST_PINMUX:
		value |= PIN_SPI_CFG_CH4;
		break;
	case SPI4_2ND_PINMUX:
		value |= PIN_SPI_CFG_CH4 | PIN_SPI_CFG_CH4_2ND_PINMUX;
		break;
	case SPI4_3RD_PINMUX:
		value |= PIN_SPI_CFG_CH4 | PIN_SPI_CFG_CH4_3RD_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg5.bit.SPI4_DAT == SPI_DAT_ENUM_2Bit)
		value |= PIN_SPI_CFG_CH4_2BITS;

	info->top_pinmux[PIN_FUNC_SPI].config = value;
	info->top_pinmux[PIN_FUNC_SPI].pin_function = PIN_FUNC_SPI;

	/*Parsing REMOTE*/
	value = 0x0;

	if (local_top_reg6.bit.REMOTE == REMOTE_ENUM_REMOTE)
		value |= PIN_REMOTE_CFG_CH1;

	info->top_pinmux[PIN_FUNC_REMOTE].config = value;
	info->top_pinmux[PIN_FUNC_REMOTE].pin_function = PIN_FUNC_REMOTE;

	/*Parsing PWM*/
	value = 0x0;

	if (local_top_reg7.bit.PWM0 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM0;

	if (local_top_reg7.bit.PWM1 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM1;

	if (local_top_reg7.bit.PWM2 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM2;

	if (local_top_reg7.bit.PWM3 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM3;

	if (local_top_reg7.bit.PWM4 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM4;

	if (local_top_reg7.bit.PWM5 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM5;

	if (local_top_reg7.bit.PWM6 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM6;

	if (local_top_reg7.bit.PWM7 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM7;

	if (local_top_reg7.bit.PWM8 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM8;

	if (local_top_reg7.bit.PWM9 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM9;

	if (local_top_reg7.bit.PWM10 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM10;

	if (local_top_reg7.bit.PWM11 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM11;

	if (local_top_reg7.bit.PWM12 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM12;

	if (local_top_reg7.bit.PWM13 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM13;

	if (local_top_reg7.bit.PWM14 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM14;

	if (local_top_reg7.bit.PWM15 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM15;

	if (local_top_reg7.bit.PWM16 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM16;

	if (local_top_reg7.bit.PWM17 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM17;

	if (local_top_reg7.bit.PWM18 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM18;

	if (local_top_reg7.bit.PWM19 == PWM_ENUM_PWM)
		value |= PIN_PWM_CFG_PWM19;

	switch (local_top_reg7.bit.PI_CNT) {
	case PICNT_ENUM_PICNT:
		value |= PIN_PWM_CFG_CCNT;
		break;
	case PICNT_ENUM_PICNT2:
		value |= PIN_PWM_CFG_CCNT | PIN_PWM_CFG_CCNT_2ND;
		break;
	default:
		break;
	}

	switch (local_top_reg7.bit.PI_CNT2) {
	case PICNT_ENUM_PICNT:
		value |= PIN_PWM_CFG_CCNT2;
		break;
	case PICNT_ENUM_PICNT2:
		value |= PIN_PWM_CFG_CCNT2 | PIN_PWM_CFG_CCNT2_2ND;
		break;
	default:
		break;
	}

	switch (local_top_reg7.bit.PI_CNT3) {
	case PICNT_ENUM_PICNT:
		value |= PIN_PWM_CFG_CCNT3;
		break;
	case PICNT_ENUM_PICNT2:
		value |= PIN_PWM_CFG_CCNT3 | PIN_PWM_CFG_CCNT3_2ND;
		break;
	default:
		break;
	}

	switch (local_top_reg7.bit.PI_CNT4) {
	case PICNT_ENUM_PICNT:
		value |= PIN_PWM_CFG_CCNT4;
		break;
	case PICNT_ENUM_PICNT2:
		value |= PIN_PWM_CFG_CCNT4 | PIN_PWM_CFG_CCNT2_2ND;
		break;
	default:
		break;
	}

	info->top_pinmux[PIN_FUNC_PWM].config = value;
	info->top_pinmux[PIN_FUNC_PWM].pin_function = PIN_FUNC_PWM;

	/*Parsing AUDIO*/
	value = 0x0;

	if (local_top_reg6.bit.AUDIO == AUDIO_ENUM_I2S)
		value |= PIN_AUDIO_CFG_I2S;

	if (local_top_reg6.bit.AUDIO_MCLK == AUDIO_MCLK_MCLK)
		value |= PIN_AUDIO_CFG_MCLK;

	info->top_pinmux[PIN_FUNC_AUDIO].config = value;
	info->top_pinmux[PIN_FUNC_AUDIO].pin_function = PIN_FUNC_AUDIO;

	/*Parsing LCD*/
	info->top_pinmux[PIN_FUNC_LCD].config = disp_pinmux_config[PINMUX_FUNC_ID_LCD];
	info->top_pinmux[PIN_FUNC_LCD].pin_function = PIN_FUNC_LCD;

	/*Parsing LCD2*/
	info->top_pinmux[PIN_FUNC_LCD2].config = disp_pinmux_config[PINMUX_FUNC_ID_LCD2];
	info->top_pinmux[PIN_FUNC_LCD2].pin_function = PIN_FUNC_LCD2;

	/*Parsing TV*/
	info->top_pinmux[PIN_FUNC_TV].config = disp_pinmux_config[PINMUX_FUNC_ID_TV];
	info->top_pinmux[PIN_FUNC_TV].pin_function = PIN_FUNC_TV;

	/*Parsing HDMI*/
	info->top_pinmux[PIN_FUNC_HDMI].config = disp_pinmux_config[PINMUX_FUNC_ID_HDMI];
	info->top_pinmux[PIN_FUNC_HDMI].pin_function = PIN_FUNC_HDMI;

	/*Parsing ETH*/
	value = 0x0;
	switch (local_top_reg6.bit.ETH) {
	case ETH_ID_ENUM_MII:
		value |= PIN_ETH_CFG_MII;
		break;
	case ETH_ID_ENUM_RMII:
		value |= PIN_ETH_CFG_RMII;
		break;
	case ETH_ID_ENUM_GMII:
		value |= PIN_ETH_CFG_GMII;
		break;
	case ETH_ID_ENUM_RGMII:
		value |= PIN_ETH_CFG_RGMII;
		break;
	case ETH_ID_ENUM_REVMII_10_100:
		value |= PIN_ETH_CFG_REVMII_10_100;
		break;
	case ETH_ID_ENUM_REVMII_10_1000:
		value |= PIN_ETH_CFG_REVMII_10_1000;
		break;
	default:
		break;
	}

	info->top_pinmux[PIN_FUNC_ETH].config = value;
	info->top_pinmux[PIN_FUNC_ETH].pin_function = PIN_FUNC_ETH;

	/*Parsing CANBUS*/
	value = 0x0;
	if (local_top_reg6.bit.CANBUS == CANBUS_ENUM_CANBUS)
		value = PIN_CANBUS_CFG_CANBUS;

	info->top_pinmux[PIN_FUNC_CANBUS].config = value;
	info->top_pinmux[PIN_FUNC_CANBUS].pin_function = PIN_FUNC_CANBUS;

	up(&top_sem);
}

/**
	Configure pinmux controller

	Configure pinmux controller by upper layer

	@param[in] info	nvt_pinctrl_info
	@return void
*/

ER pinmux_init(struct nvt_pinctrl_info *info)
{
	uint32_t i;
	int err;

	ui_sie6_16_12 = 0;
	/*Assume all PINMUX is GPIO*/
	top_reg1.reg = 0;
	top_reg2.reg = 0;
	top_reg3.reg = 0;
	top_reg4.reg = 0;
	top_reg5.reg = 0;
	top_reg6.reg = 0;
	top_reg7.reg = 0;
	top_reg8.reg = 0x02;
	top_reg9.reg = 0;
	top_reg10.reg = 0;
	top_reg_cgpio0.reg = 0xFFFFFFFF;
	top_reg_cgpio1.reg = 0xFFFFFFFF;
	top_reg_pgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio1.reg = 0xFFFFFFFF;
	top_reg_sgpio0.reg = 0xFFFFFFFF;
	top_reg_lgpio0.reg = 0xFFFFFFFF;
	top_reg_lgpio1.reg = 0xFFFFFFFF;
	top_reg_hgpi0.reg = 0xFFFFFFFF;
	top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	if (top_reg0.bit.EJTAG_SEL)
		top_reg_dgpio0.reg = 0x0000183F;
	else
		top_reg_dgpio0.reg = 0x00001FFF;

	/*Enter critical section*/
	down(&top_sem);

	/*Reset config value*/
	for (i = 0; i < sizeof(mipi_lvds_sensor)/sizeof(mipi_lvds_sensor[0]); i++) {
		mipi_lvds_sensor[i] = 0;
		mclk2_sensor[i] = 0;
	}

	mclk_sensor = 0;

	for (i = 0; i < PIN_FUNC_MAX; i++) {
		if (info->top_pinmux[i].pin_function != i) {
			pr_err("top_config[%d].pinFunction context error\n", i);
			/*Leave critical section*/
			up(&top_sem);
			return E_CTX;
		}

		err = pinmux_config_hdl[i](info->top_pinmux[i].config);
		if (err != E_OK) {
			pr_err("top_config[%d].config config error\n", i);
			/*Leave critical section*/
			up(&top_sem);
			return err;
		}
	}

	/*keep ram/rom mapping*/
	top_reg12.reg = TOP_GETREG(info, TOP_REG12_OFS);
	top_reg12.bit.RAM_SRC_SELECT = top_reg12.bit.RAM_SRC_SELECT;
	top_reg12.bit.ROM_SRC_SELECT = top_reg12.bit.ROM_SRC_SELECT;

	TOP_SETREG(info, TOP_REG1_OFS, top_reg1.reg);
	TOP_SETREG(info, TOP_REG2_OFS, top_reg2.reg);
	TOP_SETREG(info, TOP_REG3_OFS, top_reg3.reg);
	TOP_SETREG(info, TOP_REG4_OFS, top_reg4.reg);
	TOP_SETREG(info, TOP_REG5_OFS, top_reg5.reg);
	TOP_SETREG(info, TOP_REG6_OFS, top_reg6.reg);
	TOP_SETREG(info, TOP_REG7_OFS, top_reg7.reg);
	TOP_SETREG(info, TOP_REG8_OFS, top_reg8.reg);
	TOP_SETREG(info, TOP_REG9_OFS, top_reg9.reg);
	TOP_SETREG(info, TOP_REG10_OFS,top_reg10.reg);

	TOP_SETREG(info, TOP_REGCGPIO0_OFS, top_reg_cgpio0.reg);
	TOP_SETREG(info, TOP_REGCGPIO1_OFS, top_reg_cgpio1.reg);
	TOP_SETREG(info, TOP_REGPGPIO0_OFS, top_reg_pgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO1_OFS, top_reg_pgpio1.reg);
	TOP_SETREG(info, TOP_REGSGPIO0_OFS, top_reg_sgpio0.reg);
	TOP_SETREG(info, TOP_REGLGPIO0_OFS, top_reg_lgpio0.reg);
	TOP_SETREG(info, TOP_REGLGPIO1_OFS, top_reg_lgpio1.reg);
	TOP_SETREG(info, TOP_REGDGPIO0_OFS, top_reg_dgpio0.reg);
	TOP_SETREG(info, TOP_REGHGPI0_OFS,  top_reg_hgpi0.reg);

	/*Leave critical section*/
	up(&top_sem);

	return E_OK;
}


/*-----------------------------------------------------------------------------*/
/* PINMUX Interface Functions							*/
/*-----------------------------------------------------------------------------*/
static int pinmux_config_sdio(uint32_t config)
{
	if (config == PIN_SDIO_CFG_NONE) {
	} else {
		if (config & PIN_SDIO_CFG_2ND_PINMUX) {
			pr_err("SDIO does NOT have 2nd pinmux pad\r\n");
			return E_PAR;
		}

		if (config & PIN_SDIO_CFG_8BITS) {
			pr_err("SDIO does NOT support 8 bits\r\n");
			return E_PAR;
		}

		top_reg1.bit.SDIO_EXIST = SDIO_EXIST_EN;    // SDIO exist

		top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
	}

	return E_OK;
}

static int pinmux_config_sdio2(uint32_t config)
{
	if (config == PIN_SDIO_CFG_NONE) {
	} else {
		if (config & PIN_SDIO_CFG_8BITS) {
			pr_err("SDIO2 does NOT support 8 bits\r\n");
			return E_PAR;
		}

		if (config & PIN_SDIO_CFG_2ND_PINMUX) {
			pr_err("SDIO2 does NOT have 2nd pinmux pad\r\n");
			return E_PAR;
		}

		top_reg1.bit.SDIO2_EXIST = SDIO2_EXIST_EN;  // SD CLK exist

		top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_24 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_25 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_26 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_27 = GPIO_ID_EMUM_FUNC;
	}

	return E_OK;
}

static int pinmux_config_sdio3(uint32_t config)
{
	if (config == PIN_SDIO_CFG_NONE) {
	} else {
		if (config & PIN_SDIO_CFG_2ND_PINMUX) {
			if (config & PIN_SDIO_CFG_8BITS) {
				top_reg1.bit.SDIO3_BUS_WIDTH = SDIO3_BUS_WIDTH_8BITS;

				top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
			}
			top_reg1.bit.SDIO3_MUX_SEL = SDIO3_MUX_2ND;

			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
		} else {
			if (config & PIN_SDIO_CFG_8BITS) {
				pr_err("SDIO3 does NOT support 8 bits when PIN_SDIO_CFG_1ST_PINMUX \r\n");
				return E_PAR;
			}

			top_reg_cgpio0.bit.CGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_29 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_30 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_31 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio1.bit.CGPIO_32 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio1.bit.CGPIO_33 = GPIO_ID_EMUM_FUNC;
		}

		top_reg1.bit.SDIO3_EXIST = SDIO3_EXIST_EN;  // SD3 pinmux enable

	}

	return E_OK;
}

static int pinmux_config_nand(uint32_t config)
{
	if (config == PIN_NAND_CFG_NONE) {
	} else if (config & (PIN_NAND_CFG_1CS|PIN_NAND_CFG_2CS|PIN_NAND_CFG_SPI_NAND|PIN_NAND_CFG_SPI_NOR)) {
		if ((top_reg1.bit.FSPI_EXIST == FSPI_EXIST_EN)) {
			pr_err("NAND conflict with FSPI\r\n");
			return E_OBJ;
		}

		if ((top_reg1.bit.SDIO3_MUX_SEL == SDIO3_MUX_2ND) && (top_reg1.bit.SDIO3_EXIST == SDIO3_EXIST_EN)) {
			pr_err("NAND conflict with SDIO3_2\r\n");
			return E_OBJ;
		}

		if ((top_reg9.bit.UART4 == UART4_ENUM_2ND_PINMUX)) {
			pr_err("NAND conflict with UART4_2\r\n");
			return E_OBJ;
		}

		if ((top_reg10.bit.I2C_2 == I2C_ENUM_I2C)) {
			pr_err("NAND conflict with I2C_2\r\n");
			return E_OBJ;
		}

		if ((top_reg10.bit.I2C5_2 == I2C_ENUM_I2C)) {
			pr_err("NAND conflict with I2C5_2\r\n");
			return E_OBJ;
		}

		if ((top_reg11.bit.TRACE_EN == TRACE_ENUM_TRACE)) {
			pr_err("NAND conflict with TRACE\r\n");
			return E_OBJ;
		}


		if (config & PIN_NAND_CFG_2CS) {
			top_reg1.bit.NAND_CS_NUM = NAND_CS_NUM_2CS;
			top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
		}



		if (config & PIN_NAND_CFG_SPI_NAND) {
			if(config & PIN_NAND_CFG_SPI_NOR) {
				pr_err("conflict with SPI NOR\r\n");
				return E_OBJ;
			} else {
				top_reg1.bit.NAND_EXIST = NAND_EXIST_DIS;
				top_reg1.bit.FSPI_EXIST = FSPI_EXIST_EN;
				top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
			}
		} else if (config & PIN_NAND_CFG_SPI_NOR) {
			if(config & PIN_NAND_CFG_SPI_NAND) {
				pr_err("conflict with SPI NAND\r\n");
				return E_OBJ;
			} else {
				top_reg1.bit.NAND_EXIST = NAND_EXIST_DIS;
				top_reg1.bit.FSPI_EXIST = FSPI_EXIST_EN;
				top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
			}
		} else {
			top_reg1.bit.NAND_EXIST = NAND_EXIST_EN;
			top_reg1.bit.FSPI_EXIST = FSPI_EXIST_DIS;
			top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
		}

		top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
		top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC;
	} else {
		pr_err("invalid config: 0x%x\r\n", config);
		return E_PAR;
	}
	return E_OK;
}

static int pinmux_config_sensor(uint32_t config)
{
	uint32_t tmp;

	if (config == PIN_SENSOR_CFG_NONE) {
	} else {
		tmp = config & (PIN_SENSOR_CFG_8BITS|PIN_SENSOR_CFG_10BITS|PIN_SENSOR_CFG_12BITS|PIN_SENSOR_CFG_MIPI|PIN_SENSOR_CFG_LVDS);
		switch (tmp) {
		case PIN_SENSOR_CFG_8BITS:
			top_reg3.bit.SENSOR = SENSOR_ENUM_8BITS;
			top_reg_hgpi0.bit.HSIGPI_18 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_19 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_20 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_21 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_24 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_25 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_28 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_29 = GPIO_ID_EMUM_GPIO;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC; //PXCLK
			break;

		case PIN_SENSOR_CFG_10BITS:
			top_reg3.bit.SENSOR = SENSOR_ENUM_10BITS;
			top_reg_hgpi0.bit.HSIGPI_16 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_17 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_18 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_19 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_20 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_21 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_24 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_25 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_28 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_29 = GPIO_ID_EMUM_GPIO;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC; //PXCLK
			break;

		case PIN_SENSOR_CFG_12BITS:
			top_reg3.bit.SENSOR = SENSOR_ENUM_12BITS;
			top_reg_hgpi0.bit.HSIGPI_16 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_17 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_18 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_19 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_20 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_21 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_22 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_23 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_24 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_25 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_28 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_29 = GPIO_ID_EMUM_GPIO;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC; //PXCLK
			break;

		case PIN_SENSOR_CFG_MIPI:
			mipi_lvds_sensor[0] = PIN_SENSOR_CFG_MIPI;
			break;

		case PIN_SENSOR_CFG_LVDS:
			mipi_lvds_sensor[0] = PIN_SENSOR_CFG_LVDS;
			break;

		default:
			pr_err("sensor no bus width assigned: 0x%x\r\n", config);
			return E_PAR;
		}

		// Don't need check I2C2_2, because VD/HD is checked first
		// When sensor is LVDS/MIPI and LVDS_VDHD is selected
		if ((mipi_lvds_sensor[0] != 0) && (config & PIN_SENSOR_CFG_LVDS_VDHD)) {
			// Assing LVDS VD/HD when project layer select this config
			//top_reg3.bit.VD_HD_SEL = VD_HD_SEL_ENUM_LVDS_VDHD;
			top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC; //XVS
			top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC; //XHS
			top_reg4.bit.XVS_XHS_EN = XVS_XHS_SEL_ENUM_XVS_XHS;
		} else if (mipi_lvds_sensor[0] == 0) {
			// When sensor is parallel sensor interface
			// Force SN_VD/SN_HD to SIE when parallel sensor interface is selected
			//top_reg3.bit.VD_HD_SEL = VD_HD_SEL_ENUM_SIE_VDHD;
		}

		if (config & PIN_SENSOR_CFG_SHUTTER) {
			if (top_reg10.bit.I2C3 == I2C_2_ENUM_I2C) {
				pr_err("SHUTTER conflict with I2C3\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH7 == SIFCH_ENUM_SIF) {
				pr_err("SHUTTER conflict with SIFCH7\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SPI4 == SPI_MUX_1ST) {
				pr_err("SHUTTER conflict with SPI4_1\r\n");
				return E_OBJ;
			}

			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg4.bit.SHUTTER_SEL = SHUTTER_ENUM_SHUTTER;
		}

		if (config & PIN_SENSOR_CFG_MCLK) {
			top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
			mclk_sensor = 1;
		}

		if (config & PIN_SENSOR_CFG_MCLK2) {
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			mclk2_sensor[0] = 1;
		}

		if (config & (PIN_SENSOR_CFG_MES0|PIN_SENSOR_CFG_MES0_2ND)) {
			if (config & PIN_SENSOR_CFG_MES0_2ND) {
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES_SH0_SEL = MES_SEL_ENUM_2ND_PINMUX;
			} else {
				top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES_SH0_SEL = MES_SEL_ENUM_1ST_PINMUX;
			}
		}

		if (config & (PIN_SENSOR_CFG_MES1|PIN_SENSOR_CFG_MES1_2ND)) {
			if (config & PIN_SENSOR_CFG_MES1_2ND) {
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES_SH1_SEL = MES_SEL_ENUM_2ND_PINMUX;
			} else {
				top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES_SH1_SEL = MES_SEL_ENUM_1ST_PINMUX;
			}
		}

		if (config & (PIN_SENSOR_CFG_MES2|PIN_SENSOR_CFG_MES2_2ND)) {
			if (config & PIN_SENSOR_CFG_MES2_2ND) {
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES2_SH0_SEL = MES_SEL_ENUM_2ND_PINMUX;
			} else {
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES2_SH0_SEL = MES_SEL_ENUM_1ST_PINMUX;
			}
		}

		if (config & (PIN_SENSOR_CFG_MES3|PIN_SENSOR_CFG_MES3_2ND)) {
			if (config & PIN_SENSOR_CFG_MES3_2ND) {
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES2_SH1_SEL = MES_SEL_ENUM_2ND_PINMUX;
			} else {
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES2_SH1_SEL = MES_SEL_ENUM_1ST_PINMUX;
			}
		}

		if (config & PIN_SENSOR_CFG_FLCTR) {
			if (top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) {
				pr_err("FLCTR conflict with SEOSOR6 16\r\n");
				return E_OBJ;
			}

			if (top_reg3.bit.SENSOR7 == SENSOR7_ENUM_8BITS) {
				pr_err("FLCTR conflict with SEOSOR7\r\n");
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg4.bit.FLCTR = FLCTR_SEL_ENUM_FLCTR;
		}

		if (config & PIN_SENSOR_CFG_STROBE) {
			if (top_reg10.bit.I2C3 == I2C_2_ENUM_I2C) {
				pr_err("STROBE conflict with I2C3\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH6 == SIFCH_ENUM_SIF) {
				pr_err("STROBE conflict with SIFCH6\r\n");
				return E_OBJ;
			}

			if ((top_reg5.bit.SPI4 == SPI4_1ST_PINMUX) && (top_reg5.bit.SPI4_DAT == SPI_DAT_ENUM_2Bit)) {
				pr_err("STROBE conflict with SPI4_1\r\n");
				return E_OBJ;
			}

			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg4.bit.STROBE_SEL = STROBE_ENUM_STROBE;
		}

		if (config & (PIN_SENSOR_CFG_SPCLK|PIN_SENSOR_CFG_SPCLK_2ND|PIN_SENSOR_CFG_SPCLK_3RD)) {
			if (config & PIN_SENSOR_CFG_SPCLK_2ND) {
				if (top_reg5.bit.SPI2 == SPI2_1ST_PINMUX) {
					pr_err("SP_CLK_2 conflict with SPI2_1\r\n");
					return E_OBJ;
				}

				if (top_reg9.bit.UART3 == UART3_ENUM_1ST_PINMUX) {
					pr_err("SP_CLK_2 conflict with UART3_1\r\n");
					return E_OBJ;
				}

				if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF) {
					pr_err("SP_CLK_2 conflict with SIF2\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.AUDIO_MCLK == AUDIO_MCLK_MCLK) {
					pr_err("SP_CLK_2 conflict with AUDIO MCLK\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.LOCKN == LOCKN_ENUM_LOCKN) {
					pr_err("SP_CLK_2 conflict with LOCKN\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
				top_reg6.bit.SP_CLK = SP_CLK_SEL_ENUM_SP_2_CLK;
			} else if (config & PIN_SENSOR_CFG_SPCLK_3RD){
				top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
				top_reg6.bit.SP_CLK = SP_CLK_SEL_ENUM_SP_3_CLK;
			} else {
				//check PICNT4_1
				if (top_reg7.bit.PI_CNT4 == PICNT_ENUM_PICNT) {
					pr_err("SP_CLK conflict with PICNT4_1\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
				top_reg6.bit.SP_CLK = SP_CLK_SEL_ENUM_SPCLK;
			}
		}

		if (config & (PIN_SENSOR_CFG_SP2CLK|PIN_SENSOR_CFG_SP2CLK_2ND)) {
			if (config & PIN_SENSOR_CFG_SP2CLK_2ND) {
				if (top_reg5.bit.SPI2 == SPI2_1ST_PINMUX) {
					pr_err("SP_CLK2_2 conflict with SPI2_1\r\n");
					return E_OBJ;
				}

				if (top_reg9.bit.UART3 == UART3_ENUM_1ST_PINMUX) {
					pr_err("SP_CLK2_2 conflict with UART3_1\r\n");
					return E_OBJ;
				}

				if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF) {
					pr_err("SP_CLK2_2 conflict with SIF3\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.CANBUS== CANBUS_ENUM_CANBUS) {
					pr_err("SP_CLK2_2 conflict with CANBUS\r\n");
					return E_OBJ;
				}

				if (top_reg2.bit.TV_TEST_CLK == TV_TEST_CLK_TV_CLK) {
					pr_err("SP_CLK2_2 conflict with TV_TEST_CLK\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.LOCKN2 == LOCKN_ENUM_LOCKN) {
					pr_err("SP_CLK2_2 conflict with LOCKN2\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
				top_reg6.bit.SP2_CLK = SP2_CLK_SEL_ENUM_SP2_2_CLK;
			} else {
				if ((top_reg7.bit.PWM0 == PWM_ENUM_PWM) && (top_reg7.bit.PWM0_SEL == PWM_SEL_ENUM_2ND_PINMUX)) {
					pr_err("SP_CLK2_1 conflict with PWM0_2\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg6.bit.SP2_CLK = SP2_CLK_SEL_ENUM_SP2CLK;
			}
		}

		if (config & (PIN_SENSOR_CFG_LOCKN)) {
			if (top_reg5.bit.SPI2 == SPI2_1ST_PINMUX) {
				pr_err("LOCKN conflict with SPI2_1\r\n");
				return E_OBJ;
			}

			if (top_reg9.bit.UART3 == UART3_ENUM_1ST_PINMUX) {
				pr_err("LOCKN conflict with UART3_1\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF) {
				pr_err("LOCKN conflict with SIF2\r\n");
				return E_OBJ;
			}

			if (top_reg6.bit.AUDIO_MCLK == AUDIO_MCLK_MCLK) {
				pr_err("LOCKN conflict with AUDIO MCLK\r\n");
				return E_OBJ;
			}

			if (top_reg6.bit.SP_CLK == SP_CLK_SEL_ENUM_SP_2_CLK) {
				pr_err("LOCKN conflict with SP_CLK_2\r\n");
				return E_OBJ;
			}

			top_reg4.bit.LOCKN = LOCKN_ENUM_LOCKN;
		}

		if (config & (PIN_SENSOR_CFG_LOCKN2)) {
			if (top_reg5.bit.SPI2 == SPI2_1ST_PINMUX) {
				pr_err("LOCKN2 conflict with SPI2_1\r\n");
				return E_OBJ;
			}

			if (top_reg9.bit.UART3 == UART3_ENUM_1ST_PINMUX) {
				pr_err("LOCKN2 conflict with UART3_1\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF) {
				pr_err("LOCKN2 conflict with SIF3\r\n");
				return E_OBJ;
			}

			if (top_reg6.bit.CANBUS== CANBUS_ENUM_CANBUS) {
				pr_err("LOCKN2 conflict with CANBUS\r\n");
				return E_OBJ;
			}

			if (top_reg2.bit.TV_TEST_CLK == TV_TEST_CLK_TV_CLK) {
				pr_err("LOCKN2 conflict with TV_TEST_CLK\r\n");
				return E_OBJ;
			}

			if (top_reg6.bit.SP2_CLK == SP_CLK_SEL_ENUM_SP_2_CLK) {
				pr_err("LOCKN2 conflict with SP_CLK2_2\r\n");
				return E_OBJ;
			}

			top_reg4.bit.LOCKN2 = LOCKN_ENUM_LOCKN;
		}

	}

	return E_OK;
}

static int pinmux_config_sensor2(uint32_t config)
{
	uint32_t tmp;

	if (config == PIN_SENSOR2_CFG_NONE) {
	} else {
		tmp = config & (PIN_SENSOR2_CFG_MIPI|PIN_SENSOR2_CFG_LVDS);
		switch (tmp) {
		case PIN_SENSOR2_CFG_MIPI:
			mipi_lvds_sensor[1] = PIN_SENSOR2_CFG_MIPI;
			break;

		case PIN_SENSOR2_CFG_LVDS:
			mipi_lvds_sensor[1] = PIN_SENSOR2_CFG_LVDS;
			break;

		default:
			pr_err("sensor2 no bus width assigned: 0x%x\r\n", config);
			break;
		}

		if ((mipi_lvds_sensor[1] != 0) && (config & PIN_SENSOR2_CFG_LVDS_VDHD)) {
			// Assing LVDS VD/HD when project layer select this config
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC; //XVS
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC; //XHS
			top_reg4.bit.XVS_XHS2_EN = XVS_XHS_SEL_ENUM_XVS_XHS;
		}

		if (config & PIN_SENSOR2_CFG_MCLK2) {
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			mclk2_sensor[1] = 1;
		}


		if (config & (PIN_SENSOR2_CFG_MES2|PIN_SENSOR2_CFG_MES2_2ND)) {
			if (config & PIN_SENSOR2_CFG_MES2_2ND) {
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES2_SH0_SEL = MES_SEL_ENUM_2ND_PINMUX;
			} else {
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES2_SH0_SEL = MES_SEL_ENUM_1ST_PINMUX;
			}
		}

		if (config & (PIN_SENSOR2_CFG_MES3|PIN_SENSOR2_CFG_MES3_2ND)) {
			if (config & PIN_SENSOR2_CFG_MES3_2ND) {
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES2_SH1_SEL = MES_SEL_ENUM_2ND_PINMUX;
			} else {
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.MES2_SH1_SEL = MES_SEL_ENUM_1ST_PINMUX;
			}
		}

	}

	return E_OK;
}

/*
    Sensor3 pinmux config

    Sensor3 pinmux config

    @param[in] config

    @return
        - @b E_OK: success
        - @b E_PAR: parameter config not correct
*/
static int pinmux_config_sensor3(uint32_t config)
{
	uint32_t tmp;

	if (config == PIN_SENSOR3_CFG_NONE) {
	} else if (config & (PIN_SENSOR3_CFG_8BITS|PIN_SENSOR3_CFG_10BITS|PIN_SENSOR3_CFG_12BITS|PIN_SENSOR3_CFG_16BITS|PIN_SENSOR3_CFG_MIPI|PIN_SENSOR3_CFG_LVDS)) {
		tmp = config & (PIN_SENSOR3_CFG_8BITS|PIN_SENSOR3_CFG_10BITS|PIN_SENSOR3_CFG_12BITS|PIN_SENSOR3_CFG_16BITS|PIN_SENSOR3_CFG_MIPI|PIN_SENSOR3_CFG_LVDS);
		switch (tmp) {
		case PIN_SENSOR3_CFG_8BITS:
			top_reg3.bit.SENSOR3 = SENSOR_ENUM_8BITS;
			top_reg_hgpi0.bit.HSIGPI_0 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_4 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_5 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_8 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_9 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_10 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_11 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_12 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_13 = GPIO_ID_EMUM_GPIO;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC; //PXCLK
			break;

		case PIN_SENSOR3_CFG_10BITS:
			top_reg3.bit.SENSOR3 = SENSOR_ENUM_10BITS;
			top_reg_hgpi0.bit.HSIGPI_0 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_1 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_4 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_5 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_8 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_9 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_10 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_11 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_12 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_13 = GPIO_ID_EMUM_GPIO;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC; //PXCLK
			break;

		case PIN_SENSOR3_CFG_12BITS:
			top_reg3.bit.SENSOR3 = SENSOR_ENUM_12BITS;
			top_reg_hgpi0.bit.HSIGPI_0 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_1 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_2 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_3 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_4 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_5 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_8 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_9 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_10 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_11 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_12 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_13 = GPIO_ID_EMUM_GPIO;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC; //PXCLK
			break;

		case PIN_SENSOR3_CFG_16BITS:
			top_reg3.bit.SENSOR3 = SENSOR_ENUM_16BITS;
			top_reg_hgpi0.bit.HSIGPI_0 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_4 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_5 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_8 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_9 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_10 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_11 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_12 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_13 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_18 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_19 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_20 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_21 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_24 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_25 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_28 = GPIO_ID_EMUM_GPIO;
			top_reg_hgpi0.bit.HSIGPI_29 = GPIO_ID_EMUM_GPIO;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC; //PXCLK
			break;

		case PIN_SENSOR3_CFG_MIPI:
			mipi_lvds_sensor[2] = PIN_SENSOR_CFG_MIPI;
			break;

		case PIN_SENSOR3_CFG_LVDS:
			mipi_lvds_sensor[2] = PIN_SENSOR_CFG_LVDS;
			break;

		default:
			pr_err("sensor3 no bus width assigned: 0x%x\r\n", config);
			break;
		}

		if ((mipi_lvds_sensor[2] != 0) && (config & PIN_SENSOR3_CFG_LVDS_VDHD)) {
			// Assing LVDS VD/HD when project layer select this config
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC; //XVS
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC; //XHS
			top_reg4.bit.XVS_XHS3_EN = XVS_XHS_SEL_ENUM_XVS_XHS;
			top_reg4.bit.XVS_XHS3_SEL = XVS_XHS_1ST_PINMUX;
		}

		if ((mipi_lvds_sensor[2] != 0) && (config & PIN_SENSOR3_CFG_LVDS_VDHD_2ND)) {
			// Assing LVDS VD/HD when project layer select this config
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC; //XVS
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC; //XHS
			top_reg4.bit.XVS_XHS3_EN = XVS_XHS_SEL_ENUM_XVS_XHS;
			top_reg4.bit.XVS_XHS3_SEL = XVS_XHS_2ND_PINMUX;
		}


		if (config & PIN_SENSOR3_CFG_MCLK2) {
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			mclk2_sensor[2] = 1;
		}

		if (config & (PIN_SENSOR3_CFG_8BITS | PIN_SENSOR3_CFG_16BITS)) {
			if (config & PIN_SENSOR3_CFG_NOFILED) {
				top_reg_hgpi0.bit.HSIGPI_0 = GPIO_ID_EMUM_FUNC;
			}
		}
	}

	return E_OK;
}

static int pinmux_config_sensor4(uint32_t config)
{
	uint32_t tmp;

	if (config == PIN_SENSOR4_CFG_NONE) {
	} else {
		tmp = config & (PIN_SENSOR4_CFG_MIPI|PIN_SENSOR4_CFG_LVDS);
		switch (tmp) {
		case PIN_SENSOR4_CFG_MIPI:
			mipi_lvds_sensor[3] = PIN_SENSOR4_CFG_MIPI;
			break;

		case PIN_SENSOR4_CFG_LVDS:
			mipi_lvds_sensor[3] = PIN_SENSOR4_CFG_LVDS;
			break;

		default:
			pr_err("sensor4 no bus width assigned: 0x%x\r\n", config);
			break;
		}

		if ((mipi_lvds_sensor[3] != 0) && (config & PIN_SENSOR4_CFG_LVDS_VDHD)) {
			// Assing LVDS VD/HD when project layer select this config
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC; //XVS
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC; //XHS
			top_reg4.bit.XVS_XHS4_EN = XVS_XHS_SEL_ENUM_XVS_XHS;
		}

		if (config & PIN_SENSOR4_CFG_MCLK2) {
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			mclk2_sensor[3] = 1;
		}

	}
	return E_OK;
}

static int pinmux_config_sensor5(uint32_t config)
{
	uint32_t tmp;

	if (config == PIN_SENSOR5_CFG_NONE) {
	} else {
		tmp = config & (PIN_SENSOR5_CFG_MIPI|PIN_SENSOR5_CFG_LVDS);
		switch (tmp) {
		case PIN_SENSOR5_CFG_MIPI:
			mipi_lvds_sensor[4] = PIN_SENSOR5_CFG_MIPI;
			break;

		case PIN_SENSOR5_CFG_LVDS:
			mipi_lvds_sensor[4] = PIN_SENSOR5_CFG_LVDS;
			break;

		default:
			pr_err("sensor5 no bus width assigned: 0x%x\r\n", config);
			break;
		}

		if ((mipi_lvds_sensor[4] != 0) && (config & PIN_SENSOR5_CFG_LVDS_VDHD)) {
			// Assing LVDS VD/HD when project layer select this config
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC; //XVS
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC; //XHS
			top_reg4.bit.XVS_XHS5_EN = XVS_XHS_SEL_ENUM_XVS_XHS;
		}

		if (config & PIN_SENSOR5_CFG_MCLK2) {
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			mclk2_sensor[4] = 1;
		}

	}
	return E_OK;
}

static int pinmux_config_sensor6(uint32_t config)
{
	uint32_t tmp;

	if (config == PIN_SENSOR6_CFG_NONE) {
	} else if (config & (PIN_SENSOR6_CFG_8BITS|PIN_SENSOR6_CFG_12BITS|PIN_SENSOR6_CFG_16BITS|PIN_SENSOR6_CFG_MIPI|PIN_SENSOR6_CFG_LVDS)) {

		tmp = config & (PIN_SENSOR6_CFG_8BITS|PIN_SENSOR6_CFG_12BITS|PIN_SENSOR6_CFG_16BITS|PIN_SENSOR6_CFG_MIPI|PIN_SENSOR6_CFG_LVDS);
		switch (tmp) {
		case PIN_SENSOR6_CFG_8BITS:
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC; //PXCLK
			top_reg3.bit.SENSOR6 = SENSOR6_ENUM_8BITS;
			break;

		case PIN_SENSOR6_CFG_12BITS:
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10= GPIO_ID_EMUM_FUNC; //PXCLK
			top_reg3.bit.SENSOR6 = SENSOR6_ENUM_16BITS;
			ui_sie6_16_12 = 1;
			break;

		case PIN_SENSOR6_CFG_16BITS:
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10= GPIO_ID_EMUM_FUNC; //PXCLK
			top_reg3.bit.SENSOR6 = SENSOR6_ENUM_16BITS;
			ui_sie6_16_12 = 0;
			break;

		case PIN_SENSOR6_CFG_MIPI:
			mipi_lvds_sensor[5] = PIN_SENSOR6_CFG_MIPI;
			break;

		case PIN_SENSOR6_CFG_LVDS:
			mipi_lvds_sensor[5] = PIN_SENSOR6_CFG_LVDS;
			break;

		default:
			pr_err("sensor6 no bus width assigned: 0x%x\r\n", config);
			break;
		}

		if ((mipi_lvds_sensor[5] != 0) && (config & PIN_SENSOR6_CFG_LVDS_VDHD)) {
			// Assing LVDS VD/HD when project layer select this config
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC; //XVS
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC; //XHS
			top_reg4.bit.XVS_XHS6_EN = XVS_XHS_SEL_ENUM_XVS_XHS;
		}

		if (config & PIN_SENSOR6_CFG_MCLK2) {
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			mclk2_sensor[5] = 1;
		}

	}
	return E_OK;
}


static int pinmux_config_sensor7(uint32_t config)
{
	uint32_t tmp;
	if (config == PIN_SENSOR7_CFG_NONE) {
	} else if (config & (PIN_SENSOR7_CFG_8BITS|PIN_SENSOR7_CFG_MIPI|PIN_SENSOR7_CFG_LVDS)) {
		tmp = config & (PIN_SENSOR7_CFG_8BITS|PIN_SENSOR7_CFG_MIPI|PIN_SENSOR7_CFG_LVDS);
		switch (tmp) {
		case PIN_SENSOR7_CFG_8BITS:
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC; //PXCLK
			top_reg3.bit.SENSOR7 = SENSOR7_ENUM_8BITS;
			break;

		case PIN_SENSOR7_CFG_MIPI:
			mipi_lvds_sensor[6] = PIN_SENSOR7_CFG_MIPI;
			break;

		case PIN_SENSOR7_CFG_LVDS:
			mipi_lvds_sensor[6] = PIN_SENSOR7_CFG_LVDS;
			break;

		default:
			pr_err("sensor7 no bus width assigned: 0x%x\r\n", config);
			break;
		}

		if ((mipi_lvds_sensor[6] != 0) && (config & PIN_SENSOR7_CFG_LVDS_VDHD)) {
			// Assing LVDS VD/HD when project layer select this config
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC; //XVS
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC; //XHS
			top_reg4.bit.XVS_XHS7_EN = XVS_XHS_SEL_ENUM_XVS_XHS;
		}

		if (config & PIN_SENSOR7_CFG_MCLK2) {
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			mclk2_sensor[6] = 1;
		}

	}
	return E_OK;
}

static int pinmux_config_sensor8(uint32_t config)
{
	uint32_t tmp;
	if (config == PIN_SENSOR8_CFG_NONE) {
	} else if (config & (PIN_SENSOR8_CFG_8BITS|PIN_SENSOR8_CFG_MIPI|PIN_SENSOR8_CFG_LVDS)) {
		tmp = config & (PIN_SENSOR8_CFG_8BITS|PIN_SENSOR8_CFG_MIPI|PIN_SENSOR8_CFG_LVDS);
		switch (tmp) {
		case PIN_SENSOR8_CFG_8BITS:
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC; //PXCLK
			top_reg3.bit.SENSOR8 = SENSOR8_ENUM_8BITS;
			break;

		case PIN_SENSOR8_CFG_MIPI:
			mipi_lvds_sensor[7] = PIN_SENSOR8_CFG_MIPI;
			break;

		case PIN_SENSOR8_CFG_LVDS:
			mipi_lvds_sensor[7] = PIN_SENSOR8_CFG_LVDS;
			break;

		default:
			pr_err("sensor8 no bus width assigned: 0x%x\r\n", config);
			break;
		}

		if ((mipi_lvds_sensor[7] != 0) && (config & PIN_SENSOR8_CFG_LVDS_VDHD)) {
			// Assing LVDS VD/HD when project layer select this config
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC; //XVS
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC; //XHS
			top_reg4.bit.XVS_XHS8_EN = XVS_XHS_SEL_ENUM_XVS_XHS;
		}

		if (config & PIN_SENSOR8_CFG_MCLK2) {
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			mclk2_sensor[7] = 1;
		}

	}
	return E_OK;
}

static int pinmux_config_mipi_lvds(uint32_t config)
{
	if (config == PIN_MIPI_LVDS_CFG_NONE) {
	} else {
		// check clk lane 0
		if (config & PIN_MIPI_LVDS_CFG_CLK0) {
			top_reg_hgpi0.bit.HSIGPI_18 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_19 = GPIO_ID_EMUM_FUNC;
		}

		// check clk lane 1
		if (config & PIN_MIPI_LVDS_CFG_CLK1) {
			top_reg_hgpi0.bit.HSIGPI_22 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_23 = GPIO_ID_EMUM_FUNC;
		}

		// check clk lane 2
		if (config & PIN_MIPI_LVDS_CFG_CLK2) {
			top_reg_hgpi0.bit.HSIGPI_26 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_27 = GPIO_ID_EMUM_FUNC;
		}

		// check clk lane 3
		if (config & PIN_MIPI_LVDS_CFG_CLK3) {
			top_reg_hgpi0.bit.HSIGPI_30 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_31 = GPIO_ID_EMUM_FUNC;
		}

		// check clk lane 4
		if (config & PIN_MIPI_LVDS_CFG_CLK4) {
			top_reg_hgpi0.bit.HSIGPI_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_3 = GPIO_ID_EMUM_FUNC;
		}

		// check clk lane 5
		if (config & PIN_MIPI_LVDS_CFG_CLK5) {
			top_reg_hgpi0.bit.HSIGPI_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_7 = GPIO_ID_EMUM_FUNC;
		}

		// check clk lane 6
		if (config & PIN_MIPI_LVDS_CFG_CLK6) {
			top_reg_hgpi0.bit.HSIGPI_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_11 = GPIO_ID_EMUM_FUNC;
		}

		// check clk lane 7
		if (config & PIN_MIPI_LVDS_CFG_CLK7) {
			top_reg_hgpi0.bit.HSIGPI_14 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_15 = GPIO_ID_EMUM_FUNC;
		}

		// check data lane 0
		if (config & PIN_MIPI_LVDS_CFG_DAT0) {
			top_reg_hgpi0.bit.HSIGPI_16 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_17 = GPIO_ID_EMUM_FUNC;
		}

		// check data lane 1
		if (config & PIN_MIPI_LVDS_CFG_DAT1) {
			top_reg_hgpi0.bit.HSIGPI_20 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_21 = GPIO_ID_EMUM_FUNC;
		}

		// check data lane 2
		if (config & PIN_MIPI_LVDS_CFG_DAT2) {
			top_reg_hgpi0.bit.HSIGPI_24 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_25 = GPIO_ID_EMUM_FUNC;
		}

		// check data lane 3
		if (config & PIN_MIPI_LVDS_CFG_DAT3) {
			top_reg_hgpi0.bit.HSIGPI_28 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_29 = GPIO_ID_EMUM_FUNC;
		}

		// check data lane 4
		if (config & PIN_MIPI_LVDS_CFG_DAT4) {
			top_reg_hgpi0.bit.HSIGPI_0 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_1 = GPIO_ID_EMUM_FUNC;
		}

		// check data lane 5
		if (config & PIN_MIPI_LVDS_CFG_DAT5) {
			top_reg_hgpi0.bit.HSIGPI_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_5 = GPIO_ID_EMUM_FUNC;
		}

		// check data lane 6
		if (config & PIN_MIPI_LVDS_CFG_DAT6) {
			top_reg_hgpi0.bit.HSIGPI_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_9 = GPIO_ID_EMUM_FUNC;
		}

		// check data lane 7
		if (config & PIN_MIPI_LVDS_CFG_DAT7) {
			top_reg_hgpi0.bit.HSIGPI_12 = GPIO_ID_EMUM_FUNC;
			top_reg_hgpi0.bit.HSIGPI_13 = GPIO_ID_EMUM_FUNC;
		}

	}

	return E_OK;
}

static int pinmux_config_i2c(uint32_t config)
{
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		if (config & PIN_I2C_CFG_CH1) {
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH1_2ND_PINMUX) {
			if (top_reg1.bit.NAND_EXIST == NAND_EXIST_EN) {
				pr_err("I2C_2 conflict with NAND\r\n");
				return E_OBJ;
			}

			if ((top_reg9.bit.UART4 == UART4_ENUM_2ND_PINMUX)) {
				pr_err("I2C_2 conflict with UART4_2\r\n");
				return E_OBJ;
			}

			if ((top_reg11.bit.TRACE_EN == TRACE_ENUM_TRACE)) {
				pr_err("I2C_2 conflict with TRACE\r\n");
				return E_OBJ;
			}

			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C_2 = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH2) {
			if (top_reg5.bit.SIFCH6 == SIFCH_ENUM_SIF) {
				pr_err("I2C2 conflict with SIFCH6\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH7 == SIFCH_ENUM_SIF) {
				pr_err("I2C2 conflict with SIFCH7\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SPI4 == SPI4_1ST_PINMUX) {
				pr_err("I2C2 conflict with SPI4\r\n");
				return E_OBJ;
			}

			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C2 = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH2_2ND_PINMUX) {
			if (top_reg7.bit.PWM8 == PWM_ENUM_PWM) {
				pr_err("I2C2_2 conflict with to PWM8: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg7.bit.PWM9 == PWM_ENUM_PWM) {
				pr_err("I2C2_2 conflict with to PWM9: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg5.bit.SPI3 == SPI3_2ND_PINMUX) {
				pr_err("I2C2_2 conflict with to SPI3_2: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg9.bit.UART3 == UART3_ENUM_2ND_PINMUX) {
				pr_err("I2C2_2 conflict with to UART3_2: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF_2ND_PINMUX) {
				pr_err("I2C2_2 conflict with to SIFCH2_2: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF_2ND_PINMUX) {
				pr_err("I2C2_2 conflict with to SIFCH3_2: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
				pr_err("I2C2_2 conflict with to SENSOR6: 0x%x\r\n", config);
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C2_2 = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH3) {
			if (top_reg5.bit.SIFCH6 == SIFCH_ENUM_SIF) {
				pr_err("I2C3 conflict with to SIFCH6: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH7 == SIFCH_ENUM_SIF) {
				pr_err("I2C3 conflict with to SIFCH7: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg5.bit.SPI4 == SPI4_1ST_PINMUX) {
				pr_err("I2C3 conflict with to SPI4_1: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg4.bit.STROBE_SEL == STROBE_ENUM_STROBE) {
				pr_err("I2C3 conflict with to STROBE: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg4.bit.SHUTTER_SEL == SHUTTER_ENUM_SHUTTER) {
				pr_err("I2C3 conflict with to SHUTTER: 0x%x\r\n", config);
				return E_OBJ;
			}

			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C3 = I2C_ENUM_I2C;

		}

		if (config & PIN_I2C_CFG_CH3_2ND_PINMUX) {
			if (top_reg7.bit.PWM10 == PWM_ENUM_PWM) {
				pr_err("PWM10 conflict with to PWM8: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg5.bit.SPI3 == SPI3_2ND_PINMUX) {
				pr_err("I2C3_2 conflict with SPI3_2\r\n");
				return E_OBJ;
			}

			if (top_reg9.bit.UART3 == UART3_ENUM_2ND_PINMUX) {
				pr_err("I2C3_2 conflict with UART3_2\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF_2ND_PINMUX) {
				pr_err("I2C3_2 conflict with SIFCH2_2\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF_2ND_PINMUX) {
				pr_err("I2C3_2 conflict with SIFCH3_2\r\n");
				return E_OBJ;
			}

			if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
				pr_err("I2C3_2 conflict with SENSOR6\r\n");
				return E_OBJ;
			}

			if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
				pr_err("I2C3_2 conflict with SENSOR7\r\n");
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C3_2 = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH3_3RD_PINMUX) {
			if (top_reg5.bit.SIFCH4 == SIFCH_ENUM_SIF) {
				pr_err("I2C3_3 conflict with SIFCH4\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH5 == SIFCH_ENUM_SIF) {
				pr_err("I2C3_3 conflict with SIFCH5\r\n");
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C3_3 = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH4) {
			if (top_reg7.bit.PWM16 == PWM_ENUM_PWM) {
				pr_err("I2C4 conflict with to PWM16: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg7.bit.PWM17 == PWM_ENUM_PWM) {
				pr_err("I2C4 conflict with to PWM17: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg4.bit.MES_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX) {
				pr_err("I2C4 conflict with to MES_2_SH0: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg4.bit.MES_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX) {
				pr_err("I2C4 conflict with to MES_2_SH1: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg5.bit.SPI == SPI_2ND_PINMUX) {
				pr_err("I2C4 conflict with SPI_2\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_2ND_PINMUX) {
				pr_err("I2C4 conflict with SIFCH0_2\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_3RD_PINMUX) {
				pr_err("I2C4 conflict with SIFCH0_3\r\n");
				return E_OBJ;
			}

			if ((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1)) {
				pr_err("I2C4 conflict with SENSOR6\r\n");
				return E_OBJ;
			}

			if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
				pr_err("I2C4 conflict with SENSOR7\r\n");
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C4 = I2C_ENUM_I2C;

		}

		if (config & PIN_I2C_CFG_CH4_2ND_PINMUX) {
			if (top_reg5.bit.SIFCH4 == SIFCH_ENUM_SIF) {
				pr_err("I2C4_2 conflict with SIFCH4\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH5 == SIFCH_ENUM_SIF) {
				pr_err("I2C4_2 conflict with SIFCH5\r\n");
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C4_2 = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH4_3RD_PINMUX) {
			if (top_reg7.bit.PWM18 == PWM_ENUM_PWM) {
				pr_err("I2C4_3 conflict with to PWM18: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg7.bit.PWM19 == PWM_ENUM_PWM) {
				pr_err("I2C4_3 conflict with to PWM19: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX) {
				pr_err("I2C4_3 conflict with to MES2_2_SH0: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX) {
				pr_err("I2C4_3 conflict with to MES2_2_SH1: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg5.bit.SPI == SPI_2ND_PINMUX) {
				pr_err("I2C4_3 conflict with SPI_2\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_2ND_PINMUX) {
				pr_err("I2C4_3 conflict with SIFCH0_2\r\n");
				return E_OBJ;
			}

			if (top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) {
				pr_err("I2C4 conflict with SENSOR6\r\n");
				return E_OBJ;
			}

			if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
				pr_err("I2C4 conflict with SENSOR7\r\n");
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C4_3 = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH4_4TH_PINMUX) {
			if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF) {
				pr_err("I2C4_4 conflict with SIFCH0_1\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SPI4 == SPI4_3RD_PINMUX) {
				pr_err("I2C4_4 conflict with SPI4_3\r\n");
				return E_OBJ;
			}

			if (top_reg6.bit.ETH!=ETH_ID_ENUM_GPIO) {
				pr_err("I2C4_4 conflict with ETH\r\n");
				return E_OBJ;
			}

			top_reg_lgpio0.bit.LGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_29 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C4_4 = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH5) {
			top_reg_pgpio1.bit.PGPIO_45 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_46 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C5 = I2C_ENUM_I2C;
		}

		if (config & PIN_I2C_CFG_CH5_2ND_PINMUX) {
			if (top_reg1.bit.NAND_EXIST == NAND_EXIST_EN) {
				pr_err("I2C5_2 conflict with NAND\r\n");
				return E_OBJ;
			}

			if ((top_reg9.bit.UART4 == UART4_ENUM_2ND_PINMUX) && (top_reg9.bit.UART4_CTSRTS == UART_CTSRTS_PINMUX)) {
				pr_err("I2C5_2 conflict with UART4_2 CTSRTS\r\n");
				return E_OBJ;
			}

			if (top_reg11.bit.TRACE_EN == TRACE_ENUM_TRACE) {
				pr_err("I2C5_2 conflict with TRACE\r\n");
				return E_OBJ;
			}

			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg10.bit.I2C5_2 = I2C_ENUM_I2C;
		}

	}

	return E_OK;
}

static int pinmux_config_sif(uint32_t config)
{
	if (config == PIN_SIF_CFG_NONE) {
	} else {
		if (config & (PIN_SIF_CFG_CH0|PIN_SIF_CFG_CH0_2ND_PINMUX|PIN_SIF_CFG_CH0_3RD_PINMUX)) {
			if (config & PIN_SIF_CFG_CH0_2ND_PINMUX) {
				if (top_reg7.bit.PWM16 == PWM_ENUM_PWM) {
					pr_err("SIFCH0_2 conflict with PWM16\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.PWM18 == PWM_ENUM_PWM) {
					pr_err("SIFCH0_2 conflict with PWM18\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.PWM19 == PWM_ENUM_PWM) {
					pr_err("SIFCH0_2 conflict with PWM19\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.MES_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX) {
					pr_err("SIFCH0_2 conflict with MES_2_SHUT0\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX) {
					pr_err("SIFCH0_2 conflict with MES2_2_SHUT0\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX) {
					pr_err("SIFCH0_2 conflict with MES2_2_SHUT1\r\n");
					return E_OBJ;
				}

				if (top_reg5.bit.SPI == SPI_MUX_2ND) {
					pr_err("SIFCH0_2 conflict with SPI_2\r\n");
					return E_OBJ;
				}

				if (top_reg10.bit.I2C4 == I2C_ENUM_I2C) {
					pr_err("SIFCH0_2 conflict with I2C4\r\n");
					return E_OBJ;
				}

				if (top_reg10.bit.I2C4_3 == I2C_ENUM_I2C) {
					pr_err("SIFCH0_2 conflict with I2C4_3\r\n");
					return E_OBJ;
				}

				if (top_reg3.bit.SENSOR7 == SENSOR7_ENUM_8BITS) {
					pr_err("SIFCH0_2 conflict with SENSOR7\r\n");
					return E_OBJ;
				}

				if ((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1)) {
					pr_err("SIFCH0_2 conflict with SENSOR6 16 bits\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SIFCH0 = SIFCH_ENUM_SIF_2ND_PINMUX;

			} else if (config & PIN_SIF_CFG_CH0_3RD_PINMUX) {
				if (top_reg7.bit.PWM17 == PWM_ENUM_PWM) {
					pr_err("SIFCH0_3 conflict with PWM17\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.PWM18 == PWM_ENUM_PWM) {
					pr_err("SIFCH0_3 conflict with PWM18\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.PWM19 == PWM_ENUM_PWM) {
					pr_err("SIFCH0_3 conflict with PWM19\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.MES_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX) {
					pr_err("SIFCH0_3 conflict with MES_2_SHUT1\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX) {
					pr_err("SIFCH0_3 conflict with MES2_2_SHUT0\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX) {
					pr_err("SIFCH0_3 conflict with MES2_2_SHUT1\r\n");
					return E_OBJ;
				}

				if (top_reg5.bit.SPI == SPI_MUX_2ND) {
					pr_err("SIFCH0_3 conflict with SPI_2\r\n");
					return E_OBJ;
				}

				if (top_reg10.bit.I2C4 == I2C_ENUM_I2C) {
					pr_err("SIFCH0_3 conflict with I2C4\r\n");
					return E_OBJ;
				}

				if (top_reg10.bit.I2C4_3 == I2C_ENUM_I2C) {
					pr_err("SIFCH0_3 conflict with I2C4_3\r\n");
					return E_OBJ;
				}

				if (top_reg3.bit.SENSOR7 == SENSOR_ENUM_8BITS) {
					pr_err("SIFCH0_3 conflict with SENSOR7\r\n");
					return E_OBJ;
				}

				if ((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1)) {
					pr_err("SIFCH0_3 conflict with SENSOR6 16 bits\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SIFCH0 = SIFCH_ENUM_SIF_3RD_PINMUX;
			} else {
				if (top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_PARALLEL_LCD) {
					pr_err("SIFCH0_1 conflict with LCD Parallel\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.PI_CNT4 == PICNT_ENUM_PICNT2) {
					pr_err("SIFCH0_1 conflict with PI_CNT4_2\r\n");
					return E_OBJ;
				}

				if (top_reg10.bit.I2C4_4 == I2C_ENUM_I2C) {
					pr_err("SIFCH0_1 conflict with I2C4_4\r\n");
					return E_OBJ;
				}

				if (top_reg5.bit.SPI4 == SPI_MUX_3RD) {
					pr_err("SIFCH0_1 conflict with SPI4_3\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
					pr_err("SIFCH0_1 conflict with ETH\r\n");
					return E_OBJ;
				}

				if (top_reg5.bit.SPI4 == SPI4_3RD_PINMUX) {
					pr_err("SIFCH0_1 conflict with SPI4_3\r\n");
					return E_OBJ;
				}

				top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_28 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_29 = GPIO_ID_EMUM_FUNC;

				top_reg5.bit.SIFCH0 = SIFCH_ENUM_SIF;
			}
		}

		if (config & (PIN_SIF_CFG_CH1|PIN_SIF_CFG_CH1_2ND_PINMUX)) {
			if (config & PIN_SIF_CFG_CH1_2ND_PINMUX) {
				pr_err("SIF_CH1 no 2ND PINMUX\r\n");
				return E_OBJ;
			} else {
				top_reg_lgpio0.bit.LGPIO_30 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_31 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio1.bit.LGPIO_32 = GPIO_ID_EMUM_FUNC;
			}
			top_reg5.bit.SIFCH1 = SIFCH_ENUM_SIF;
		}

		if (config & (PIN_SIF_CFG_CH2|PIN_SIF_CFG_CH2_2ND_PINMUX)) {
			if (config & PIN_SIF_CFG_CH2_2ND_PINMUX) {
				if (top_reg7.bit.PWM8 == PWM_ENUM_PWM) {
					pr_err("SIFCH2_2 conflict with PWM8\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.PWM10 == PWM_ENUM_PWM) {
					pr_err("SIFCH2_2 conflict with PWM10\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.PWM11 == PWM_ENUM_PWM) {
					pr_err("SIFCH2_2 conflict with PWM11\r\n");
					return E_OBJ;
				}

				if (top_reg5.bit.SPI3 == SPI3_2ND_PINMUX) {
					pr_err("SIFCH2_2 conflict with SPI3_2\r\n");
					return E_OBJ;
				}

				if (top_reg9.bit.UART3 == UART3_ENUM_2ND_PINMUX) {
					pr_err("SIFCH2_2 conflict with UART3_2\r\n");
					return E_OBJ;
				}

				if (top_reg10.bit.I2C2_2 == I2C_ENUM_I2C) {
					pr_err("SIFCH2_2 conflict with I2C2_2\r\n");
					return E_OBJ;
				}

				if (top_reg10.bit.I2C3_2 == I2C_ENUM_I2C) {
					pr_err("SIFCH2_2 conflict with I2C3_2\r\n");
					return E_OBJ;
				}

				if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
					pr_err("SIFCH2_2 conflict with SENSOR6\r\n");
					return E_OBJ;
				}

				if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
					pr_err("SIFCH2_2 conflict with SENSOR7\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SIFCH2 = SIFCH_ENUM_SIF_2ND_PINMUX;
			} else {
				if (top_reg5.bit.SPI2 == SPI2_1ST_PINMUX) {
					pr_err("SIFCH2 conflict with SPI2_1\r\n");
					return E_OBJ;
				}

				if (top_reg9.bit.UART3 == UART3_ENUM_1ST_PINMUX) {
					pr_err("SIFCH2 conflict with UART3_1\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.AUDIO_MCLK == AUDIO_MCLK_MCLK) {
					pr_err("SIFCH2 conflict with AUDIO MCLK\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.CANBUS == CANBUS_ENUM_CANBUS) {
					pr_err("SIFCH2 conflict with CANBUS\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.SP_CLK == SP_CLK_SEL_ENUM_SP_2_CLK) {
					pr_err("SIFCH2 conflict with SP_CLK_2\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.LOCKN == LOCKN_ENUM_LOCKN) {
					pr_err("SIFCH2 conflict with LOCKN\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SIFCH2 = SIFCH_ENUM_SIF;
			}
		}

		if (config & (PIN_SIF_CFG_CH3|PIN_SIF_CFG_CH3_2ND_PINMUX)) {
			if (config & PIN_SIF_CFG_CH3_2ND_PINMUX) {
				if (top_reg7.bit.PWM9 == PWM_ENUM_PWM) {
					pr_err("SIFCH3_2 conflict with PWM9\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.PWM10 == PWM_ENUM_PWM) {
					pr_err("SIFCH3_2 conflict with PWM10\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.PWM11 == PWM_ENUM_PWM) {
					pr_err("SIFCH3_2 conflict with PWM11\r\n");
					return E_OBJ;
				}

				if (top_reg5.bit.SPI3 == SPI3_2ND_PINMUX) {
					pr_err("SIFCH3_2 conflict with SPI3_2\r\n");
					return E_OBJ;
				}

				if (top_reg9.bit.UART3 == UART3_ENUM_2ND_PINMUX) {
					pr_err("SIFCH3_2 conflict with UART3_2\r\n");
					return E_OBJ;
				}

				if (top_reg10.bit.I2C2_2 == I2C_ENUM_I2C) {
					pr_err("SIFCH3_2 conflict with I2C2_2\r\n");
					return E_OBJ;
				}

				if (top_reg10.bit.I2C3_2 == I2C_ENUM_I2C) {
					pr_err("SIFCH3_2 conflict with I2C3_2\r\n");
					return E_OBJ;
				}

				if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
					pr_err("SIFCH3_2 conflict with SENSOR6\r\n");
					return E_OBJ;
				}

				if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
					pr_err("SIFCH3_2 conflict with SENSOR7\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SIFCH3 = SIFCH_ENUM_SIF_2ND_PINMUX;

			} else {
				if (top_reg5.bit.SPI2 == SPI2_1ST_PINMUX) {
					pr_err("SIFCH3 conflict with SPI2_1\r\n");
					return E_OBJ;
				}

				if (top_reg9.bit.UART3 == UART3_ENUM_1ST_PINMUX) {
					pr_err("SIFCH3 conflict with UART3_1\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
					pr_err("SIFCH3 conflict with ETH\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.SP2_CLK == SP2_CLK_SEL_ENUM_SP2_2_CLK) {
					pr_err("SIFCH3 conflict with SPCLK_2_2\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.CANBUS == CANBUS_ENUM_CANBUS) {
					pr_err("SIFCH3 conflict with CANBUS\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.LOCKN2 == LOCKN_ENUM_LOCKN) {
					pr_err("SIFCH3 conflict with LOCKN2\r\n");
					return E_OBJ;
				}

				if (top_reg2.bit.TV_TEST_CLK == TV_TEST_CLK_TV_CLK) {
					pr_err("SIFCH3 conflict with TV_TEST_CLK\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SIFCH3 = SIFCH_ENUM_SIF;
			}
		}

		if (config & PIN_SIF_CFG_CH4) {
			if (top_reg10.bit.I2C4_2 == I2C_ENUM_I2C) {
				pr_err("SIFCH4 conflict with I2C4_2\r\n");
				return E_OBJ;
			}

			if (top_reg10.bit.I2C3_3 == I2C_ENUM_I2C) {
				pr_err("SIFCH4 conflict with I2C3_3\r\n");
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.SIFCH4 = SIFCH_ENUM_SIF;
		}

		if (config & PIN_SIF_CFG_CH5) {
			if (top_reg10.bit.I2C4_2 == I2C_ENUM_I2C) {
				pr_err("SIFCH4 conflict with I2C4_2\r\n");
				return E_OBJ;
			}

			if (top_reg10.bit.I2C3_3 == I2C_ENUM_I2C) {
				pr_err("SIFCH4 conflict with I2C3_3\r\n");
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.SIFCH5 = SIFCH_ENUM_SIF;
		}

		if (config & PIN_SIF_CFG_CH6) {
			if (top_reg10.bit.I2C2 == I2C_ENUM_I2C) {
				pr_err("SIFCH6 conflict with I2C2\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SPI4 == SPI_MUX_1ST) {
				pr_err("SIFCH6 conflict with SPI4_1\r\n");
				return E_OBJ;
			}

			if (top_reg4.bit.STROBE_SEL == STROBE_ENUM_STROBE) {
				pr_err("SIFCH6 conflict with STROBE\r\n");
				return E_OBJ;
			}

			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.SIFCH6 = SIFCH_ENUM_SIF;
		}

		if (config & PIN_SIF_CFG_CH7) {
			if (top_reg10.bit.I2C2 == I2C_ENUM_I2C) {
				pr_err("SIFCH7 conflict with I2C2\r\n");
				return E_OBJ;
			}

			if (top_reg5.bit.SPI4 == SPI_MUX_1ST) {
				pr_err("SIFCH7 conflict with SPI4_1\r\n");
				return E_OBJ;
			}

			if (top_reg4.bit.SHUTTER_SEL == SHUTTER_ENUM_SHUTTER) {
				pr_err("SIFCH7 conflict with SHUTTER\r\n");
				return E_OBJ;
			}

			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.SIFCH7 = SIFCH_ENUM_SIF;
		}
	}

	return E_OK;
}

static int pinmux_config_uart(uint32_t config)
{
	if (config == PIN_UART_CFG_NONE) {
	} else if (config & (PIN_UART_CFG_CH1|PIN_UART_CFG_CH2|PIN_UART_CFG_CH3|PIN_UART_CFG_CH4)) {
		if (config & (PIN_UART_CFG_CH1|PIN_UART_CFG_CH1_TX)) {
			top_reg_pgpio1.bit.PGPIO_42 = GPIO_ID_EMUM_FUNC;

			if (config & (PIN_UART_CFG_CH1)) {
				top_reg_pgpio1.bit.PGPIO_43 = GPIO_ID_EMUM_FUNC;
			}
			top_reg9.bit.UART = UART_ENUM_UART;
		}

		if (config & PIN_UART_CFG_CH2) {
			if (config & PIN_UART_CFG_CH2_2ND) {
				if ((top_reg1.bit.SDIO3_EXIST == SDIO3_EXIST_EN)  && (top_reg1.bit.SDIO3_MUX_SEL == SDIO3_MUX_1ST)) {
					pr_err("UART2 conflict with to SDIO3_1: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg5.bit.SPI3 == SPI3_1ST_PINMUX) {
					pr_err("UART2 conflict with to SPI3_1: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (config & PIN_UART_CFG_CH2_CTSRTS) {
					top_reg_cgpio0.bit.CGPIO_31 = GPIO_ID_EMUM_FUNC;
					top_reg_cgpio1.bit.CGPIO_32 = GPIO_ID_EMUM_FUNC;
					top_reg9.bit.UART2_CTSRTS = UART_CTSRTS_PINMUX;
				}

				top_reg_cgpio0.bit.CGPIO_29 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_30 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.UART2 = UART2_ENUM_2ND_PINMUX;
			} else {
				if (top_reg5.bit.SPI4 == SPI4_2ND_PINMUX) {
					pr_err("UART2 conflict with to SPI4_2: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg6.bit.AUDIO == AUDIO_ENUM_I2S) {
					pr_err("UART2 conflict with to AUDIO: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (config & PIN_UART_CFG_CH2_CTSRTS) {
					if ((top_reg7.bit.PI_CNT == PICNT_ENUM_PICNT)||(top_reg7.bit.PI_CNT2 == PICNT_ENUM_PICNT)) {
						pr_err("UART2 conflict with to PICNT_1 or PICNT2_1: 0x%x\r\n", config);
						return E_OBJ;
					}

					top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
					top_reg_pgpio1.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC;
					top_reg9.bit.UART2_CTSRTS = UART_CTSRTS_PINMUX;
				}

				top_reg_pgpio1.bit.PGPIO_34 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_35 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.UART2 = UART2_ENUM_1ST_PINMUX;
			}
		}

		if (config & PIN_UART_CFG_CH3) {
			if (config & PIN_UART_CFG_CH3_2ND) {
				if (top_reg5.bit.SPI3 == SPI3_2ND_PINMUX) {
					pr_err("UART3 conflict with to SPI3_2: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg10.bit.I2C2_2 == I2C_ENUM_I2C) {
					pr_err("UART3 conflict with to I2C2_2: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg7.bit.PWM8 == PWM_ENUM_PWM) {
					pr_err("UART3 conflict with to PWM8: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg7.bit.PWM9 == PWM_ENUM_PWM) {
					pr_err("UART3 conflict with to PWM9: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF_2ND_PINMUX) {
					pr_err("UART3 conflict with to SIFCH2_2: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF_2ND_PINMUX) {
					pr_err("UART3 conflict with to SIFCH3_2: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
					pr_err("UART3 conflict with to SENSOR6: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (config & PIN_UART_CFG_CH3_CTSRTS) {
					if (top_reg7.bit.PWM10 == PWM_ENUM_PWM) {
						pr_err("UART3 conflict with to PWM10: 0x%x\r\n", config);
						return E_OBJ;
					}

					if (top_reg7.bit.PWM11 == PWM_ENUM_PWM) {
						pr_err("UART3 conflict with to PWM11: 0x%x\r\n", config);
						return E_OBJ;
					}

					if (top_reg10.bit.I2C3_2 == I2C_ENUM_I2C) {
						pr_err("UART3 conflict with to I2C3_2: 0x%x\r\n", config);
						return E_OBJ;
					}

					if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
						pr_err("UART3 conflict with to SENSOR7: 0x%x\r\n", config);
						return E_OBJ;
					}

					top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
					top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
					top_reg9.bit.UART3_CTSRTS = UART_CTSRTS_PINMUX;
				}

				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.UART3 = UART3_ENUM_2ND_PINMUX;
			} else {
				if (top_reg5.bit.SPI2 == SPI2_1ST_PINMUX) {
					pr_err("UART3 conflict with to SPI2_1: 0x%x\r\n", config);
					return E_OBJ;
				}

				if ((top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF)||(top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF)) {
					pr_err("UART3 conflict with to SIF2 or SIF3: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg6.bit.AUDIO_MCLK == AUDIO_MCLK_MCLK) {
					pr_err("UART3 conflict with AUDIO MCLK\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.CANBUS== CANBUS_ENUM_CANBUS) {
					pr_err("UART3 conflict with CANBUS\r\n");
					return E_OBJ;
				}

				if (top_reg2.bit.TV_TEST_CLK == TV_TEST_CLK_TV_CLK) {
					pr_err("UART3 conflict with TV_TEST_CLK\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.LOCKN == LOCKN_ENUM_LOCKN) {
					pr_err("UART3 conflict with LOCKN\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.LOCKN2 == LOCKN_ENUM_LOCKN) {
					pr_err("UART3 conflict with LOCKN2\r\n");
					return E_OBJ;
				}

				if (top_reg6.bit.SP_CLK == SP_CLK_SEL_ENUM_SP_2_CLK) {
					pr_err("UART3 conflict with to SP_CLK_2: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg6.bit.SP2_CLK == SP2_CLK_SEL_ENUM_SP2_2_CLK) {
					pr_err("UART3 conflict with to SP_CLK2_2: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (config & PIN_UART_CFG_CH3_CTSRTS) {
					if (top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
						pr_err("UART3 conflict with to ETH: 0x%x\r\n", config);
						return E_OBJ;
					}

					top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
					top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;
					top_reg9.bit.UART3_CTSRTS = UART_CTSRTS_PINMUX;
				}

				top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.UART3 = UART3_ENUM_1ST_PINMUX;

			}
		}

		if (config & PIN_UART_CFG_CH4) {
			if (config & PIN_UART_CFG_CH4_2ND) {
				if (top_reg1.bit.NAND_EXIST == NAND_EXIST_EN) {
					pr_err("UART4 conflict with to NAND: 0x%x\r\n", config);
					return E_OBJ;
				}

				if ((top_reg10.bit.I2C_2 == I2C_ENUM_I2C)) {
					pr_err("UART4 conflict with I2C_2\r\n");
					return E_OBJ;
				}

				if ((top_reg11.bit.TRACE_EN == TRACE_ENUM_TRACE)) {
					pr_err("UART4 conflict with TRACE\r\n");
					return E_OBJ;
				}

				if (config & PIN_UART_CFG_CH4_CTSRTS) {
					if ((top_reg10.bit.I2C5_2 == I2C_ENUM_I2C)) {
						pr_err("UART4 conflict with I2C5_2\r\n");
						return E_OBJ;
					}

					top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
					top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
					top_reg9.bit.UART4_CTSRTS = UART_CTSRTS_PINMUX;
				}

				top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.UART4 = UART4_ENUM_2ND_PINMUX;
			} else {
				if (top_reg7.bit.PWM4 == PWM_ENUM_PWM) {
					pr_err("UART4 conflict with to PWM4: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg7.bit.PWM5 == PWM_ENUM_PWM) {
					pr_err("UART4 conflict with to PWM5: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
					pr_err("UART4 conflict with to SENSOR6: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (top_reg4.bit.XVS_XHS4_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
					pr_err("UART4 conflict with to XVS_XHS4: 0x%x\r\n", config);
					return E_OBJ;
				}

				if (config & PIN_UART_CFG_CH4_CTSRTS) {
					if (top_reg7.bit.PWM6 == PWM_ENUM_PWM) {
						pr_err("UART4 conflict with to PWM6: 0x%x\r\n", config);
						return E_OBJ;
					}

					if (top_reg7.bit.PWM7 == PWM_ENUM_PWM) {
						pr_err("UART4 conflict with to PWM7: 0x%x\r\n", config);
						return E_OBJ;
					}

					if (top_reg4.bit.XVS_XHS5_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
						pr_err("UART4 conflict with to XVS_XHS5: 0x%x\r\n", config);
						return E_OBJ;
					}

					top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
					top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
					top_reg9.bit.UART4_CTSRTS = UART_CTSRTS_PINMUX;
				}

				top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.UART4 = UART4_ENUM_1ST_PINMUX;
			}
		}
	} else {
		pr_err("invalid config: 0x%x\r\n", config);
		return E_PAR;
	}

	return E_OK;
}

static int pinmux_config_spi(uint32_t config)
{
	if (config == PIN_SPI_CFG_NONE) {
	} else {
		if (config & (PIN_SPI_CFG_CH1|PIN_SPI_CFG_CH1_2ND_PINMUX|PIN_SPI_CFG_CH1_2BITS)) {
			if (config & PIN_SPI_CFG_CH1_2ND_PINMUX) {
				if ((top_reg7.bit.PWM16 == PWM_ENUM_PWM)||(top_reg4.bit.MES_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX)|| (top_reg10.bit.I2C4 == I2C_ENUM_I2C)||(top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_2ND_PINMUX)||((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1))) {
					pr_err("SPI2_2 conflict with PWM16 or MES or I2C4 or SIF_2\r\n");
					return E_OBJ;
				} else if ((top_reg7.bit.PWM17 == PWM_ENUM_PWM)||(top_reg4.bit.MES_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX)|| (top_reg10.bit.I2C4 == I2C_ENUM_I2C)||(top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_3RD_PINMUX)||((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1))) {
					pr_err("SPI2_2 conflict with PWM17 or MES2 or I2C4 or SIF_3\r\n");
					return E_OBJ;
				} else if ((top_reg7.bit.PWM18 == PWM_ENUM_PWM)||(top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX)|| (top_reg10.bit.I2C4_3 == I2C_ENUM_I2C)||(top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_2ND_PINMUX)) {
					pr_err("SPI2_2 conflict with PWM18 or MES_2 or I2C4_3 or SIF_2\r\n");
					return E_OBJ;
				} else if ((config & PIN_SPI_CFG_CH2_2BITS)&& ((top_reg7.bit.PWM19 == PWM_ENUM_PWM)||(top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX)|| (top_reg10.bit.I2C4_3 == I2C_ENUM_I2C)||(top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_2ND_PINMUX))) {
					pr_err("SPI2_2 conflict with PWM19 or MES_2_2 or I2C4_3 or SIF_2\r\n");
					return E_OBJ;
				}

				if (config & PIN_SPI_CFG_CH1_2BITS) {
					top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI = SPI_2ND_PINMUX;
			} else {
				if (config & PIN_SPI_CFG_CH1_2BITS) {
					top_reg_pgpio1.bit.PGPIO_41 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_pgpio1.bit.PGPIO_38 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_39 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_40 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI = SPI_1ST_PINMUX;
			}
		}

		if (config & (PIN_SPI_CFG_CH2|PIN_SPI_CFG_CH2_2ND_PINMUX|PIN_SPI_CFG_CH2_3RD_PINMUX|PIN_SPI_CFG_CH2_2BITS)) {
			if (config & PIN_SPI_CFG_CH2_3RD_PINMUX) {
				if (top_reg1.bit.SDIO2_EXIST == SDIO2_EXIST_EN) {
					pr_err("SPI2_3 conflict with SDIO2\r\n");
					return E_OBJ;
				} else if (top_reg11.bit.DSP_EJTAG_SEL) {
					pr_err("SPI2_3 conflict with DSP_EJTAG\r\n");
					return E_OBJ;
				}

				if (config & PIN_SPI_CFG_CH2_2BITS) {
					top_reg_cgpio0.bit.CGPIO_24 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI2_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_25 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI2 = SPI2_3RD_PINMUX;

			} else if (config & PIN_SPI_CFG_CH2_2ND_PINMUX) {
				if ((top_reg7.bit.PWM12 == PWM_ENUM_PWM) || (top_reg3.bit.SENSOR7 == SENSOR7_ENUM_8BITS) ) {
					pr_err("SPI2_2 conflict with PWM12 or SIE7 8 bits\r\n");
					return E_OBJ;
				} else if (top_reg7.bit.PWM13 == PWM_ENUM_PWM) {
					pr_err("SPI2_2 conflict with PWM13\r\n");
					return E_OBJ;
				} else if ((top_reg7.bit.PWM14 == PWM_ENUM_PWM) || ((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1))) {
					pr_err("SPI2_2 conflict with PWM14 or SIE6 16 bits\r\n");
					return E_OBJ;
				}

				if (config & PIN_SPI_CFG_CH2_2BITS) {
					if (top_reg7.bit.PWM15 == PWM_ENUM_PWM) {
						pr_err("SPI2_2 conflict with PWM15\r\n");
						return E_OBJ;
					}
				}

				if (config & PIN_SPI_CFG_CH2_2BITS) {
					top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI2_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI2 = SPI2_2ND_PINMUX;
			} else {
				if (top_reg9.bit.UART3 == UART3_ENUM_1ST_PINMUX) {
					pr_err("SPI2_1 conflict with UART3 1st\r\n");
					return E_OBJ;
				} else if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF) {
					pr_err("SPI2_1 conflict with SIF2\r\n");
					return E_OBJ;
				} else if (top_reg6.bit.AUDIO_MCLK == AUDIO_MCLK_MCLK) {
					pr_err("SPI2_1 conflict with AUDIO MCLK\r\n");
					return E_OBJ;
				} else if (top_reg6.bit.CANBUS == CANBUS_ENUM_CANBUS) {
					pr_err("SPI2_1 conflict with CANBUS\r\n");
					return E_OBJ;
				} else if (top_reg6.bit.SP_CLK == SP_CLK_SEL_ENUM_SP_2_CLK) {
					pr_err("SPI2_1 conflict with SPCLK_2\r\n");
					return E_OBJ;
				}

				if (config & PIN_SPI_CFG_CH2_2BITS) {
					if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF) {
						pr_err("SPI2_1 conflict with SIF3\r\n");
						return E_OBJ;
					} else if (top_reg6.bit.SP2_CLK == SP2_CLK_SEL_ENUM_SP2_2_CLK) {
						pr_err("SPI2_1 conflict with SP2_2_CLK\r\n");
						return E_OBJ;
					} else if (top_reg6.bit.CANBUS== CANBUS_ENUM_CANBUS) {
						pr_err("SPI2_1 conflict with CANBUS\r\n");
						return E_OBJ;
					} else if (top_reg2.bit.TV_TEST_CLK == TV_TEST_CLK_TV_CLK) {
						pr_err("SPI2_1 conflict with TV_TEST_CLK\r\n");
						return E_OBJ;
					} else if (top_reg4.bit.LOCKN2 == LOCKN_ENUM_LOCKN) {
						pr_err("SPI2_1 conflict with LOCKN2\r\n");
						return E_OBJ;
					}
				}

				if (config & PIN_SPI_CFG_CH2_2BITS) {
					top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI2_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI2 = SPI2_1ST_PINMUX;
			}
		}

		if (config & (PIN_SPI_CFG_CH3|PIN_SPI_CFG_CH3_2ND_PINMUX|PIN_SPI_CFG_CH3_2BITS|PIN_SPI_CFG_CH3_RDY|PIN_SPI_CFG_CH3_RDY_2ND_PINMUX)) {
			if (config & PIN_SPI_CFG_CH3_2ND_PINMUX) {
				if ((top_reg7.bit.PWM8 == PWM_ENUM_PWM) || (top_reg7.bit.PWM10 == PWM_ENUM_PWM) || (top_reg7.bit.PWM11 == PWM_ENUM_PWM)) {
					pr_err("SPI3_2 conflict with PWM8/10/11\r\n");
					return E_OBJ;
				} else if ((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_8BITS) || (top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) || (top_reg3.bit.SENSOR7 == SENSOR7_ENUM_8BITS)) {
					pr_err("SPI3_2 conflict with SIE6 or SIE7\r\n");
					return E_OBJ;
				} else if (top_reg9.bit.UART3 == UART3_ENUM_2ND_PINMUX) {
					pr_err("SPI3_2 conflict with UART3_2\r\n");
					return E_OBJ;
				} else if (top_reg10.bit.I2C2_2 == I2C_ENUM_I2C) {
					pr_err("SPI3_2 conflict with I2C2_2\r\n");
					return E_OBJ;
				} else if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF_2ND_PINMUX) {
					pr_err("SPI3_2 conflict with SIFCH2_2\r\n");
					return E_OBJ;
				}

				if (config & PIN_SPI_CFG_CH3_2BITS) {
					if (top_reg7.bit.PWM9 == PWM_ENUM_PWM) {
						pr_err("SPI3_2 conflict with PWM9\r\n");
						return E_OBJ;
					} else if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF_2ND_PINMUX) {
						pr_err("SPI3_2 conflict with SIFCH3_2\r\n");
						return E_OBJ;
					}
				}

				if (config & PIN_SPI_CFG_CH3_2BITS) {
					top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI3_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI3 = SPI3_2ND_PINMUX;
			} else {
				if (top_reg1.bit.SDIO3_EXIST == SDIO3_EXIST_EN) {
					pr_err("SPI3_1 conflict with SDIO3\r\n");
					return E_OBJ;
				} else if (top_reg9.bit.UART2 == UART2_ENUM_2ND_PINMUX) {
					pr_err("SPI3_1 conflict with UART2_2\r\n");
					return E_OBJ;
				}

				if (config & PIN_SPI_CFG_CH3_2BITS) {
					top_reg_cgpio0.bit.CGPIO_30 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI3_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_cgpio0.bit.CGPIO_28 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_29 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_31 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI3 = SPI3_1ST_PINMUX;
			}

			if (config & PIN_SPI_CFG_CH3_RDY_2ND_PINMUX) {
				if (top_reg7.bit.PWM3 == PWM_ENUM_PWM) {
					pr_err("SPI3_RDY2 conflict with PWM3\r\n");
					return E_OBJ;
				} else if (top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_1ST_PINMUX) {
					pr_err("SPI3_RDY2 conflict with MES2_SHUT1\r\n");
					return E_OBJ;
				} else if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
					pr_err("SPI3_RDY2 conflict with SENSOR6\r\n");
					return E_OBJ;
				} else if ((top_reg4.bit.XVS_XHS3_EN == VD_HD_SEL_ENUM_SIE_VDHD) && (top_reg4.bit.XVS_XHS3_SEL == XVS_XHS_2ND_PINMUX)) {
					pr_err("SPI3_RDY2 conflict with XVS_XHS3_2\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI3_RDY = SPI_RDY_2ND_PINMUX;
			} else if (config & PIN_SPI_CFG_CH3_RDY) {
				if (top_reg1.bit.SDIO3_EXIST == SDIO3_EXIST_EN) {
					pr_err("SPI3_RDY conflict with SDIO3\r\n");
					return E_OBJ;
				} else if ( (top_reg9.bit.UART2 == UART2_ENUM_2ND_PINMUX) && (top_reg9.bit.UART2_CTSRTS = UART_CTSRTS_PINMUX)) {
					pr_err("SPI3_RDY conflict with UART2_2 CTSRTS\r\n");
					return E_OBJ;
				}

				top_reg_cgpio1.bit.CGPIO_32 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI3_RDY = SPI_RDY_1ST_PINMUX;
			}
		}

		if (config & (PIN_SPI_CFG_CH4|PIN_SPI_CFG_CH4_2ND_PINMUX|PIN_SPI_CFG_CH4_3RD_PINMUX|PIN_SPI_CFG_CH4_2BITS)) {
			if (config & PIN_SPI_CFG_CH4_3RD_PINMUX) {
				if (top_reg7.bit.PI_CNT4 == PICNT_ENUM_PICNT2) {
					pr_err("SPI4_3 conflict with PI_CNT4\r\n");
					return E_OBJ;
				} else if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF) {
					pr_err("SPI4_3 conflict with SIFCH0\r\n");
					return E_OBJ;
				} else if (top_reg10.bit.I2C4_4 == I2C_ENUM_I2C) {
					pr_err("SPI4_3 conflict with I2C4_4\r\n");
					return E_OBJ;
				} else if (top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
					pr_err("SPI4_3 conflict with ETH\r\n");
					return E_OBJ;
				}

				if (config & PIN_SPI_CFG_CH4_2BITS) {
					if (top_reg6.bit.SP_CLK == SP_CLK_SEL_ENUM_SP_3_CLK) {
						pr_err("SPI4_3 conflict with SP_CLK_3\r\n");
						return E_OBJ;
					}

					top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI4_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_28 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_29 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI4 = SPI4_3RD_PINMUX;
			} else if (config & PIN_SPI_CFG_CH4_2ND_PINMUX) {
				if (top_reg9.bit.UART2 == UART2_ENUM_1ST_PINMUX) {
					pr_err("SPI4_2 conflict with UART2_1\r\n");
					return E_OBJ;
				} else if (top_reg6.bit.AUDIO == AUDIO_ENUM_I2S) {
					pr_err("SPI4_2 conflict with I2S\r\n");
					return E_OBJ;
				} else if (top_reg7.bit.PI_CNT == PICNT_ENUM_PICNT) {
					pr_err("SPI4_2 conflict with PICNT_1\r\n");
					return E_OBJ;
				} else if (config & PIN_SPI_CFG_CH4_2BITS) {
					if (top_reg7.bit.PI_CNT2 == PICNT_ENUM_PICNT) {
						pr_err("SPI4_2 conflict with PICNT2_1\r\n");
						return E_OBJ;
					}
				}

				if (config & PIN_SPI_CFG_CH4_2BITS) {
					top_reg_pgpio1.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI4_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_pgpio1.bit.PGPIO_34 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_35 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI4 = SPI4_2ND_PINMUX;
			} else {
				if ((top_reg10.bit.I2C2 == I2C_ENUM_I2C) || (top_reg10.bit.I2C3 == I2C_2_ENUM_I2C)) {
					pr_err("SPI4_1 conflict with I2C2_1 or I2C3_1\r\n");
					return E_OBJ;
				} else if (top_reg5.bit.SIFCH7 == SIFCH_ENUM_SIF) {
					pr_err("SPI4_1 conflict with SIF7\r\n");
					return E_OBJ;
				} else if (top_reg4.bit.SHUTTER_SEL == SHUTTER_ENUM_SHUTTER) {
					pr_err("SPI4_1 conflict with SHUTTER\r\n");
					return E_OBJ;
				}

				if (config & PIN_SPI_CFG_CH4_2BITS) {
					if (top_reg5.bit.SIFCH6 == SIFCH_ENUM_SIF) {
						pr_err("SPI4_1 conflict with SIF6\r\n");
						return E_OBJ;
					} else if (top_reg4.bit.STROBE_SEL == STROBE_ENUM_STROBE) {
						pr_err("SPI4_1 conflict with STROBE\r\n");
						return E_OBJ;
					}
				}

				if (config & PIN_SPI_CFG_CH4_2BITS) {
					top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
					top_reg5.bit.SPI4_DAT = SPI_DAT_ENUM_2Bit;
				}

				top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SPI4 = SPI4_1ST_PINMUX;
			}
		}
	}

	return E_OK;
}

static int pinmux_config_remote(uint32_t config)
{
	if (config == PIN_REMOTE_CFG_NONE) {
	} else if (config & PIN_REMOTE_CFG_CH1) {
		if (top_reg7.bit.PI_CNT3 == PICNT_ENUM_PICNT) {
			pr_err("REMOTE conflict with to CCNT3_1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg6.bit.SP2_CLK == SP2_CLK_SEL_ENUM_SP2CLK) {
			pr_err("REMOTE conflict with to SP_CLK2_1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) {
			pr_err("REMOTE conflict with to SENSOR6 16Bits: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR7 == SENSOR7_ENUM_8BITS) {
			pr_err("REMOTE conflict with to SENSOR7: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;

		top_reg6.bit.REMOTE = REMOTE_ENUM_REMOTE;
	} else {
		pr_err("invalid config: 0x%x\r\n", config);
		return E_PAR;
	}

	return E_OK;
}

static int pinmux_config_pwm(uint32_t config)
{
	if (config & PIN_PWM_CFG_PWM0) {
		if (top_reg4.bit.MES_SH0_SEL == MES_SEL_ENUM_1ST_PINMUX) {
			pr_err("PWM0 conflict with to MES_SHUT0: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS6_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM0 conflict with to XVS_XHS6: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM0 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM1) {
		if (top_reg4.bit.MES_SH1_SEL == MES_SEL_ENUM_1ST_PINMUX) {
			pr_err("PWM1 conflict with to MES_SHUT1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS6_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM1 conflict with to XVS_XHS6: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM1 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM2) {
		if (top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_1ST_PINMUX) {
			pr_err("PWM2 conflict with to MES2_SHUT0: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
			pr_err("PWM2 conflict with to SENSOR6: 0x%x\r\n", config);
			return E_OBJ;
		}

		if ((top_reg4.bit.XVS_XHS3_EN == VD_HD_SEL_ENUM_SIE_VDHD)&&(top_reg4.bit.XVS_XHS3_SEL == XVS_XHS_2ND_PINMUX)) {
			pr_err("PWM1 conflict with to XVS_XHS3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM2 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM3) {
		if (top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_1ST_PINMUX) {
			pr_err("PWM3 conflict with to MES2_SHUT1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg5.bit.SPI3_RDY == SPI_RDY_2ND_PINMUX) {
			pr_err("PWM3 conflict with to SPI3_2_RDY: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
			pr_err("PWM3 conflict with to SENSOR6: 0x%x\r\n", config);
			return E_OBJ;
		}

		if ((top_reg4.bit.XVS_XHS3_EN == VD_HD_SEL_ENUM_SIE_VDHD)&&(top_reg4.bit.XVS_XHS3_SEL == XVS_XHS_2ND_PINMUX)) {
			pr_err("PWM3 conflict with to XVS_XHS3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM3 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM4) {
		if (top_reg9.bit.UART4 == UART4_ENUM_1ST_PINMUX) {
			pr_err("PWM4 conflict with to UART4_1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
			pr_err("PWM4 conflict with to SENSOR6: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS4_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM4 conflict with to XVS_XHS4: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM4 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM5) {
		if (top_reg9.bit.UART4 == UART4_ENUM_1ST_PINMUX) {
			pr_err("PWM5 conflict with to UART4_1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
			pr_err("PWM5 conflict with to SENSOR6: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS4_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM5 conflict with to XVS_XHS4: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM5 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM6) {
		if ((top_reg9.bit.UART4 == UART4_ENUM_1ST_PINMUX)  && (top_reg9.bit.UART4_CTSRTS == UART_CTSRTS_PINMUX)) {
			pr_err("PWM6 conflict with to UART4_1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
			pr_err("PWM6 conflict with to SENSOR6: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS5_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM6 conflict with to XVS_XHS5: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM6 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM7) {
		if ((top_reg9.bit.UART4 == UART4_ENUM_1ST_PINMUX) && (top_reg9.bit.UART4_CTSRTS == UART_CTSRTS_PINMUX)) {
			pr_err("PWM7 conflict with to UART4_1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
			pr_err("PWM7 conflict with to SENSOR6: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS5_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM7 conflict with to XVS_XHS5: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM7 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM8) {
		if (top_reg5.bit.SPI3 == SPI3_2ND_PINMUX) {
			pr_err("PWM8 conflict with to SPI3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg9.bit.UART3 == UART3_ENUM_2ND_PINMUX) {
			pr_err("PWM8 conflict with to UART3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg10.bit.I2C2_2 == I2C_ENUM_I2C) {
			pr_err("PWM8 conflict with I2C2_2\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF_2ND_PINMUX) {
			pr_err("PWM8 conflict with to SIFCH2_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
			pr_err("PWM8 conflict with to SENSOR6: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM8 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM9) {
		if (top_reg5.bit.SPI3 == SPI3_2ND_PINMUX) {
			pr_err("PWM9 conflict with to SPI3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg9.bit.UART3 == UART3_ENUM_2ND_PINMUX) {
			pr_err("PWM9 conflict with to UART3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg10.bit.I2C2_2 == I2C_ENUM_I2C) {
			pr_err("PWM9 conflict with I2C2_2\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF_2ND_PINMUX) {
			pr_err("PWM9 conflict with to SIFCH3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
			pr_err("PWM9 conflict with to SENSOR6: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM9 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM10) {
		if (top_reg5.bit.SPI3 == SPI3_2ND_PINMUX) {
			pr_err("PWM10 conflict with to SPI3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if ((top_reg9.bit.UART3 == UART3_ENUM_2ND_PINMUX) && (top_reg9.bit.UART3_CTSRTS == UART_CTSRTS_PINMUX)) {
			pr_err("PWM10 conflict with to UART3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg10.bit.I2C3_2 == I2C_ENUM_I2C) {
			pr_err("PWM10 conflict with I2C3_2\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF_2ND_PINMUX) {
			pr_err("PWM10 conflict with to SIFCH3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF_2ND_PINMUX) {
			pr_err("PWM10 conflict with to SIFCH2_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 != SENSOR6_ENUM_GPIO) {
			pr_err("PWM10 conflict with to SENSOR6: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM10 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM11) {
		if ((top_reg5.bit.SPI3 == SPI3_2ND_PINMUX) && (top_reg5.bit.SPI3_DAT == SPI_DAT_ENUM_2Bit)) {
			pr_err("PWM11 conflict with to SPI3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if ((top_reg9.bit.UART3 == UART3_ENUM_2ND_PINMUX) && (top_reg9.bit.UART3_CTSRTS == UART_CTSRTS_PINMUX)) {
			pr_err("PWM11 conflict with to UART3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF_2ND_PINMUX) {
			pr_err("PWM11 conflict with to SIFCH3_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF_2ND_PINMUX) {
			pr_err("PWM11 conflict with to SIFCH2_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
			pr_err("PWM11 conflict with to SENSOR7: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM11 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM12) {
		if (top_reg5.bit.SPI2 == SPI2_2ND_PINMUX) {
			pr_err("PWM12 conflict with to SPI2_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS7_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM12 conflict with to XVS_XHS7: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;

		top_reg7.bit.PWM12 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM13) {
		if (top_reg5.bit.SPI2 == SPI2_2ND_PINMUX) {
			pr_err("PWM13 conflict with to SPI2_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS7_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM13 conflict with to XVS_XHS7: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;

		top_reg7.bit.PWM13 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM14) {
		if (top_reg5.bit.SPI2 == SPI2_2ND_PINMUX) {
			pr_err("PWM14 conflict with to SPI2_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
			pr_err("PWM14 conflict with to SENSOR7: 0x%x\r\n", config);
			return E_OBJ;
		}

		if ((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1)) {
			pr_err("PWM14 conflict with to SENSOR6 16 Bits: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS8_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM14 conflict with to XVS_XHS8: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM14 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM15) {
		if (top_reg5.bit.SPI2 == SPI2_2ND_PINMUX) {
			pr_err("PWM15 conflict with to SPI2_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
			pr_err("PWM15 conflict with to SENSOR7: 0x%x\r\n", config);
			return E_OBJ;
		}

		if ((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1)) {
			pr_err("PWM15 conflict with to SENSOR6 16 Bits: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg4.bit.XVS_XHS8_EN == VD_HD_SEL_ENUM_SIE_VDHD) {
			pr_err("PWM15 conflict with to XVS_XHS8: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM15 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM16) {
		if (top_reg4.bit.MES_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX) {
			pr_err("PWM16 conflict with to MES_2_SHUT0: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg5.bit.SPI == SPI_2ND_PINMUX) {
			pr_err("PWM16 conflict with to SPI_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg10.bit.I2C4 == I2C_ENUM_I2C) {
			pr_err("PWM16 conflict with I2C4\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_2ND_PINMUX) {
			pr_err("PWM16 conflict with SIFCH0_2\r\n");
			return E_OBJ;
		}

			if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
			pr_err("PWM16 conflict with to SENSOR7: 0x%x\r\n", config);
			return E_OBJ;
		}

		if ((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1)) {
			pr_err("PWM16 conflict with to SENSOR6 16 Bits: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM16 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM17) {
		if (top_reg4.bit.MES_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX) {
			pr_err("PWM17 conflict with to MES_2_SHUT1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg5.bit.SPI == SPI_2ND_PINMUX) {
			pr_err("PWM17 conflict with to SPI_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg10.bit.I2C4 == I2C_ENUM_I2C) {
			pr_err("PWM17 conflict with I2C4\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_3RD_PINMUX) {
			pr_err("PWM17 conflict with SIFCH0_3\r\n");
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
			pr_err("PWM17 conflict with to SENSOR7: 0x%x\r\n", config);
			return E_OBJ;
		}

		if ((top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) && (ui_sie6_16_12 != 1)) {
			pr_err("PWM17 conflict with to SENSOR6 16 Bits: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM17 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM18) {
		if (top_reg4.bit.MES2_SH0_SEL == MES_SEL_ENUM_2ND_PINMUX) {
			pr_err("PWM18 conflict with to MES2_2_SHUT0: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg5.bit.SPI == SPI_2ND_PINMUX) {
			pr_err("PWM18 conflict with to SPI_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg10.bit.I2C4_3 == I2C_ENUM_I2C) {
			pr_err("PWM18 conflict with I2C4_3\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_2ND_PINMUX) {
			pr_err("PWM18 conflict with SIFCH0_3\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_3RD_PINMUX) {
			pr_err("PWM18 conflict with SIFCH0_3\r\n");
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
			pr_err("PWM18 conflict with to SENSOR7: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) {
			pr_err("PWM18 conflict with to SENSOR6 16 Bits: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM18 = PWM_ENUM_PWM;
	}

	if (config & PIN_PWM_CFG_PWM19) {
		if (top_reg4.bit.MES2_SH1_SEL == MES_SEL_ENUM_2ND_PINMUX) {
			pr_err("PWM19 conflict with to MES2_2_SHUT1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg5.bit.SPI == SPI_2ND_PINMUX) {
			pr_err("PWM19 conflict with to SPI_2: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg10.bit.I2C4_3 == I2C_ENUM_I2C) {
			pr_err("PWM19 conflict with I2C4_3\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_2ND_PINMUX) {
			pr_err("PWM19 conflict with SIFCH0_3\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF_3RD_PINMUX) {
			pr_err("PWM19 conflict with SIFCH0_3\r\n");
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
			pr_err("PWM19 conflict with to SENSOR7: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) {
			pr_err("PWM19 conflict with to SENSOR6 16 Bits: 0x%x\r\n", config);
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
		top_reg7.bit.PWM19 = PWM_ENUM_PWM;
	}

	if (config & (PIN_PWM_CFG_CCNT|PIN_PWM_CFG_CCNT_2ND)) {
		if (config & PIN_PWM_CFG_CCNT_2ND) {
			if (top_reg3.bit.SENSOR8 != SENSOR8_ENUM_GPIO) {
				pr_err("PICNT conflict with to SENSOR8: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
				pr_err("PICNT conflict with ETH\r\n");
				return E_OBJ;
			}

			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.PI_CNT = PICNT_ENUM_PICNT2;
		} else {
			if (top_reg5.bit.SPI4 == SPI4_2ND_PINMUX) {
				pr_err("PICNT conflict with to SPI4_2: 0x%x\r\n", config);
				return E_OBJ;
			}

			if ((top_reg9.bit.UART2 == UART2_ENUM_1ST_PINMUX) && (top_reg9.bit.UART2_CTSRTS == UART_CTSRTS_PINMUX)) {
				pr_err("PICNT conflict with UART2_1\r\n");
				return E_OBJ;
			}

			if (top_reg6.bit.AUDIO == AUDIO_ENUM_I2S) {
				pr_err("PICNT conflict with AUDIO\r\n");
				return E_OBJ;
			}

			top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.PI_CNT = PICNT_ENUM_PICNT;
		}
	}

	if (config & (PIN_PWM_CFG_CCNT2|PIN_PWM_CFG_CCNT2_2ND)) {
		if (config & PIN_PWM_CFG_CCNT2_2ND) {
			if (top_reg3.bit.SENSOR8 != SENSOR8_ENUM_GPIO) {
				pr_err("PICNT2 conflict with to SENSOR8: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
				pr_err("PICNT2 conflict with ETH\r\n");
				return E_OBJ;
			}

			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.PI_CNT2 = PICNT_ENUM_PICNT2;
		} else {
			if ((top_reg5.bit.SPI4 == SPI4_2ND_PINMUX) && (top_reg5.bit.SPI4_DAT == SPI_DAT_ENUM_2Bit)) {
				pr_err("PICNT2 conflict with SPI5_2\r\n");
				return E_OBJ;
			}

			if ((top_reg9.bit.UART2 == UART2_ENUM_1ST_PINMUX) && (top_reg9.bit.UART2_CTSRTS == UART_CTSRTS_PINMUX)) {
				pr_err("PICNT2 conflict with UART2_1\r\n");
				return E_OBJ;
			}

			if (top_reg6.bit.AUDIO == AUDIO_ENUM_I2S) {
				pr_err("PICNT2 conflict with AUDIO\r\n");
				return E_OBJ;
			}

			top_reg_pgpio1.bit.PGPIO_34 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.PI_CNT2 = PICNT_ENUM_PICNT;
		}
	}

	if (config & (PIN_PWM_CFG_CCNT3|PIN_PWM_CFG_CCNT3_2ND)) {
		if (config & PIN_PWM_CFG_CCNT3_2ND) {
			if (top_reg3.bit.SENSOR8 != SENSOR8_ENUM_GPIO) {
				pr_err("PICNT3 conflict with to SENSOR8: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
				pr_err("PICNT3 conflict with ETH\r\n");
				return E_OBJ;
			}

			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.PI_CNT3 = PICNT_ENUM_PICNT2;
		} else {
			if (top_reg6.bit.REMOTE == REMOTE_ENUM_REMOTE) {
				pr_err("PICNT3 conflict with to REMOTE: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg3.bit.SENSOR7 != SENSOR7_ENUM_GPIO) {
				pr_err("PICNT3 conflict with to SENSOR7: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg3.bit.SENSOR6 == SENSOR6_ENUM_16BITS) {
				pr_err("PICNT3 conflict with to SENSOR6 16 Bits: 0x%x\r\n", config);
				return E_OBJ;
			}
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.PI_CNT3 = PICNT_ENUM_PICNT;
		}
	}

	if (config & (PIN_PWM_CFG_CCNT4|PIN_PWM_CFG_CCNT4_2ND)) {
		if (config & PIN_PWM_CFG_CCNT4_2ND) {
			if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF) {
				pr_err("PICNT4 conflict with to SIFCH0_1: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg5.bit.SPI4 == SPI4_3RD_PINMUX) {
				pr_err("PICNT4 conflict with to SPI4_3: 0x%x\r\n", config);
				return E_OBJ;
			}

			if (top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
				pr_err("PICNT4 conflict with ETH\r\n");
				return E_OBJ;
			}


			top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.PI_CNT4 = PICNT_ENUM_PICNT2;
		} else {
			if (top_reg6.bit.SP_CLK == SP_CLK_SEL_ENUM_SPCLK) {
				pr_err("PICNT4 conflict with to SP_CLK: 0x%x\r\n", config);
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.PI_CNT4 = PICNT_ENUM_PICNT;
		}
	}

	return E_OK;
}

static int pinmux_config_audio(uint32_t config)
{
	if (config & (PIN_AUDIO_CFG_I2S)) {
		if (top_reg5.bit.SPI4 == SPI4_2ND_PINMUX) {
			pr_err("AUDIO conflict with SPI4_2\r\n");
			return E_OBJ;
		}

		if (top_reg9.bit.UART2 == UART2_ENUM_1ST_PINMUX) {
			pr_err("AUDIO conflict with UART2_1\r\n");
			return E_OBJ;
		}

		if (top_reg7.bit.PI_CNT == PICNT_ENUM_PICNT) {
			pr_err("AUDIO conflict with PICNT_1\r\n");
			return E_OBJ;
		}

		if (top_reg7.bit.PI_CNT2 == PICNT_ENUM_PICNT) {
			pr_err("AUDIO conflict with PICNT2_1\r\n");
			return E_OBJ;
		}

		top_reg_pgpio1.bit.PGPIO_34 = GPIO_ID_EMUM_FUNC;
		top_reg_pgpio1.bit.PGPIO_35 = GPIO_ID_EMUM_FUNC;
		top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
		top_reg_pgpio1.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC;
		top_reg6.bit.AUDIO = AUDIO_ENUM_I2S;
	}

	if (config & (PIN_AUDIO_CFG_MCLK)) {
		if (top_reg5.bit.SPI2 == SPI2_1ST_PINMUX) {
			pr_err("AUDIO MCLK conflict with SPI2_1\r\n");
			return E_OBJ;
		}

		if (top_reg9.bit.UART3 == UART3_ENUM_1ST_PINMUX) {
			pr_err("AUDIO MCLK conflict with UART3_1\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF) {
			pr_err("AUDIO MCLK conflict with to SIFCH2_1: 0x%x\r\n", config);
			return E_OBJ;
		}

		if (top_reg6.bit.SP_CLK == SP_CLK_SEL_ENUM_SP_2_CLK) {
			pr_err("AUDIO MCLK conflict with SP_CLK_2\r\n");
			return E_OBJ;
		}

		if (top_reg4.bit.LOCKN == LOCKN_ENUM_LOCKN) {
			pr_err("AUDIO MCLK conflict with LOCKN\r\n");
			return E_OBJ;
		}

		top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
		top_reg6.bit.AUDIO_MCLK = AUDIO_MCLK_MCLK;
	}

	return E_OK;
}

static int pinmux_config_lcd(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_DISPMUX_SEL_MASK;
	if (tmp == PINMUX_DISPMUX_SEL_MASK) {
		pr_err("invalid locate: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_LCD] = config;

	return E_OK;
}

static int pinmux_config_lcd2(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_DISPMUX_SEL_MASK;
	if (tmp == PINMUX_DISPMUX_SEL_MASK) {
		pr_err("invalid locate: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_LCD2] = config;

	return E_OK;
}

static int pinmux_config_tv(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_TV_HDMI_CFG_MASK;
	if ((tmp != PINMUX_TV_HDMI_CFG_NORMAL) && (tmp != PINMUX_TV_HDMI_CFG_PINMUX_ON)) {
		pr_err("invalid config: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_TV] = config;

	return E_OK;
}

static int pinmux_config_hdmi(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_TV_HDMI_CFG_MASK;
	if ((tmp != PINMUX_TV_HDMI_CFG_NORMAL) && (tmp != PINMUX_TV_HDMI_CFG_PINMUX_ON)) {
		pr_err("invalid config: 0x%x\r\n", config);
		return E_PAR;
	}

	if (config & PINMUX_HDMI_CFG_HOTPLUG) {
		top_reg_pgpio1.bit.PGPIO_47 = GPIO_ID_EMUM_FUNC;
	}

	if (config & PINMUX_HDMI_CFG_CEC) {
		top_reg_pgpio1.bit.PGPIO_44 = GPIO_ID_EMUM_FUNC;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_HDMI] = config;

	return E_OK;
}

static int pinmux_config_eth(uint32_t config)
{
	if (config == PIN_ETH_CFG_NONE) {
	} else if (config & (PIN_ETH_CFG_MII|PIN_ETH_CFG_RMII|PIN_ETH_CFG_GMII|PIN_ETH_CFG_RGMII|PIN_ETH_CFG_REVMII_10_100|PIN_ETH_CFG_REVMII_10_1000|PIN_ETH_CFG_MII_NO_TXER)) {
		if (top_reg5.bit.SIFCH0 == SIFCH_ENUM_SIF) {
			pr_err("ETH conflict with SIF0\r\n");
			return E_OBJ;
		}

		if (top_reg10.bit.I2C4_4 == I2C_ENUM_I2C) {
			pr_err("ETH conflict with I2C4_4\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SPI4 == SPI4_3RD_PINMUX) {
			pr_err("ETH conflict with SPI4_3\r\n");
			return E_OBJ;
		}

		top_reg_lgpio0.bit.LGPIO_28 = GPIO_ID_EMUM_FUNC;
		top_reg_lgpio0.bit.LGPIO_29 = GPIO_ID_EMUM_FUNC;

		if (config & (PIN_ETH_CFG_GMII|PIN_ETH_CFG_RMII|PIN_ETH_CFG_RGMII|PIN_ETH_CFG_REVMII_10_1000)) {
			top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
		}

		if (config & (PIN_ETH_CFG_MII|PIN_ETH_CFG_MII_NO_TXER)) {
			if (config & PIN_ETH_CFG_MII) {
				top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			}

			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.ETH = ETH_ID_ENUM_MII;
		} else if (config & PIN_ETH_CFG_RMII) {
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.ETH = ETH_ID_ENUM_RMII;
		} else if (config & PIN_ETH_CFG_GMII) {
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.ETH = ETH_ID_ENUM_GMII;
		} else if (config & PIN_ETH_CFG_RGMII) {
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.ETH = ETH_ID_ENUM_RGMII;
		} else if (config & PIN_ETH_CFG_REVMII_10_100) {
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.ETH = ETH_ID_ENUM_REVMII_10_100;
		} else if (config & PIN_ETH_CFG_REVMII_10_1000) {
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.ETH = ETH_ID_ENUM_REVMII_10_1000;
		}
	}
	return E_OK;
}

static int pinmux_config_canbus(uint32_t config)
{

	if (config & (PIN_CANBUS_CFG_CANBUS)) {
		if (top_reg5.bit.SPI2 == SPI2_1ST_PINMUX) {
			pr_err("CANBUS conflict with SPI2_1\r\n");
			return E_OBJ;
		}

		if (top_reg9.bit.UART3 == UART3_ENUM_1ST_PINMUX) {
			pr_err("CANBUS conflict with UART3 1st\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH2 == SIFCH_ENUM_SIF) {
			pr_err("CANBUS conflict with SIF2_1\r\n");
			return E_OBJ;
		}

		if (top_reg5.bit.SIFCH3 == SIFCH_ENUM_SIF) {
			pr_err("CANBUS conflict with SIF3_1\r\n");
			return E_OBJ;
		}

		if (top_reg6.bit.SP2_CLK == SP2_CLK_SEL_ENUM_SP2_2_CLK) {
			pr_err("CANBUS conflict with SP2_2_CLK\r\n");
			return E_OBJ;
		}

		if (top_reg2.bit.TV_TEST_CLK == TV_TEST_CLK_TV_CLK) {
			pr_err("CANBUS conflict with TV_TEST_CLK\r\n");
			return E_OBJ;
		}

		if (top_reg4.bit.LOCKN2 == LOCKN_ENUM_LOCKN) {
			pr_err("CANBUS conflict with LOCKN2\r\n");
			return E_OBJ;
		}

		top_reg6.bit.CANBUS = CANBUS_ENUM_CANBUS;
		top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
		top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
		top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;
	}
	return E_OK;
}

/*
	Set Memory Interface Display pinmux

	Select Memory Interface Display

	@param[in] config   pinmux config (from configuration table)
	@param[in] pinmux   pinmux type

	@return
		- @b E_OK: success
		- @b E_PAR: invalid pinmux
		- @b E_OBJ: pinmux conflict
*/
static ER pinmux_select_mi_display(struct nvt_pinctrl_info *info, u32 config, u32 pinmux)
{
	u32 pinmux_type;
	union TOP_REG2 local_top_reg2;
	union TOP_REG3 local_top_reg3;
	union TOP_REG6 local_top_reg6;
	union TOP_REG7 local_top_reg7;
	union TOP_REG11 local_top_reg11;
	union TOP_REGLGPIO0 local_top_reg_lgpio0;

	local_top_reg2.reg = TOP_GETREG(info, TOP_REG2_OFS);
	local_top_reg3.reg = TOP_GETREG(info, TOP_REG3_OFS);
	local_top_reg6.reg = TOP_GETREG(info, TOP_REG6_OFS);
	local_top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);
	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);
	local_top_reg_lgpio0.reg = TOP_GETREG(info, TOP_REGLGPIO0_OFS);

	pinmux_type = pinmux & ~(PINMUX_LCD_SEL_FEATURE_MSK | PINMUX_PMI_CFG_MASK);

	/*1. check pinmux conflict*/
	switch (pinmux_type) {
	case PINMUX_LCD_SEL_GPIO:
		break;
	case PINMUX_LCD_SEL_PARALLE_MI_8BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_9BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_16BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_18BITS:
		if (config & PINMUX_PMI_CFG_2ND_PINMUX) {
			if ((pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_16BITS) ||
				(pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_18BITS)) {
				pr_err("PMI_2 can only up to 9 bits: 0x%x\r\n", pinmux);
				return E_PAR;
			}

			/*2nd PMI pinmux, can up to 9 bits*/
			if (local_top_reg2.bit.CCIR_HVLD_VVLD == CCIR_HVLD_VVLD_EN) {
				pr_err("PMI2 conflict with HVLD: 0x%x\r\n", pinmux);
				return E_OBJ;
			}
			if ((local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_CCIR601) ||
				((local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_CCIR656) && (local_top_reg2.bit.CCIR_DATA_WIDTH == CCIR_DATA_WIDTH_16BITS)) ||
				(local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_RGB_16BITS) ||
				(local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_PARALLEL_LCD)) {
				pr_err("PMI2 conflict with LCD CCIR601, CCIR656 16 bits, RGB 16 bits, RGB888/666/565: 0x%x\r\n", pinmux);
				return E_OBJ;
			}
			if (local_top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
				pr_err("PMI2 conflict with ETH\r\n");
				return E_OBJ;
			}
			if ((local_top_reg11.bit.TRACE_EN == TRACE_ENUM_TRACE)) {
				pr_err("PMI2 conflict with TRACE\r\n");
				return E_OBJ;
			}
			if (local_top_reg3.bit.SENSOR8 != SENSOR8_ENUM_GPIO) {
				pr_err("PMI2 conflict with to SENSOR8: 0x%x\r\n", config);
				return E_OBJ;
			}
			if (local_top_reg7.bit.PI_CNT == PICNT_ENUM_PICNT2) {
				pr_err("PMI2 conflict with PICNT_2\r\n");
				return E_OBJ;
			}
			if (local_top_reg7.bit.PI_CNT2 == PICNT_ENUM_PICNT2) {
				pr_err("PMI2 conflict with PICNT2_2\r\n");
				return E_OBJ;
			}
			if (local_top_reg7.bit.PI_CNT3 == PICNT_ENUM_PICNT2) {
				pr_err("PMI2 conflict with PICNT3_2\r\n");
				return E_OBJ;
			}
			local_top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			if (PINMUX_LCD_SEL_PARALLE_MI_9BITS)
				local_top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;

			local_top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;

		} else {
			/*1st PMI pinmux, can up to 18 bits*/
			if (local_top_reg2.bit.LCD_TYPE != LCDTYPE_ENUM_GPIO) {
				pr_err("PMI conflict with LCD: 0x%x\r\n", pinmux);
				return E_OBJ;
			}
			if (local_top_reg2.bit.CCIR_HVLD_VVLD == CCIR_HVLD_VVLD_EN) {
				pr_err("PMI conflict with HVLD: 0x%x\r\n", pinmux);
				return E_OBJ;
			}
			if ((local_top_reg2.bit.LCD2_TYPE == LCDTYPE_ENUM_CCIR656) ||
				(local_top_reg2.bit.LCD2_TYPE == LCDTYPE_ENUM_SerialRGB_8BITS) ||
				(local_top_reg2.bit.LCD2_TYPE == LCDTYPE_ENUM_SerialYCbCr_8BITS)) {
				pr_err("PMI conflict with LCD2 CCIR656, Serial RGB/YCbCr: 0x%x\r\n", pinmux);
				return E_OBJ;
			}
			if (local_top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
				pr_err("PMI conflict with ETH\r\n");
				return E_OBJ;
			}
			if ((local_top_reg11.bit.TRACE_EN == TRACE_ENUM_TRACE)) {
				pr_err("PMI conflict with TRACE\r\n");
				return E_OBJ;
			}
			if (local_top_reg3.bit.SENSOR8 != SENSOR8_ENUM_GPIO) {
				pr_err("PMI conflict with to SENSOR8: 0x%x\r\n", config);
				return E_OBJ;
			}

			if ((pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_16BITS) ||
				(pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_18BITS)) {
				if (local_top_reg2.bit.LCD2_TYPE != LCDTYPE_ENUM_GPIO) {
					pr_err("PMI 16/18 conflict with LCD2: 0x%x\r\n", pinmux);
					return E_OBJ;
				}
				if (local_top_reg7.bit.PI_CNT == PICNT_ENUM_PICNT2) {
					pr_err("PMI conflict with PICNT_2\r\n");
					return E_OBJ;
				}
				if (local_top_reg7.bit.PI_CNT2 == PICNT_ENUM_PICNT2) {
					pr_err("PMI conflict with PICNT2_2\r\n");
					return E_OBJ;
				}
				if (local_top_reg7.bit.PI_CNT3 == PICNT_ENUM_PICNT2) {
					pr_err("PMI conflict with PICNT3_2\r\n");
					return E_OBJ;
				}
				local_top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
				if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_18BITS) {
					local_top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
					local_top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
				}
			}

			local_top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;

		}
		break;
	case PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO:
		if (local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_CCIR601) {
			pr_err("SMI 2 bits conflict with LCD CCIR601: 0x%x\r\n", pinmux);
			return E_OBJ;
		}
		if (((local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_CCIR601) && (local_top_reg2.bit.CCIR_DATA_WIDTH == CCIR_DATA_WIDTH_16BITS)) ||
			((local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_CCIR656) && (local_top_reg2.bit.CCIR_DATA_WIDTH == CCIR_DATA_WIDTH_16BITS)) ||
			(local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_RGB_16BITS) ||
			(local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_PARALLEL_LCD)) {
			pr_err("SMI conflict with LCD CCIR601/656 16 bits, RGB 16 bits, RGB888/666/565: 0x%x\r\n", pinmux);
			return E_OBJ;
		}
		if (local_top_reg2.bit.LCD2_TYPE != LCDTYPE_ENUM_GPIO) {
			pr_err("SMI conflict with LCD2: 0x%x\r\n", pinmux);
			return E_OBJ;
		}
		local_top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
		break;
	case PINMUX_LCD_SEL_SERIAL_MI_SDIO:
		if (((local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_CCIR601) && (local_top_reg2.bit.CCIR_DATA_WIDTH == CCIR_DATA_WIDTH_16BITS)) ||
			((local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_CCIR656) && (local_top_reg2.bit.CCIR_DATA_WIDTH == CCIR_DATA_WIDTH_16BITS)) ||
			(local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_RGB_16BITS) ||
			(local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_PARALLEL_LCD)) {
			pr_err("SMI conflict with LCD CCIR601/656 16 bits, RGB 16 bits, RGB888/666/565: 0x%x\r\n", pinmux);
			return E_OBJ;
		}
		if (local_top_reg2.bit.LCD2_TYPE != LCDTYPE_ENUM_GPIO) {
			pr_err("SMI conflict with LCD2: 0x%x\r\n", pinmux);
			return E_OBJ;
		}
		local_top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;

		break;
	default:
		pr_err("invalid MI config: 0x%x\r\n", pinmux);
		return E_PAR;
	}

	/*2. setup pinmux registers*/
	local_top_reg2.bit.MEMIF_TYPE = MEMIF_TYPE_GPIO;
	switch (pinmux_type) {
	case PINMUX_LCD_SEL_GPIO:
		break;
	case PINMUX_LCD_SEL_PARALLE_MI_8BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_9BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_16BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_18BITS:
		if (config & PINMUX_PMI_CFG_2ND_PINMUX) {
			if ((pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_16BITS) ||
				(pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_18BITS)) {
				pr_err("2nd PMI can only up to 9 bits: 0x%x\r\n", pinmux);
				return E_PAR;
			}

			/*2nd PMI pinmux, can up to 9 bits*/
			local_top_reg2.bit.PMEM_SEL = PMEM_SEL_ENUM_2ND_PINMUX;
		} else {
			/*1st PMI pinmux, can up to 18 bits*/
			local_top_reg2.bit.PMEM_SEL = PMEM_SEL_ENUM_1ST_PINMUX;
		}

		if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_8BITS)
			local_top_reg2.bit.PMEMIF_DATA_WIDTH = PMEMIF_DATA_WIDTH_8BITS;
		else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_9BITS)
			local_top_reg2.bit.PMEMIF_DATA_WIDTH = PMEMIF_DATA_WIDTH_9BITS;
		else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_16BITS)
			local_top_reg2.bit.PMEMIF_DATA_WIDTH = PMEMIF_DATA_WIDTH_16BITS;
		else
			local_top_reg2.bit.PMEMIF_DATA_WIDTH = PMEMIF_DATA_WIDTH_18BITS;

		local_top_reg2.bit.MEMIF_TYPE = MEMIF_TYPE_PARALLEL;
		break;
	case PINMUX_LCD_SEL_SERIAL_MI_SDIO:
	case PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO:
		if (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDIO)
			local_top_reg2.bit.SMEMF_DATA_WIDTH = SMEMIF_DATA_WIDTH_1BITS;
		else
			local_top_reg2.bit.SMEMF_DATA_WIDTH = SMEMIF_DATA_WIDTH_2BITS;

		local_top_reg2.bit.MEMIF_TYPE = MEMIF_TYPE_SERIAL;
		break;
	default:
		break;
	}
	if (pinmux & PINMUX_LCD_SEL_TE_ENABLE) {
		if (config & PINMUX_PMI_CFG_2ND_PINMUX)
			local_top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
		else {
			if ((pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDIO) || (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO))
				local_top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			else
				local_top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
		}

		local_top_reg2.bit.TE_SEL = TE_SEL_EN;
	}

	/*Enter critical section*/
	down(&top_sem);

	TOP_SETREG(info, TOP_REG2_OFS, local_top_reg2.reg);
	TOP_SETREG(info, TOP_REGLGPIO0_OFS, local_top_reg_lgpio0.reg);

	/*Leave critical section*/
	up(&top_sem);

	return E_OK;
}

/*
	Set Primary LCD pinmux

	Select LCD pinmux with IDE1 controller

	@param[in] config   pinmux config (from configuration table)
	@param[in] pinmux   pinmux type

	@return
		- @b E_OK: success
		- @b E_PAR: invalid pinmux
		- @b E_OBJ: pinmux conflict
*/
static ER pinmux_select_primary_lcd(struct nvt_pinctrl_info *info, u32 config, u32 pinmux)
{
	u32 pinmux_type;
	union TOP_REG2 local_top_reg2;
	union TOP_REG6 local_top_reg6;
	union TOP_REGLGPIO0 local_top_reg_lgpio0;

	local_top_reg2.reg = TOP_GETREG(info, TOP_REG2_OFS);
	local_top_reg6.reg = TOP_GETREG(info, TOP_REG6_OFS);
	local_top_reg_lgpio0.reg = TOP_GETREG(info, TOP_REGLGPIO0_OFS);

	pinmux_type = pinmux & ~(PINMUX_LCD_SEL_FEATURE_MSK | PINMUX_PMI_CFG_MASK);

	/*1. check pinmux conflict */
	switch (pinmux_type) {
	case PINMUX_LCD_SEL_GPIO:
		break;
	case PINMUX_LCD_SEL_CCIR656_16BITS:
		/*CCIR656 16bits conflict with: all LCD2, all MI*/
		if ((local_top_reg2.bit.LCD2_TYPE != LCDTYPE_ENUM_GPIO) ||
			(local_top_reg2.bit.MEMIF_TYPE != MEMIF_TYPE_GPIO)) {
			pr_err("LCD CCIR656 16 bitsconflict with LCD2/MI\r\n");
			return E_OBJ;
		}
		if (local_top_reg6.bit.ETH == ETH_ID_ENUM_GMII) {
			pr_err("LCD CCIR656 16 bits conflict with GMII\r\n");
			return E_OBJ;
		}
		break;
	case PINMUX_LCD_SEL_CCIR656:
	case PINMUX_LCD_SEL_SERIAL_RGB_8BITS:
	case PINMUX_LCD_SEL_SERIAL_RGB_6BITS:
	case PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS:
		/*CCIR656, Serial RGB666/888, Serial YCbCr conflict with: primary parallel MI*/
		if ((local_top_reg2.bit.MEMIF_TYPE == MEMIF_TYPE_PARALLEL) &&
			(local_top_reg2.bit.PMEM_SEL == PMEM_SEL_ENUM_1ST_PINMUX)) {
			pr_err("LCD CCIR656 or srial RGB/YCbCr conflict with PMI\r\n");
			return E_OBJ;
		}
		if (local_top_reg6.bit.ETH == ETH_ID_ENUM_GMII) {
			pr_err("LCD CCIR656 or srial RGB/YCbCr conflict with GMII\r\n");
			return E_OBJ;
		}
		break;
	case PINMUX_LCD_SEL_CCIR601_16BITS:
		/*CCIR601 16 bits conflict with: all LCD2, all MI*/
		if ((local_top_reg2.bit.LCD2_TYPE != LCDTYPE_ENUM_GPIO) ||
			(local_top_reg2.bit.MEMIF_TYPE != MEMIF_TYPE_GPIO)) {
			pr_err("LCD CCIR601 16 bitsconflict with LCD2/MI\r\n");
			return E_OBJ;
		}
		if (local_top_reg6.bit.ETH == ETH_ID_ENUM_GMII) {
			pr_err("LCD CCIR601 16 bits conflict with GMII\r\n");
			return E_OBJ;
		}
		break;
	case PINMUX_LCD_SEL_CCIR601:
		/* CCIR601 conflict with: primary parallel MI
		When HVLD/VVLD is active, conflict with: LCD2 CCIR656, LCD2 Serial RGB888, LCD2 Serial YCbCr, all parallel MI, and 2 bits serial MI*/
		if (pinmux & PINMUX_LCD_SEL_HVLD_VVLD) {
			if ((local_top_reg2.bit.LCD2_TYPE == LCDTYPE_ENUM_CCIR656) ||
				(local_top_reg2.bit.LCD2_TYPE == LCDTYPE_ENUM_SerialRGB_8BITS) ||
				(local_top_reg2.bit.LCD2_TYPE == LCDTYPE_ENUM_SerialYCbCr_8BITS) ||
				(local_top_reg2.bit.MEMIF_TYPE == MEMIF_TYPE_PARALLEL) ||
				((local_top_reg2.bit.MEMIF_TYPE == MEMIF_TYPE_SERIAL) && (local_top_reg2.bit.SMEMF_DATA_WIDTH == SMEMIF_DATA_WIDTH_2BITS))) {
				pr_err("LCD CCIR601 HVLD/VVLD conflict with LCD2/PMI\r\n");
				return E_OBJ;
			}
		}
		if ((local_top_reg2.bit.MEMIF_TYPE == MEMIF_TYPE_PARALLEL) &&
			(local_top_reg2.bit.PMEM_SEL == PMEM_SEL_ENUM_1ST_PINMUX)) {
			pr_err("LCD conflict with PMI\r\n");
			return E_OBJ;
		}
		if (local_top_reg6.bit.ETH == ETH_ID_ENUM_GMII) {
			pr_err("LCD conflict with GMII\r\n");
			return E_OBJ;
		}
		break;
	case PINMUX_LCD_SEL_PARALLE_RGB666:
	case PINMUX_LCD_SEL_PARALLE_RGB888:
	case PINMUX_LCD_SEL_PARALLE_RGB565:
		/*RGB888/666/565 conflict with: all LCD2, all MI*/
		if ((local_top_reg2.bit.LCD2_TYPE != LCDTYPE_ENUM_GPIO) ||
			(local_top_reg2.bit.MEMIF_TYPE != MEMIF_TYPE_GPIO)) {
			pr_err("LCD RGB565/666/888 conflict with LCD2/MI\r\n");
			return E_OBJ;
		}
		if (local_top_reg6.bit.ETH == ETH_ID_ENUM_GMII) {
			pr_err("LCD RGB565/666/888 conflict with GMII\r\n");
			return E_OBJ;
		}
		break;
	case PINMUX_LCD_SEL_RGB_16BITS:
		/*RGB 16bits conflict with: all LCD2, all MI*/
		if ((local_top_reg2.bit.LCD2_TYPE != LCDTYPE_ENUM_GPIO) ||
			(local_top_reg2.bit.MEMIF_TYPE != MEMIF_TYPE_GPIO)) {
			pr_err("LCD RGB 16 bits conflict with LCD2/MI\r\n");
			return E_OBJ;
		}
		if (local_top_reg6.bit.ETH == ETH_ID_ENUM_GMII) {
			pr_err("LCD RGB 16 bits conflict with GMII\r\n");
			return E_OBJ;
		}
		break;
	case PINMUX_LCD_SEL_PARALLE_MI_8BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_9BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_16BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_18BITS:
		if (local_top_reg6.bit.ETH == ETH_ID_ENUM_GMII) {
			pr_err("LCD parallel conflict with GMII\r\n");
			return E_OBJ;
		}
	case PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO:
	case PINMUX_LCD_SEL_SERIAL_MI_SDIO: {
			ER erReturn = pinmux_select_mi_display(info, config, pinmux);
			if (erReturn != E_OK)
				pr_err("Primary LCD configure MI fail\r\n");

			mi_display = PINMUX_DISPMUX_SEL_LCD;
			return erReturn;
		}
		break;
	case PINMUX_LCD_SEL_MIPI:
		break;
	default:
		pr_err("invalid primary LCD config: 0x%x\r\n", pinmux);
		return E_PAR;
	}

	/*2. setup pinmux registers*/
	local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_GPIO;
	local_top_reg2.bit.CCIR_HVLD_VVLD = CCIR_HVLD_VVLD_GPIO;
	local_top_reg2.bit.PLCD_DE = PLCD_DE_ENUM_GPIO;
	switch (pinmux_type) {
	case PINMUX_LCD_SEL_GPIO:
		break;
	case PINMUX_LCD_SEL_CCIR656:
	case PINMUX_LCD_SEL_CCIR656_16BITS:
		if (pinmux_type == PINMUX_LCD_SEL_CCIR656)
			local_top_reg2.bit.CCIR_DATA_WIDTH = CCIR_DATA_WIDTH_8BITS;
		else
			local_top_reg2.bit.CCIR_DATA_WIDTH = CCIR_DATA_WIDTH_16BITS;

		local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_CCIR656;
		break;
	case PINMUX_LCD_SEL_CCIR601:
		if (pinmux & PINMUX_LCD_SEL_HVLD_VVLD)
			local_top_reg2.bit.CCIR_HVLD_VVLD = CCIR_HVLD_VVLD_EN;

		if (pinmux & PINMUX_LCD_SEL_FIELD)
			local_top_reg2.bit.CCIR_FIELD = CCIR_FIELD_EN;

		local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_CCIR601;
		break;
	case PINMUX_LCD_SEL_CCIR601_16BITS:
		local_top_reg2.bit.CCIR_DATA_WIDTH = CCIR_DATA_WIDTH_16BITS;
		local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_CCIR601;
		break;
	case PINMUX_LCD_SEL_PARALLE_RGB565:
	case PINMUX_LCD_SEL_PARALLE_RGB666:
	case PINMUX_LCD_SEL_PARALLE_RGB888:
		if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB565)
			local_top_reg2.bit.PLCD_DATA_WIDTH = PLCD_DATA_WIDTH_RGB565;
		else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB666)
			local_top_reg2.bit.PLCD_DATA_WIDTH = PLCD_DATA_WIDTH_RGB666;
		else
			local_top_reg2.bit.PLCD_DATA_WIDTH = PLCD_DATA_WIDTH_RGB888;

		local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_PARALLEL_LCD;
		break;
	case PINMUX_LCD_SEL_SERIAL_RGB_8BITS:
		local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_SerialRGB_8BITS;
		break;
	case PINMUX_LCD_SEL_SERIAL_RGB_6BITS:
		local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_SerialRGB_6BITS;
		break;
	case PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS:
		local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_SerialYCbCr_8BITS;
		break;
	case PINMUX_LCD_SEL_RGB_16BITS:
		local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_RGB_16BITS;
		break;
	case PINMUX_LCD_SEL_MIPI:
		local_top_reg2.bit.LCD_TYPE = LCDTYPE_ENUM_MIPI_DSI;
		if (pinmux & PINMUX_LCD_SEL_TE_ENABLE) {
			local_top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			local_top_reg2.bit.TE_SEL = TE_SEL_EN;
		}
		break;
	default:
		break;
	}

	if (pinmux & PINMUX_LCD_SEL_DE_ENABLE) {
		local_top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
		local_top_reg2.bit.PLCD_DE = PLCD_DE_ENUM_DE;
	}

	if ((pinmux_type >= PINMUX_LCD_SEL_CCIR656) && (pinmux_type <= PINMUX_LCD_SEL_RGB_16BITS)) {
		if ((pinmux_type != PINMUX_LCD_SEL_CCIR656) && (pinmux_type != PINMUX_LCD_SEL_CCIR656_16BITS)) {
			local_top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
		}
		if (pinmux_type != PINMUX_LCD_SEL_SERIAL_RGB_6BITS) {
			local_top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
		}

		local_top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;

		if ((pinmux_type == PINMUX_LCD_SEL_CCIR656_16BITS) || (pinmux_type == PINMUX_LCD_SEL_CCIR601_16BITS) ||
			(pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB565) || (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB666) ||
			(pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB888) || (pinmux_type == PINMUX_LCD_SEL_RGB_16BITS)) {
			if ((pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB565) || (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB666) ||
				(pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB888)) {
				local_top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
				local_top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
			}

			local_top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			local_top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
		}

	}

	/*Enter critical section*/
	down(&top_sem);

	TOP_SETREG(info, TOP_REG2_OFS, local_top_reg2.reg);
	TOP_SETREG(info, TOP_REGLGPIO0_OFS, local_top_reg_lgpio0.reg);

	/*Leave critical section*/
	up(&top_sem);

	return E_OK;
}

/*
    Set Secondary LCD pinmux

    Select LCD pinmux on secondary interface

    @param[in] config   pinmux config (from configuration table)
    @param[in] pinmux   pinmux setting

    @return
        - @b E_OK: success
        - @b E_PAR: invalid pinmux
        - @b E_OBJ: pinmux conflict
*/
static ER pinmux_select_secondary_lcd(struct nvt_pinctrl_info *info, u32 config, u32 pinmux)
{
	u32 pinmux_type;
	union TOP_REG2 local_top_reg2;
	union TOP_REG6 local_top_reg6;
	union TOP_REG7 local_top_reg7;
	union TOP_REGLGPIO0 local_top_reg_lgpio0;

	local_top_reg2.reg = TOP_GETREG(info, TOP_REG2_OFS);
	local_top_reg6.reg = TOP_GETREG(info, TOP_REG6_OFS);
	local_top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);
	local_top_reg_lgpio0.reg = TOP_GETREG(info, TOP_REGLGPIO0_OFS);

	pinmux_type = pinmux & ~(PINMUX_LCD_SEL_FEATURE_MSK | PINMUX_PMI_CFG_MASK);
	//
	// 1. check pinmux conflict
	//
	switch (pinmux_type) {
	case PINMUX_LCD_SEL_GPIO:
		break;
	case PINMUX_LCD_SEL_CCIR656:
	case PINMUX_LCD_SEL_CCIR601:
	case PINMUX_LCD_SEL_SERIAL_RGB_8BITS:
	case PINMUX_LCD_SEL_SERIAL_RGB_6BITS:
	case PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS:
		/*CCIR656, Serial RGB666/888, Serial YCbCr conflict with: primary CCIR601 8/16, CCIR656 16 bits, RGB888/666/565,RGB 16 bits*/
		if (((local_top_reg2.bit.CCIR_HVLD_VVLD == CCIR_HVLD_VVLD_EN) || local_top_reg2.bit.CCIR_FIELD == CCIR_FIELD_EN) ||
			((local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_CCIR656) && (local_top_reg2.bit.CCIR_DATA_WIDTH == CCIR_DATA_WIDTH_16BITS)) ||
			(local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_PARALLEL_LCD) ||
			(local_top_reg2.bit.LCD_TYPE == LCDTYPE_ENUM_RGB_16BITS)) {
			pr_err("LCD2 CCIR656/601 or srial RGB/YCbCr conflict with LCD\r\n");
			return E_OBJ;
		}

		/*CCIR656, Serial RGB666/888, Serial YCbCr conflict with: serial MI*/
		if (local_top_reg2.bit.MEMIF_TYPE == MEMIF_TYPE_SERIAL) {
			pr_err("LCD2 CCIR656 or srial RGB/YCbCr conflict with SMI\r\n");
			return E_OBJ;
		}

		/*PICNT_2*/
		if (local_top_reg7.bit.PI_CNT == PICNT_ENUM_PICNT2) {
			pr_err("LCD2 CCIR656 or srial RGB/YCbCr conflict with PICNT_2\r\n");
			return E_OBJ;
		}

		if (pinmux_type != PINMUX_LCD_SEL_CCIR656) {
			/*PICNT2_2*/
			if (local_top_reg7.bit.PI_CNT2 == PICNT_ENUM_PICNT2) {
				pr_err("LCD2 CCIR656 or srial RGB/YCbCr conflict with PICNT2_2\r\n");
				return E_OBJ;
			}

			/*PICNT3_2*/
			if (local_top_reg7.bit.PI_CNT3 == PICNT_ENUM_PICNT2) {
				pr_err("LCD2 CCIR656 or srial RGB/YCbCr conflict with PICNT3_2\r\n");
				return E_OBJ;
			}
		}

		/*ETH*/
		if (local_top_reg6.bit.ETH != ETH_ID_ENUM_GPIO) {
			pr_err("LCD2 CCIR656 or srial RGB/YCbCr conflict with ETH\r\n");
			return E_OBJ;
		}


		if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_6BITS) {
			/*Serial RGB666 conflict with: primary parallel MI 16/18 bits, secondary parallel MI 8/9 bits*/
			if (local_top_reg2.bit.MEMIF_TYPE == MEMIF_TYPE_PARALLEL) {
				if ((local_top_reg2.bit.PMEM_SEL == PMEM_SEL_ENUM_1ST_PINMUX) &&
					((local_top_reg2.bit.PMEMIF_DATA_WIDTH == PMEMIF_DATA_WIDTH_16BITS) || (local_top_reg2.bit.PMEMIF_DATA_WIDTH == PMEMIF_DATA_WIDTH_18BITS))) {
					pr_err("LCD2 RGB666 conflict with 1ST PMI 16/18 bits\r\n");
					return E_OBJ;
				}
				if (local_top_reg2.bit.PMEM_SEL == PMEM_SEL_ENUM_2ND_PINMUX) {
					pr_err("LCD2 RGB666 conflict with 2ND PMI\r\n");
					return E_OBJ;
				}
			}
		} else {
			/*CCIR656, Serial RGB888, Serial YCbCr conflict with: parallel MI*/
			if (local_top_reg2.bit.MEMIF_TYPE == MEMIF_TYPE_PARALLEL) {
				pr_err("LCD2 CCIR656 or srial RGB888/YCbCr conflict with PMI\r\n");
				return E_OBJ;
			}
		}
		break;

	case PINMUX_LCD_SEL_PARALLE_MI_8BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_9BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_16BITS:
	case PINMUX_LCD_SEL_PARALLE_MI_18BITS:
	case PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO:
	case PINMUX_LCD_SEL_SERIAL_MI_SDIO: {
			ER erReturn = pinmux_select_mi_display(info, config, pinmux);
			if (erReturn != E_OK) {
				pr_err("Secondary LCD configure MI fail\r\n");

			}
			mi_display = PINMUX_DISPMUX_SEL_LCD2;
			return erReturn;
		}
		break;
	default:
		pr_err("invalid Secondary LCD config: 0x%x\r\n", pinmux);
		return E_PAR;
	}

	/*2. setup pinmux registers*/
	local_top_reg2.bit.LCD2_TYPE = LCDTYPE_ENUM_GPIO;
	local_top_reg2.bit.PLCD2_DE = PLCD_DE_ENUM_GPIO;
	switch (pinmux_type) {
	case PINMUX_LCD_SEL_GPIO:
		break;
	case PINMUX_LCD_SEL_CCIR656:
		local_top_reg2.bit.LCD2_TYPE = LCDTYPE_ENUM_CCIR656;
		break;
	case PINMUX_LCD_SEL_SERIAL_RGB_8BITS:
		local_top_reg2.bit.LCD2_TYPE = LCDTYPE_ENUM_SerialRGB_8BITS;
		break;
	case PINMUX_LCD_SEL_SERIAL_RGB_6BITS:
		local_top_reg2.bit.LCD2_TYPE = LCDTYPE_ENUM_SerialRGB_6BITS;
		break;
	case PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS:
	case PINMUX_LCD_SEL_CCIR601:
		local_top_reg2.bit.LCD2_TYPE = LCDTYPE_ENUM_SerialYCbCr_8BITS;
		break;
	default:
		break;
	}

	if (pinmux_type != PINMUX_LCD_SEL_SERIAL_RGB_6BITS) {
		local_top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
		local_top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
	}
	local_top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
	local_top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
	local_top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
	local_top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
	local_top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
	local_top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
	local_top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
	local_top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
	local_top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;


	if (pinmux & PINMUX_LCD_SEL_DE_ENABLE) {
		local_top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
		local_top_reg2.bit.PLCD2_DE = PLCD_DE_ENUM_DE;
	}

	/*Enter critical section*/
	down(&top_sem);

	TOP_SETREG(info, TOP_REG2_OFS, local_top_reg2.reg);
	TOP_SETREG(info, TOP_REGLGPIO0_OFS, local_top_reg_lgpio0.reg);

	/*Leave critical section*/
	up(&top_sem);

	return E_OK;
}

/*
	Active DSP PERI channel

	Active DSP PERI channel

	@param[in] pinmux   pinmux setting
		- @b PINMUX_DSP_CH_ACTIVE
		- @b PINMUX_DSP_CH_INACTIVE

	@return
		- @b E_OK: success
		- @b E_PAR: invalid pinmux
*/
static ER pinmux_active_dsp_peri(struct nvt_pinctrl_info *info, u32 pinmux)
{
	union TOP_REG11 local_top_reg11;

	/*Enter critical section*/
	down(&top_sem);

	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);

	switch (pinmux) {
	case PINMUX_DSP_CH_ACTIVE:
		local_top_reg11.bit.DSP_PERI_EN = 1;
		break;
	case PINMUX_DSP_CH_INACTIVE:
		local_top_reg11.bit.DSP_PERI_EN = 0;
		break;
	default:
		up(&top_sem);
		pr_err("invalid pinmux: 0x%x\r\n", pinmux);
		return E_PAR;
	}
	TOP_SETREG(info, TOP_REG11_OFS, local_top_reg11.reg);

	/*Leave critical section*/
	up(&top_sem);

	return E_OK;
}

/*
	Active DSP IOP channel

	Active DSP IOP channel

	@param[in] pinmux   pinmux setting
		- @b PINMUX_DSP_CH_ACTIVE
		- @b PINMUX_DSP_CH_INACTIVE

	@return
		- @b E_OK: success
		- @b E_PAR: invalid pinmux
*/
static ER pinmux_active_dsp_iop(struct nvt_pinctrl_info *info, u32 pinmux)
{
	union TOP_REG11 local_top_reg11;

	/*Enter critical section*/
	down(&top_sem);

	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);

	switch (pinmux) {
	case PINMUX_DSP_CH_ACTIVE:
		local_top_reg11.bit.DSP_IOP_EN = 1;
		break;
	case PINMUX_DSP_CH_INACTIVE:
		local_top_reg11.bit.DSP_IOP_EN = 0;
		break;
	default:
		up(&top_sem);
		pr_err("invalid pinmux: 0x%x\r\n", pinmux);
		return E_PAR;
	}
	TOP_SETREG(info, TOP_REG11_OFS, local_top_reg11.reg);

	/*Leave critical section*/
	up(&top_sem);

	return E_OK;
}

/*
	Active DSP2 PERI channel

	Active DSP2 PERI channel

	@param[in] pinmux   pinmux setting
		- @b PINMUX_DSP_CH_ACTIVE
		- @b PINMUX_DSP_CH_INACTIVE

	@return
		- @b E_OK: success
		- @b E_PAR: invalid pinmux
*/
static ER pinmux_active_dsp2_peri(struct nvt_pinctrl_info *info, u32 pinmux)
{
	union TOP_REG11 local_top_reg11;

	/*Enter critical section*/
	down(&top_sem);

	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);

	switch (pinmux) {
	case PINMUX_DSP_CH_ACTIVE:
		local_top_reg11.bit.DSP2_PERI_EN = 1;
		break;
	case PINMUX_DSP_CH_INACTIVE:
		local_top_reg11.bit.DSP2_PERI_EN = 0;
		break;
	default:
		up(&top_sem);
		pr_err("invalid pinmux: 0x%x\r\n", pinmux);
		return E_PAR;
	}
	TOP_SETREG(info, TOP_REG11_OFS, local_top_reg11.reg);

	/*Leave critical section*/
	up(&top_sem);

	return E_OK;
}

/*
	Active DSP2 IOP channel

	Active DSP2 IOP channel

	@param[in] pinmux   pinmux setting
		- @b PINMUX_DSP_CH_ACTIVE
		- @b PINMUX_DSP_CH_INACTIVE

	@return
		- @b E_OK: success
		- @b E_PAR: invalid pinmux
*/
static ER pinmux_active_dsp2_iop(struct nvt_pinctrl_info *info, u32 pinmux)
{
	union TOP_REG11 local_top_reg11;

	/*Enter critical section*/
	down(&top_sem);

	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);

	switch (pinmux) {
	case PINMUX_DSP_CH_ACTIVE:
		local_top_reg11.bit.DSP2_IOP_EN = 1;
		break;
	case PINMUX_DSP_CH_INACTIVE:
		local_top_reg11.bit.DSP2_IOP_EN = 0;
		break;
	default:
		up(&top_sem);
		pr_err("invalid pinmux: 0x%x\r\n", pinmux);
		return E_PAR;
	}
	TOP_SETREG(info, TOP_REG11_OFS, local_top_reg11.reg);

	/*Leave critical section*/
	up(&top_sem);

	return E_OK;
}


/**
	Set Function PINMUX

	Set Function PINMUX.

	@param[in] id   Function ID
			- @b PINMUX_FUNC_ID_LCD: 1st LCD
			- @b PINMUX_FUNC_ID_LCD2: 2nd LCD
			- @b PINMUX_FUNC_ID_TV: TV
			- @b PINMUX_FUNC_ID_HDMI: HDMI
	@param[in] pinmux pinmux setting

	@return
		- @b E_OK: success
		- @b E_ID: id out of range
*/
int pinmux_set_host(struct nvt_pinctrl_info *info, PINMUX_FUNC_ID id, u32 pinmux)
{
	ER ret = E_OK;
	switch ((u32)id) {
	case PINMUX_FUNC_ID_LCD:
	case PINMUX_FUNC_ID_LCD2: {
		u32 lcd_location = disp_pinmux_config[id] & PINMUX_DISPMUX_SEL_MASK;
		if (lcd_location == PINMUX_DISPMUX_SEL_LCD)
			ret = pinmux_select_primary_lcd(info, disp_pinmux_config[id], pinmux);
		else if (lcd_location == PINMUX_DISPMUX_SEL_LCD2)
			ret = pinmux_select_secondary_lcd(info, disp_pinmux_config[id], pinmux);
		}
		break;
	case PINMUX_FUNC_ID_DSP_PERI_CH_EN:
		pinmux_active_dsp_peri(info, pinmux);
		break;
	case PINMUX_FUNC_ID_DSP_IOP_CH_EN:
		pinmux_active_dsp_iop(info, pinmux);
		break;
	case PINMUX_FUNC_ID_DSP2_PERI_CH_EN:
		pinmux_active_dsp2_peri(info, pinmux);
		break;
	case PINMUX_FUNC_ID_DSP2_IOP_CH_EN:
		pinmux_active_dsp2_iop(info, pinmux);
		break;
	default:
		ret = E_ID;
		break;
	}

	return ret;
}
