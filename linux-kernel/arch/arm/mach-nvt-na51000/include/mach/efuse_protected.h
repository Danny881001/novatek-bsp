/*
    Novatek protected header file of NT96660's driver.

    The header file for Novatek protected APIs of NT96660's driver.

    @file       efuse_protected.h
    @ingroup    mIDriver
    @note       For Novatek internal reference, don't export to agent or customer.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _NVT_EFUSE_PROTECTED_H
#define _NVT_EFUSE_PROTECTED_H

#include "nvt_type.h"

typedef enum {
	EFUSE_PARAM_OP_PRI = 0x0,
	EFUSE_PARAM_OP_SEC,

	EFUSE_PARAM_OP_CNT,
	ENUM_DUMMY4WORD(EFUSE_PARAM_OP_DATA)
} EFUSE_PARAM_OP_DATA;

#define EFUSE_FIELD_MAX             32                  //Total 32 USHORT = 32 x 16 = 512
#define EFUSE_SECOND_SOURCE_OFS     0x100

#define EFUSE_SYS_PARAM_MAX         16                  //Total 16 USHORT = 16 x 16 = 256
#define EFUSE_USR_FIELD_MAX         16                  //Total 16 USHORT = 16 x 16 = 256

#define EFUSE_STATUS_VALID                  0x8000
#define EFUSE_STATUS_FORCE_INVALID          0x4000

typedef enum {
	EFUSE_TRIM_DATA_HDMI_PRI            = 0x0,
	EFUSE_TRIM_DATA_USB_PRI             = 0x1,
	EFUSE_TRIM_DATA_DDR_DRIVING_PRI     = 0x2,
	EFUSE_TRIM_DATA_TVDEC_PRI           = 0x3,
	EFUSE_TRIM_DATA_SLVSEC_PRI          = 0x5,
	EFUSE_TRIM_DATA_VX1_PRI             = 0x6,
	EFUSE_TRIM_DATA_DDR2_DRIVING_PRI    = 0x7,
	EFUSE_TRIM_PARAM_FIELD				= 0x8,

	EFUSE_TRIM_DATA_USB2_PRI            = 0xd,
	EFUSE_LOT_ID1                       = 0xe,
	EFUSE_LOT_ID2                       = 0xf,

	EFUSE_TRIM_DATA_USB2_SEC            = EFUSE_TRIM_DATA_USB2_PRI + EFUSE_SECOND_SOURCE_OFS,
	EFUSE_TRIM_DATA_DDR2_DRIVING_SEC    = EFUSE_TRIM_DATA_DDR2_DRIVING_PRI + EFUSE_SECOND_SOURCE_OFS,
	EFUSE_TRIM_DATA_Vx1_SEC             = EFUSE_TRIM_DATA_VX1_PRI + EFUSE_SECOND_SOURCE_OFS,
	EFUSE_TRIM_DATA_SLVSEC_SEC          = EFUSE_TRIM_DATA_SLVSEC_PRI + EFUSE_SECOND_SOURCE_OFS,
	EFUSE_TRIM_DATA_TVDEC_SEC           = EFUSE_TRIM_DATA_TVDEC_PRI + EFUSE_SECOND_SOURCE_OFS,
	EFUSE_TRIM_DATA_DDR_DRIVING_SEC     = EFUSE_TRIM_DATA_DDR_DRIVING_PRI + EFUSE_SECOND_SOURCE_OFS,
	EFUSE_TRIM_DATA_USB_SEC             = EFUSE_TRIM_DATA_USB_PRI + EFUSE_SECOND_SOURCE_OFS,
	EFUSE_TRIM_DATA_HDMI_SEC            = EFUSE_TRIM_DATA_HDMI_PRI + EFUSE_SECOND_SOURCE_OFS,

	EFUSE_PARAM_FIELD_CNT               = 0x10,

	ENUM_DUMMY4WORD(EFUSE_PARAM_DATA_ADDR)
} EFUSE_PARAM_DATA_ADDR;

typedef enum {
	EFUSE_HDMI_TRIM_DATA = 0x100,
	EFUSE_TVDC_TRIM_DATA,
	EFUSE_USBC_TRIM_DATA,
	EFUSE_DDRP_TRIM_DATA,
	EFUSE_SLVSEC_TRIM_DATA,
	EFUSE_VX1_TRIM_DATA,
	EFUSE_DDR2P_TRIM_DATA,
	EFUSE_USBC2_TRIM_DATA,

	EFUSE_PARAM_CNT,
	ENUM_DUMMY4WORD(EFUSE_PARAM_DATA)
} EFUSE_PARAM_DATA;

typedef enum {
	EFUSE_OTP_1ST_KEY_SET_FIELD = 0x0,        // This if for secure boot
	EFUSE_OTP_2ND_KEY_SET_FIELD,
	EFUSE_OTP_3RD_KEY_SET_FIELD,
	EFUSE_OTP_4TH_KEY_SET_FIELD,
	EFUSE_OTP_TOTAL_KEY_SET_FIELD,
} EFUSE_OTP_KEY_SET_FIELD;

#define EFUSE_OTP_1ST_KEY_SET_FIELD_FLAG    28  	// This bit identify key set 0 was set
#define EFUSE_OTP_2ND_KEY_SET_FIELD_FLAG    29  	// This bit identify key set 1 was set
#define EFUSE_OTP_3RD_KEY_SET_FIELD_FLAG    30 		// This bit identify key set 2 was set
#define EFUSE_OTP_4TH_KEY_SET_FIELD_FLAG    31  	// This bit identify key set 3 was set


#define EFUSE_SUCCESS                E_OK
#define EFUSE_FREEZE_ERR             -1001          // Programmed already, only can read
#define EFUSE_INACTIVE_ERR           -1002          // This field is empty(not programmed yet)
#define EFUSE_INVALIDATE_ERR         -1003          // This field force invalidate already
#define EFUSE_UNKNOW_PARAM_ERR       -1004          // efuse param field not defined
#define EFUSE_OPS_ERR                -1005          // efuse operation error
#define EFUSE_SECURITY_ERR           -1006          // efuse under security mode => can not read back
#define EFUSE_PARAM_ERR              -1007          // efuse param error


extern INT32 efuse_read_param_ops(EFUSE_PARAM_DATA param, UINT16 *data);
extern INT32 efuse_otp_set_key(EFUSE_OTP_KEY_SET_FIELD key_set_index);
extern UINT32 efuse_key_get_flag(EFUSE_OTP_KEY_SET_FIELD key_set_index);
extern INT32 efuse_write_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key);

#endif

