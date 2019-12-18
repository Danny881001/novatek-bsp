/*
	PAD controller register header

	PAD controller register header

	@file       pad_reg.h
	@ingroup    mIDrvSys_PAD
	@note       Nothing

	Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _PAD_REG_H
#define _PAD_REG_H

#include "rcw_macro.h"

//0x00 PAD Pull UP/DOWN Register 0
#define PAD_PUPD0_REG_OFS                   0x00
union PAD_PUPD0_REG {
	uint32_t reg;
	struct {
	unsigned int CGPIO0:2;   // MC0/C_GPIO0 Pull up/down
	unsigned int CGPIO1:2;   // MC1/C_GPIO1 Pull up/down
	unsigned int CGPIO2:2;   // MC2/C_GPIO2 Pull up/down
	unsigned int CGPIO3:2;   // MC3/C_GPIO3 Pull up/down
	unsigned int CGPIO4:2;   // MC4/C_GPIO4 Pull up/down
	unsigned int CGPIO5:2;   // MC5/C_GPIO5 Pull up/down
	unsigned int CGPIO6:2;   // MC6/C_GPIO6 Pull up/down
	unsigned int CGPIO7:2;   // MC7/C_GPIO7 Pull up/down
	unsigned int CGPIO8:2;   // MC8/C_GPIO8 Pull up/down
	unsigned int CGPIO9:2;   // MC9/C_GPIO9 Pull up/down
	unsigned int CGPIO10:2;  // MC10/C_GPIO10 Pull up/down
	unsigned int CGPIO11:2;  // MC11/C_GPIO11 Pull up/down
	unsigned int CGPIO12:2;  // MC12/C_GPIO12 Pull up/down
	unsigned int CGPIO13:2;  // MC13/C_GPIO13 Pull up/down
	unsigned int CGPIO14:2;  // MC14/C_GPIO14 Pull up/down
	unsigned int CGPIO15:2;  // MC15/C_GPIO15 Pull up/down
	} bit;
};

//0x04 PAD Pull UP/DOWN Register 1
#define PAD_PUPD1_REG_OFS                   0x04
union PAD_PUPD1_REG {
	uint32_t reg;
	struct {
	unsigned int CGPIO16:2;  // MC16/C_GPIO16 Pull up/down
	unsigned int CGPIO17:2;  // MC17/C_GPIO17 Pull up/down
	unsigned int CGPIO18:2;  // MC18/C_GPIO18 Pull up/down
	unsigned int CGPIO19:2;  // MC19/C_GPIO19 Pull up/down
	unsigned int CGPIO20:2;  // MC20/C_GPIO20 Pull up/down
	unsigned int CGPIO21:2;  // MC21/C_GPIO21 Pull up/down
	unsigned int CGPIO22:2;  // MC22/C_GPIO22 Pull up/down
	unsigned int CGPIO23:2;  // MC23/C_GPIO23 Pull up/down
	unsigned int CGPIO24:2;  // MC24/C_GPIO24 Pull up/down
	unsigned int CGPIO25:2;  // MC25/C_GPIO25 Pull up/down
	unsigned int CGPIO26:2;  // MC26/C_GPIO26 Pull up/down
	unsigned int CGPIO27:2;  // MC27/C_GPIO27 Pull up/down
	unsigned int CGPIO28:2;  // MC28/C_GPIO28 Pull up/down
	unsigned int CGPIO29:2;  // MC29/C_GPIO29 Pull up/down
	unsigned int CGPIO30:2;  // MC30/C_GPIO30 Pull up/down
	unsigned int CGPIO31:2;  // MC31/C_GPIO31 Pull up/down
	} bit;
};

//0x08 PAD Pull UP/DOWN Register 2
#define PAD_PUPD2_REG_OFS                   0x08
union PAD_PUPD2_REG {
	uint32_t reg;
	struct {
	unsigned int CGPIO32:2;  // MC32/C_GPIO32 Pull up/down
	unsigned int CGPIO33:2;  // MC33/C_GPIO33 Pull up/down
	unsigned int reserved0:28;
	} bit;
};

//0x0C PAD Pull UP/DOWN Register 3
#define PAD_PUPD3_REG_OFS                   0x0C
union PAD_PUPD3_REG {
	uint32_t reg;
	struct {
	unsigned int SGPIO0:2;   //S_GPIO0 Pull up/down
	unsigned int SGPIO1:2;   //S_GPIO1 Pull up/down
	unsigned int SGPIO2:2;   //S_GPIO2 Pull up/down
	unsigned int SGPIO3:2;   //S_GPIO3 Pull up/down
	unsigned int SGPIO4:2;   //S_GPIO4 Pull up/down
	unsigned int SGPIO5:2;   //S_GPIO5 Pull up/down
	unsigned int SGPIO6:2;   //S_GPIO6 Pull up/down
	unsigned int SGPIO7:2;   //S_GPIO7 Pull up/down
	unsigned int SGPIO8:2;   //S_GPIO8 Pull up/down
	unsigned int SGPIO9:2;   //S_GPIO9 Pull up/down
	unsigned int SGPIO10:2;  //S_GPIO10 Pull up/down
	unsigned int SGPIO11:2;  //S_GPIO11 Pull up/down
	unsigned int reserved0:8;
	} bit;
};

//0x10 PAD Pull UP/DOWN Register 4
#define PAD_PUPD4_REG_OFS                   0x10
union PAD_PUPD4_REG {
	uint32_t reg;
	struct {
	unsigned int PGPIO0:2;   // P_GPIO0 Pull up/down
	unsigned int PGPIO1:2;   // P_GPIO1 Pull up/down
	unsigned int PGPIO2:2;   // P_GPIO2 Pull up/down
	unsigned int PGPIO3:2;   // P_GPIO3 Pull up/down
	unsigned int PGPIO4:2;   // P_GPIO4 Pull up/down
	unsigned int PGPIO5:2;   // P_GPIO5 Pull up/down
	unsigned int PGPIO6:2;   // P_GPIO6 Pull up/down
	unsigned int PGPIO7:2;   // P_GPIO7 Pull up/down
	unsigned int PGPIO8:2;   // P_GPIO8 Pull up/down
	unsigned int PGPIO9:2;   // P_GPIO9 Pull up/down
	unsigned int PGPIO10:2;  // P_GPIO10 Pull up/down
	unsigned int PGPIO11:2;  // P_GPIO11 Pull up/down
	unsigned int PGPIO12:2;  // P_GPIO12 Pull up/down
	unsigned int PGPIO13:2;  // P_GPIO13 Pull up/down
	unsigned int PGPIO14:2;  // P_GPIO14 Pull up/down
	unsigned int PGPIO15:2;  // P_GPIO15 Pull up/down
	} bit;
};

//0x14 PAD Pull UP/DOWN Register 5
#define PAD_PUPD5_REG_OFS                   0x14
union PAD_PUPD5_REG {
	uint32_t reg;
	struct {
	unsigned int PGPIO16:2;  // P_GPIO16 Pull up/down
	unsigned int PGPIO17:2;  // P_GPIO17 Pull up/down
	unsigned int PGPIO18:2;  // P_GPIO18 Pull up/down
	unsigned int PGPIO19:2;  // P_GPIO19 Pull up/down
	unsigned int PGPIO20:2;  // P_GPIO20 Pull up/down
	unsigned int PGPIO21:2;  // P_GPIO21 Pull up/down
	unsigned int PGPIO22:2;  // P_GPIO22 Pull up/down
	unsigned int PGPIO23:2;  // P_GPIO23 Pull up/down
	unsigned int PGPIO24:2;  // P_GPIO24 Pull up/down
	unsigned int PGPIO25:2;  // P_GPIO25 Pull up/down
	unsigned int PGPIO26:2;  // P_GPIO26 Pull up/down
	unsigned int PGPIO27:2;  // P_GPIO27 Pull up/down
	unsigned int PGPIO28:2;  // P_GPIO28 Pull up/down
	unsigned int PGPIO29:2;  // P_GPIO29 Pull up/down
	unsigned int PGPIO30:2;  // P_GPIO30 Pull up/down
	unsigned int PGPIO31:2;  // P_GPIO31 Pull up/down
	} bit;
};

//0x18 PAD Pull UP/DOWN Register 6
#define PAD_PUPD6_REG_OFS                   0x18
union PAD_PUPD6_REG {
	uint32_t reg;
	struct {
	unsigned int PGPIO32:2;  // P_GPIO32 Pull up/down
	unsigned int PGPIO33:2;  // P_GPIO33 Pull up/down
	unsigned int PGPIO34:2;  // P_GPIO34 Pull up/down
	unsigned int PGPIO35:2;  // P_GPIO35 Pull up/down
	unsigned int PGPIO36:2;  // P_GPIO36 Pull up/down
	unsigned int PGPIO37:2;  // P_GPIO37 Pull up/down
	unsigned int PGPIO38:2;  // P_GPIO38 Pull up/down
	unsigned int PGPIO39:2;  // P_GPIO39 Pull up/down
	unsigned int PGPIO40:2;  // P_GPIO40 Pull up/down
	unsigned int PGPIO41:2;  // P_GPIO41 Pull up/down
	unsigned int PGPIO42:2;  // P_GPIO42 Pull up/down
	unsigned int PGPIO43:2;  // P_GPIO43 Pull up/down
	unsigned int PGPIO44:2;  // P_GPIO44 Pull up/down
	unsigned int PGPIO45:2;  // P_GPIO45 Pull up/down
	unsigned int PGPIO46:2;  // P_GPIO46 Pull up/down
	unsigned int PGPIO47:2;  // P_GPIO47 Pull up/down
	} bit;
};

//0x1C PAD Pull UP/DOWN Register 7
#define PAD_PUPD7_REG_OFS                   0x1C
union PAD_PUPD7_REG {
	uint32_t reg;
	struct {
	unsigned int LGPIO0:2;   // L_GPIO0 Pull up/down
	unsigned int LGPIO1:2;   // L_GPIO1 Pull up/down
	unsigned int LGPIO2:2;   // L_GPIO2 Pull up/down
	unsigned int LGPIO3:2;   // L_GPIO3 Pull up/down
	unsigned int LGPIO4:2;   // L_GPIO4 Pull up/down
	unsigned int LGPIO5:2;   // L_GPIO5 Pull up/down
	unsigned int LGPIO6:2;   // L_GPIO6 Pull up/down
	unsigned int LGPIO7:2;   // L_GPIO7 Pull up/down
	unsigned int LGPIO8:2;   // L_GPIO8 Pull up/down
	unsigned int LGPIO9:2;   // L_GPIO9 Pull up/down
	unsigned int LGPIO10:2;  // L_GPIO10 Pull up/down
	unsigned int LGPIO11:2;  // L_GPIO11 Pull up/down
	unsigned int LGPIO12:2;  // L_GPIO12 Pull up/down
	unsigned int LGPIO13:2;  // L_GPIO13 Pull up/down
	unsigned int LGPIO14:2;  // L_GPIO14 Pull up/down
	unsigned int LGPIO15:2;  // L_GPIO15 Pull up/down
	} bit;
};

//0x20 PAD Pull UP/DOWN Register 8
#define PAD_PUPD8_REG_OFS                   0x20
union PAD_PUPD8_REG {
	uint32_t reg;
	struct {
	unsigned int LGPIO16:2;  // L_GPIO16 Pull up/down
	unsigned int LGPIO17:2;  // L_GPIO17 Pull up/down
	unsigned int LGPIO18:2;  // L_GPIO18 Pull up/down
	unsigned int LGPIO19:2;  // L_GPIO19 Pull up/down
	unsigned int LGPIO20:2;  // L_GPIO20 Pull up/down
	unsigned int LGPIO21:2;  // L_GPIO21 Pull up/down
	unsigned int LGPIO22:2;  // L_GPIO22 Pull up/down
	unsigned int LGPIO23:2;  // L_GPIO23 Pull up/down
	unsigned int LGPIO24:2;  // L_GPIO24 Pull up/down
	unsigned int LGPIO25:2;  // L_GPIO25 Pull up/down
	unsigned int LGPIO26:2;  // L_GPIO26 Pull up/down
	unsigned int LGPIO27:2;  // L_GPIO27 Pull up/down
	unsigned int LGPIO28:2;  // L_GPIO28 Pull up/down
	unsigned int LGPIO29:2;  // L_GPIO29 Pull up/down
	unsigned int LGPIO30:2;  // L_GPIO30 Pull up/down
	unsigned int LGPIO31:2;  // L_GPIO31 Pull up/down
	} bit;
};

//0x24 PAD Pull UP/DOWN Register 9
#define PAD_PUPD9_REG_OFS                   0x24
union PAD_PUPD9_REG {
	uint32_t reg;
	struct {
	unsigned int LGPIO32:2;  //L_GPIO32 Pull up/down
	unsigned int reserved0:30;
	} bit;
};

//0x28 PAD Pull UP/DOWN Register 10
#define PAD_PUPD10_REG_OFS                  0x28
union PAD_PUPD10_REG {
	uint32_t reg;
	struct {
	unsigned int DGPIO0:2;   // D_GPIO0 Pull up/down
	unsigned int DGPIO1:2;   // D_GPIO1 Pull up/down
	unsigned int DGPIO2:2;   // D_GPIO2 Pull up/down
	unsigned int DGPIO3:2;   // D_GPIO3 Pull up/down
	unsigned int DGPIO4:2;   // D_GPIO4 Pull up/down
	unsigned int DGPIO5:2;   // D_GPIO5 Pull up/down
	unsigned int DGPIO6:2;   // D_GPIO6 Pull up/down
	unsigned int DGPIO7:2;   // D_GPIO7 Pull up/down
	unsigned int DGPIO8:2;   // D_GPIO8 Pull up/down
	unsigned int DGPIO9:2;   // D_GPIO9 Pull up/down
	unsigned int DGPIO10:2;  // D_GPIO10 Pull up/down
	unsigned int DGPIO11:2;  // D_GPIO11 Pull up/down
	unsigned int DGPIO12:2;  // D_GPIO12 Pull up/down
	unsigned int reserved0:6;
	} bit;
};

//0x40 PAD Driving/Sink Register 0
#define PAD_DS_REG_OFS                      0x40
union PAD_DS_REG {
	uint32_t reg;
	struct {
	unsigned int CGPIO0:2;   // C_GPIO0 Driving/Sink
	unsigned int CGPIO1:2;   // C_GPIO1 Driving/Sink
	unsigned int CGPIO2:2;   // C_GPIO2 Driving/Sink
	unsigned int CGPIO3:2;   // C_GPIO3 Driving/Sink
	unsigned int CGPIO4:2;   // C_GPIO4 Driving/Sink
	unsigned int CGPIO5:2;   // C_GPIO5 Driving/Sink
	unsigned int CGPIO6:2;   // C_GPIO6 Driving/Sink
	unsigned int CGPIO7:2;   // C_GPIO7 Driving/Sink
	unsigned int CGPIO8:2;   // C_GPIO8 Driving/Sink
	unsigned int CGPIO9:2;   // C_GPIO9 Driving/Sink
	unsigned int CGPIO10:2;  // C_GPIO10 Driving/Sink
	unsigned int CGPIO11:2;  // C_GPIO11 Driving/Sink
	unsigned int CGPIO12:2;  // C_GPIO12 Driving/Sink
	unsigned int CGPIO13:2;  // C_GPIO13 Driving/Sink
	unsigned int CGPIO14:2;  // C_GPIO14 Driving/Sink
	unsigned int CGPIO15:2;  // C_GPIO15 Driving/Sink
	} bit;
};

//0x44 PAD Driving/Sink Register 1
#define PAD_DS1_REG_OFS                     0x44
union PAD_DS1_REG {
	uint32_t reg;
	struct {
	unsigned int CGPIO16:2;  // C_GPIO16 Driving/Sink
	unsigned int CGPIO17:2;  // C_GPIO17 Driving/Sink
	unsigned int CGPIO18:2;  // C_GPIO18 Driving/Sink
	unsigned int CGPIO19:2;  // C_GPIO19 Driving/Sink
	unsigned int CGPIO20:2;  // C_GPIO20 Driving/Sink
	unsigned int CGPIO21:2;  // C_GPIO21 Driving/Sink
	unsigned int CGPIO22:2;  // C_GPIO22 Driving/Sink
	unsigned int CGPIO23:2;  // C_GPIO23 Driving/Sink
	unsigned int CGPIO24:2;  // C_GPIO24 Driving/Sink
	unsigned int CGPIO25:2;  // C_GPIO25 Driving/Sink
	unsigned int CGPIO26:2;  // C_GPIO26 Driving/Sink
	unsigned int CGPIO27:2;  // C_GPIO27 Driving/Sink
	unsigned int CGPIO28:2;  // C_GPIO28 Driving/Sink
	unsigned int CGPIO29:2;  // C_GPIO29 Driving/Sink
	unsigned int CGPIO30:2;  // C_GPIO30 Driving/Sink
	unsigned int CGPIO31:2;  // C_GPIO31 Driving/Sink
	} bit;
};

//0x48 PAD Driving/Sink Register 2
#define PAD_DS2_REG_OFS                     0x48
union PAD_DS2_REG {
	uint32_t reg;
	struct {
	unsigned int CGPIO32:2;  // C_GPIO32 Driving/Sink
	unsigned int CGPIO33:2;  // C_GPIO33 Driving/Sink
	unsigned int reserved0:28;
	} bit;
};

//0x4C PAD Driving/Sink Register 3
#define PAD_DS3_REG_OFS                     0x4C
union PAD_DS3_REG {
	uint32_t reg;
	struct {
	unsigned int SGPIO0:2;   // S_GPIO0 Driving/Sink
	unsigned int SGPIO1:2;   // S_GPIO1 Driving/Sink
	unsigned int SGPIO2:2;   // S_GPIO2 Driving/Sink
	unsigned int SGPIO3:2;   // S_GPIO3 Driving/Sink
	unsigned int SGPIO4:2;   // S_GPIO4 Driving/Sink
	unsigned int SGPIO5:2;   // S_GPIO5 Driving/Sink
	unsigned int SGPIO6:2;   // S_GPIO6 Driving/Sink
	unsigned int SGPIO7:2;   // S_GPIO7 Driving/Sink
	unsigned int SGPIO8:2;   // S_GPIO8 Driving/Sink
	unsigned int SGPIO9:2;   // S_GPIO9 Driving/Sink
	unsigned int SGPIO10:2;  // S_GPIO10 Pull up/down
	unsigned int reserved0:10;
	} bit;
};


//0x50 PAD Driving/Sink Register 4
#define PAD_DS4_REG_OFS                     0x50
union PAD_DS4_REG {
	uint32_t reg;
	struct {
	unsigned int PGPIO0:2;   // P_GPIO0 Driving/Sink
	unsigned int PGPIO1:2;   // P_GPIO1 Driving/Sink
	unsigned int PGPIO2:2;   // P_GPIO2 Driving/Sink
	unsigned int PGPIO3:2;   // P_GPIO3 Driving/Sink
	unsigned int PGPIO4:2;   // P_GPIO4 Driving/Sink
	unsigned int PGPIO5:2;   // P_GPIO5 Driving/Sink
	unsigned int PGPIO6:2;   // P_GPIO6 Driving/Sink
	unsigned int PGPIO7:2;   // P_GPIO7 Driving/Sink
	unsigned int PGPIO8:2;   // P_GPIO8 Driving/Sink
	unsigned int PGPIO9:2;   // P_GPIO9 Driving/Sink
	unsigned int PGPIO10:2;  // P_GPIO10 Driving/Sink
	unsigned int PGPIO11:2;  // P_GPIO11 Driving/Sink
	unsigned int PGPIO12:2;  // P_GPIO12 Driving/Sink
	unsigned int PGPIO13:2;  // P_GPIO13 Driving/Sink
	unsigned int PGPIO14:2;  // P_GPIO14 Driving/Sink
	unsigned int PGPIO15:2;  // P_GPIO15 Driving/Sink
	} bit;
};

//0x54 PAD Driving/Sink Register 5
#define PAD_DS5_REG_OFS                     0x54
union PAD_DS5_REG {
	uint32_t reg;
	struct {
	unsigned int PGPIO16:2;  // P_GPIO16 Driving/Sink
	unsigned int PGPIO17:2;  // P_GPIO17 Driving/Sink
	unsigned int PGPIO18:2;  // P_GPIO18 Driving/Sink
	unsigned int PGPIO19:2;  // P_GPIO19 Driving/Sink
	unsigned int PGPIO20:2;  // P_GPIO20 Driving/Sink
	unsigned int PGPIO21:2;  // P_GPIO21 Driving/Sink
	unsigned int PGPIO22:2;  // P_GPIO22 Driving/Sink
	unsigned int PGPIO23:2;  // P_GPIO23 Driving/Sink
	unsigned int PGPIO24:2;  // P_GPIO24 Driving/Sink
	unsigned int PGPIO25:2;  // P_GPIO25 Driving/Sink
	unsigned int PGPIO26:2;  // P_GPIO26 Driving/Sink
	unsigned int PGPIO27:2;  // P_GPIO27 Driving/Sink
	unsigned int PGPIO28:2;  // P_GPIO28 Driving/Sink
	unsigned int PGPIO29:2;  // P_GPIO29 Driving/Sink
	unsigned int PGPIO30:2;  // P_GPIO30 Driving/Sink
	unsigned int PGPIO31:2;  // P_GPIO31 Driving/Sink
	} bit;
};

//0x58 PAD Driving/sink Register 6
#define PAD_DS6_REG_OFS                     0x58
union PAD_DS6_REG {
	uint32_t reg;
	struct {
	unsigned int PGPIO32:2;   // PGPIO32 Driving/Sink
	unsigned int PGPIO33:2;   // PGPIO33 Driving/Sink
	unsigned int PGPIO34:2;   // PGPIO34 Driving/Sink
	unsigned int PGPIO35:2;   // PGPIO35 Driving/Sink
	unsigned int PGPIO36:2;   // PGPIO36 Driving/Sink
	unsigned int PGPIO37:2;   // PGPIO37 Driving/Sink
	unsigned int PGPIO38:2;   // PGPIO38 Driving/Sink
	unsigned int PGPIO39:2;   // PGPIO39 Driving/Sink
	unsigned int PGPIO40:2;   // PGPIO40 Driving/Sink
	unsigned int PGPIO41:2;   // PGPIO41 Driving/Sink
	unsigned int PGPIO42:2;   // PGPIO42 Driving/Sink
	unsigned int PGPIO43:2;   // PGPIO43 Driving/Sink
	unsigned int PGPIO44:2;   // PGPIO44 Driving/Sink
	unsigned int PGPIO45:2;   // PGPIO45 Driving/Sink
	unsigned int PGPIO46:2;   // PGPIO46 Driving/Sink
	unsigned int PGPIO47:2;   // PGPIO47 Driving/Sink
	} bit;
};

//0x5C PAD Driving/sink Register 7
#define PAD_DS7_REG_OFS                     0x5C
union PAD_DS7_REG {
	uint32_t reg;
	struct {
	unsigned int LGPIO0:2;    // LGPIO0 Driving/Sink
	unsigned int LGPIO1:2;    // LGPIO1 Driving/Sink
	unsigned int LGPIO2:2;    // LGPIO2 Driving/Sink
	unsigned int LGPIO3:2;    // LGPIO3 Driving/Sink
	unsigned int LGPIO4:2;    // LGPIO4 Driving/Sink
	unsigned int LGPIO5:2;    // LGPIO5 Driving/Sink
	unsigned int LGPIO6:2;    // LGPIO6 Driving/Sink
	unsigned int LGPIO7:2;    // LGPIO7 Driving/Sink
	unsigned int LGPIO8:2;    // LGPIO8 Driving/Sink
	unsigned int LGPIO9:2;    // LGPIO9 Driving/Sink
	unsigned int LGPIO10:2;   // LGPIO10 Driving/Sink
	unsigned int LGPIO11:2;   // LGPIO11 Driving/Sink
	unsigned int LGPIO12:2;   // LGPIO12 Driving/Sink
	unsigned int LGPIO13:2;   // LGPIO13 Driving/Sink
	unsigned int LGPIO14:2;   // LGPIO14 Driving/Sink
	unsigned int LGPIO15:2;   // LGPIO15 Driving/Sink
	} bit;
};

//0x60 PAD Driving/sink Register 8
#define PAD_DS8_REG_OFS                     0x60
union PAD_DS8_REG {
	uint32_t reg;
	struct {
	unsigned int LGPIO16:2;  // L_GPIO16 Driving/Sink
	unsigned int LGPIO17:2;  // L_GPIO17 Driving/Sink
	unsigned int LGPIO18:2;  // L_GPIO18 Driving/Sink
	unsigned int LGPIO19:2;  // L_GPIO19 Driving/Sink
	unsigned int LGPIO20:2;  // L_GPIO20 Driving/Sink
	unsigned int LGPIO21:2;  // L_GPIO21 Driving/Sink
	unsigned int LGPIO22:2;  // L_GPIO22 Driving/Sink
	unsigned int LGPIO23:2;  // L_GPIO23 Driving/Sink
	unsigned int LGPIO24:2;  // L_GPIO24 Driving/Sink
	unsigned int LGPIO25:2;  // L_GPIO25 Driving/Sink
	unsigned int LGPIO26:2;  // L_GPIO26 Driving/Sink
	unsigned int LGPIO27:2;  // L_GPIO27 Driving/Sink
	unsigned int LGPIO28:2;  // L_GPIO28 Driving/Sink
	unsigned int LGPIO29:2;  // L_GPIO29 Driving/Sink
	unsigned int LGPIO30:2;  // L_GPIO30 Driving/Sink
	unsigned int LGPIO31:2;  // L_GPIO31 Driving/Sink
	} bit;
};

//0x64 PAD Driving/sink Register 9
#define PAD_DS9_REG_OFS                     0x64
union PAD_DS9_REG {
	uint32_t reg;
	struct {
	unsigned int LGPIO32:2;   // LGPIO32 Driving/Sink
	unsigned int reserved0:30;
	} bit;
};

//0x68 PAD Driving/sink Register 10
#define PAD_DS10_REG_OFS                    0x68
union PAD_DS10_REG {
	uint32_t reg;
	struct {
	unsigned int DGPIO0:2;    // DGPIO0 Driving/Sink
	unsigned int DGPIO1:2;    // DGPIO1 Driving/Sink
	unsigned int DGPIO2:2;    // DGPIO2 Driving/Sink
	unsigned int DGPIO3:2;    // DGPIO3 Driving/Sink
	unsigned int DGPIO4:2;    // DGPIO4 Driving/Sink
	unsigned int DGPIO5:2;    // DGPIO5 Driving/Sink
	unsigned int DGPIO6:2;    // DGPIO6 Driving/Sink
	unsigned int DGPIO7:2;    // DGPIO7 Driving/Sink
	unsigned int DGPIO8:2;    // DGPIO8 Driving/Sink
	unsigned int DGPIO9:2;    // DGPIO9 Driving/Sink
	unsigned int DGPIO10:2;   // DGPIO10 Driving/Sink
	unsigned int DGPIO11:2;   // DGPIO11 Driving/Sink
	unsigned int DGPIO12:2;   // DGPIO12 Driving/Sink
	unsigned int reserved0:6;
	} bit;
};

//0x6C PAD Driving/sink Register 11
#define PAD_DS11_REG_OFS                     0x6C

//0x70 PAD Driving/sink Register 12
#define PAD_DS12_REG_OFS                     0x70

//0x80 PAD Power Control Register
#define PAD_PWR_REG_OFS                      0x80
union PAD_PWR_REG {
	uint32_t reg;
	struct {
	unsigned int MC_POWER0:1;      // MC22~MC27 power source. 0: 3.3V, 1: 1.8V
	unsigned int MC_SEL:1;          // regulator bias current
	unsigned int MC_OPSEL:1;       // regulator OPA gain/phase option
	unsigned int MC_PULLDOWN:1;    // regulator output pull down control
	unsigned int MC_REGULATOR_EN:1;// regulator enable
	unsigned int reserved0:7;
	} bit;
};

//0x84 PAD Power Control Register 1
#define PAD_PWR1_REG_OFS                     0x84
union PAD_PWR1_REG {
	uint32_t reg;
	struct {
	unsigned int MC_POWER0:1;      // MC22~MC27 power source. 0: 3.3V, 1: 1.8V
	unsigned int MC_SEL:1;          // regulator bias current
	unsigned int MC_OPSEL:1;       // regulator OPA gain/phase option
	unsigned int MC_PULLDOWN:1;    // regulator output pull down control
	unsigned int MC_REGULATOR_EN:1;// regulator enable
	unsigned int reserved0:7;
	} bit;
};

//0x88 PAD Power Control Register 2
#define PAD_PWR2_REG_OFS                     0x88
union PAD_PWR2_REG {
	uint32_t reg;
	struct {
	unsigned int ADC_POWER0:1;      // ADC power source. 0: 3.3V, 1: 1.8V
	unsigned int ADC_SEL:1;         // regulator bias current
	unsigned int ADC_OPSEL:1;       // regulator OPA gain/phase option
	unsigned int ADC_PULLDOWN:1;    // regulator output pull down control
	unsigned int ADC_REGULATOR_EN:1;// regulator enable
	unsigned int ADC_VAD:2;
	unsigned int reserved0:25;
	} bit;
};
#endif
