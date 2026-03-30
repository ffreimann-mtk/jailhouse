/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) 2025 MediaTek
 *
 * Sharing of EINT between root and other inmate cells and control
 * concurrent access to shared registers.
 *
 * Authors:
 *   Felix Freimann <felix.freimann@mediatek.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <asm/mtk-common-eint.h>
#include <jailhouse/control.h>
#include <jailhouse/unit.h>


#define EINT_OFFSET_STA_0                   0x0000
#define EINT_OFFSET_STA_1                   0x0004
#define EINT_OFFSET_STA_2                   0x0008
#define EINT_OFFSET_STA_3                   0x000c
#define EINT_OFFSET_STA_4                   0x0010
#define EINT_OFFSET_STA_5                   0x0014
#define EINT_OFFSET_STA_6                   0x0018
#define EINT_OFFSET_STA_7                   0x001c
#define EINT_OFFSET_ACK_0                   0x0040
#define EINT_OFFSET_ACK_1                   0x0044
#define EINT_OFFSET_ACK_2                   0x0048
#define EINT_OFFSET_ACK_3                   0x004c
#define EINT_OFFSET_ACK_4                   0x0050
#define EINT_OFFSET_ACK_5                   0x0054
#define EINT_OFFSET_ACK_6                   0x0058
#define EINT_OFFSET_ACK_7                   0x005c
#define EINT_OFFSET_MASK_0                  0x0080
#define EINT_OFFSET_MASK_1                  0x0084
#define EINT_OFFSET_MASK_2                  0x0088
#define EINT_OFFSET_MASK_3                  0x008c
#define EINT_OFFSET_MASK_4                  0x0090
#define EINT_OFFSET_MASK_5                  0x0094
#define EINT_OFFSET_MASK_6                  0x0098
#define EINT_OFFSET_MASK_7                  0x009c
#define EINT_OFFSET_MASK_SET_0              0x00c0
#define EINT_OFFSET_MASK_SET_1              0x00c4
#define EINT_OFFSET_MASK_SET_2              0x00c8
#define EINT_OFFSET_MASK_SET_3              0x00cc
#define EINT_OFFSET_MASK_SET_4              0x00d0
#define EINT_OFFSET_MASK_SET_5              0x00d4
#define EINT_OFFSET_MASK_SET_6              0x00d8
#define EINT_OFFSET_MASK_SET_7              0x00dc
#define EINT_OFFSET_MASK_CLR_0              0x0100
#define EINT_OFFSET_MASK_CLR_1              0x0104
#define EINT_OFFSET_MASK_CLR_2              0x0108
#define EINT_OFFSET_MASK_CLR_3              0x010c
#define EINT_OFFSET_MASK_CLR_4              0x0110
#define EINT_OFFSET_MASK_CLR_5              0x0114
#define EINT_OFFSET_MASK_CLR_6              0x0118
#define EINT_OFFSET_MASK_CLR_7              0x011c
#define EINT_OFFSET_SENS_0                  0x0140
#define EINT_OFFSET_SENS_1                  0x0144
#define EINT_OFFSET_SENS_2                  0x0148
#define EINT_OFFSET_SENS_3                  0x014c
#define EINT_OFFSET_SENS_4                  0x0150
#define EINT_OFFSET_SENS_5                  0x0154
#define EINT_OFFSET_SENS_6                  0x0158
#define EINT_OFFSET_SENS_7                  0x015c
#define EINT_OFFSET_SENS_SET_0              0x0180
#define EINT_OFFSET_SENS_SET_1              0x0184
#define EINT_OFFSET_SENS_SET_2              0x0188
#define EINT_OFFSET_SENS_SET_3              0x018c
#define EINT_OFFSET_SENS_SET_4              0x0190
#define EINT_OFFSET_SENS_SET_5              0x0194
#define EINT_OFFSET_SENS_SET_6              0x0198
#define EINT_OFFSET_SENS_SET_7              0x019c
#define EINT_OFFSET_SENS_CLR_0              0x01c0
#define EINT_OFFSET_SENS_CLR_1              0x01c4
#define EINT_OFFSET_SENS_CLR_2              0x01c8
#define EINT_OFFSET_SENS_CLR_3              0x01cc
#define EINT_OFFSET_SENS_CLR_4              0x01d0
#define EINT_OFFSET_SENS_CLR_5              0x01d4
#define EINT_OFFSET_SENS_CLR_6              0x01d8
#define EINT_OFFSET_SENS_CLR_7              0x01dc
#define EINT_OFFSET_SOFT_0                  0x0200
#define EINT_OFFSET_SOFT_1                  0x0204
#define EINT_OFFSET_SOFT_2                  0x0208
#define EINT_OFFSET_SOFT_3                  0x020c
#define EINT_OFFSET_SOFT_4                  0x0210
#define EINT_OFFSET_SOFT_5                  0x0214
#define EINT_OFFSET_SOFT_6                  0x0218
#define EINT_OFFSET_SOFT_7                  0x021c
#define EINT_OFFSET_SOFT_SET_0              0x0240
#define EINT_OFFSET_SOFT_SET_1              0x0244
#define EINT_OFFSET_SOFT_SET_2              0x0248
#define EINT_OFFSET_SOFT_SET_3              0x024c
#define EINT_OFFSET_SOFT_SET_4              0x0250
#define EINT_OFFSET_SOFT_SET_5              0x0254
#define EINT_OFFSET_SOFT_SET_6              0x0258
#define EINT_OFFSET_SOFT_SET_7              0x025c
#define EINT_OFFSET_SOFT_CLR_0              0x0280
#define EINT_OFFSET_SOFT_CLR_1              0x0284
#define EINT_OFFSET_SOFT_CLR_2              0x0288
#define EINT_OFFSET_SOFT_CLR_3              0x028c
#define EINT_OFFSET_SOFT_CLR_4              0x0290
#define EINT_OFFSET_SOFT_CLR_5              0x0294
#define EINT_OFFSET_SOFT_CLR_6              0x0298
#define EINT_OFFSET_SOFT_CLR_7              0x029c
#define EINT_OFFSET_POL_0                   0x0300
#define EINT_OFFSET_POL_1                   0x0304
#define EINT_OFFSET_POL_2                   0x0308
#define EINT_OFFSET_POL_3                   0x030c
#define EINT_OFFSET_POL_4                   0x0310
#define EINT_OFFSET_POL_5                   0x0314
#define EINT_OFFSET_POL_6                   0x0318
#define EINT_OFFSET_POL_7                   0x031c
#define EINT_OFFSET_POL_SET_0               0x0340
#define EINT_OFFSET_POL_SET_1               0x0344
#define EINT_OFFSET_POL_SET_2               0x0348
#define EINT_OFFSET_POL_SET_3               0x034c
#define EINT_OFFSET_POL_SET_4               0x0350
#define EINT_OFFSET_POL_SET_5               0x0354
#define EINT_OFFSET_POL_SET_6               0x0358
#define EINT_OFFSET_POL_SET_7               0x035c
#define EINT_OFFSET_POL_CLR_0               0x0380
#define EINT_OFFSET_POL_CLR_1               0x0384
#define EINT_OFFSET_POL_CLR_2               0x0388
#define EINT_OFFSET_POL_CLR_3               0x038c
#define EINT_OFFSET_POL_CLR_4               0x0390
#define EINT_OFFSET_POL_CLR_5               0x0394
#define EINT_OFFSET_POL_CLR_6               0x0398
#define EINT_OFFSET_POL_CLR_7               0x039c
#define EINT_OFFSET_D0EN_0                  0x0400
#define EINT_OFFSET_D0EN_1                  0x0404
#define EINT_OFFSET_D0EN_2                  0x0408
#define EINT_OFFSET_D0EN_3                  0x040c
#define EINT_OFFSET_D0EN_4                  0x0410
#define EINT_OFFSET_D0EN_5                  0x0414
#define EINT_OFFSET_D0EN_6                  0x0418
#define EINT_OFFSET_DBNC_3_0                0x0500
#define EINT_OFFSET_DBNC_7_4                0x0504
#define EINT_OFFSET_DBNC_B_8                0x0508
#define EINT_OFFSET_DBNC_F_C                0x050c
#define EINT_OFFSET_DBNC_1_3_0              0x0510
#define EINT_OFFSET_DBNC_1_7_4              0x0514
#define EINT_OFFSET_DBNC_SET_3_0            0x0600
#define EINT_OFFSET_DBNC_SET_7_4            0x0604
#define EINT_OFFSET_DBNC_SET_B_8            0x0608
#define EINT_OFFSET_DBNC_SET_F_C            0x060c
#define EINT_OFFSET_DBNC_SET_1_3_0          0x0610
#define EINT_OFFSET_DBNC_SET_1_7_4          0x0614
#define EINT_OFFSET_DBNC_CLR_3_0            0x0700
#define EINT_OFFSET_DBNC_CLR_7_4            0x0704
#define EINT_OFFSET_DBNC_CLR_B_8            0x0708
#define EINT_OFFSET_DBNC_CLR_F_C            0x070c
#define EINT_OFFSET_DBNC_CLR_1_3_0          0x0710
#define EINT_OFFSET_DBNC_CLR_1_7_4          0x0714
#define EINT_OFFSET_EVENT_MASK_0            0x0800
#define EINT_OFFSET_EVENT_MASK_0_SET        0x0840
#define EINT_OFFSET_EVENT_MASK_0_CLR        0x0880
#define EINT_OFFSET_CON_0                   0x0900
#define EINT_OFFSET_SEL_0                   0x0940
#define EINT_OFFSET_SEL_0_SET               0x0980
#define EINT_OFFSET_SEL_0_CLR               0x09c0
#define EINT_OFFSET_EEVT                    0x09f0
#define EINT_OFFSET_RAW_STA_0               0x0a00
#define EINT_OFFSET_RAW_STA_1               0x0a04
#define EINT_OFFSET_RAW_STA_2               0x0a08
#define EINT_OFFSET_RAW_STA_3               0x0a0c
#define EINT_OFFSET_RAW_STA_4               0x0a10
#define EINT_OFFSET_RAW_STA_5               0x0a14
#define EINT_OFFSET_RAW_STA_6               0x0a18
#define EINT_OFFSET_RAW_STA_7               0x0a1c
#define EINT_OFFSET_SECURE_EINT_EN          0x0b00
#define EINT_OFFSET_SECURE_DIR_EINT_EN      0x0b10
#define EINT_OFFSET_DCM_ON                  0x0b20
#define EINT_OFFSET_SECURE_STATUS           0x0b30
#define EINT_OFFSET_SYNC_EN_0               0x0c00
#define EINT_OFFSET_SYNC_EN_1               0x0c04
#define EINT_OFFSET_SYS_TIMER_LATCH_CTRL    0x0c20
#define EINT_OFFSET_SYS_TIMER_LATCH_0_L     0x0c24
#define EINT_OFFSET_SYS_TIMER_LATCH_0_H     0x0c28
#define EINT_OFFSET_SYS_TIMER_LATCH_1_L     0x0c2c
#define EINT_OFFSET_SYS_TIMER_LATCH_1_H     0x0c30
#define EINT_OFFSET_SYS_TIMER_LATCH_2_L     0x0c34
#define EINT_OFFSET_SYS_TIMER_LATCH_2_H     0x0c38
#define EINT_OFFSET_SYNC_EN_SET_0           0x0d00
#define EINT_OFFSET_SYNC_EN_SET_1           0x0d04
#define EINT_OFFSET_SYS_TIMER_IRQ_SRC_SEL   0x0d20
#define EINT_OFFSET_SYNC_EN_CLR_0           0x0e00
#define EINT_OFFSET_SYNC_EN_CLR_1           0x0e04
#define EINT_OFFSET_FPGA_EMUL_0             0x0f00

#define EINT_SIZE  (0x00001000)
#define REG_SIZE   (sizeof (u32))

#define EINT_IRQ_ID  267        /* SPI interrupt for EINT's. */


/* The following two macro's are required to customize the common macro's. */
#define REG_DIST_IDX_SHIFT  (2)
#define REG_NAME(_name)     EINT_OFFSET_ ## _name


/* Access descriptor for [EINT_OFFSET_STA_0 .. EINT_OFFSET_D0EN_7] */
static const access_descr_t eint_access_descr_0 [] =
{
    /* 0x0000 */ ACCESS_DESCR (STA_0, STA_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (STA_1, STA_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (STA_2, STA_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (STA_3, STA_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0010 */ ACCESS_DESCR (STA_4, STA_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (STA_5, STA_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (STA_6, STA_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (STA_7, STA_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0020 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0030 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0040 */ ACCESS_DESCR (ACK_0, ACK_0, ACCESS_WO, ACCESS_ONE_BIT),           ACCESS_DESCR (ACK_1, ACK_0, ACCESS_WO, ACCESS_ONE_BIT),           ACCESS_DESCR (ACK_2, ACK_0, ACCESS_WO, ACCESS_ONE_BIT),           ACCESS_DESCR (ACK_3, ACK_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0050 */ ACCESS_DESCR (ACK_4, ACK_0, ACCESS_WO, ACCESS_ONE_BIT),           ACCESS_DESCR (ACK_5, ACK_0, ACCESS_WO, ACCESS_ONE_BIT),           ACCESS_DESCR (ACK_6, ACK_0, ACCESS_WO, ACCESS_ONE_BIT),           ACCESS_DESCR (ACK_7, ACK_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0060 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0070 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0080 */ ACCESS_DESCR (MASK_0, MASK_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (MASK_1, MASK_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (MASK_2, MASK_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (MASK_3, MASK_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0090 */ ACCESS_DESCR (MASK_4, MASK_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (MASK_5, MASK_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (MASK_6, MASK_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (MASK_7, MASK_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x00a0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x00b0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x00c0 */ ACCESS_DESCR (MASK_SET_0, MASK_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_SET_1, MASK_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_SET_2, MASK_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_SET_3, MASK_SET_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x00d0 */ ACCESS_DESCR (MASK_SET_4, MASK_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_SET_5, MASK_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_SET_6, MASK_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_SET_7, MASK_SET_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x00e0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x00f0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0100 */ ACCESS_DESCR (MASK_CLR_0, MASK_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_CLR_1, MASK_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_CLR_2, MASK_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_CLR_3, MASK_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0110 */ ACCESS_DESCR (MASK_CLR_4, MASK_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_CLR_5, MASK_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_CLR_6, MASK_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (MASK_CLR_7, MASK_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0120 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0130 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0140 */ ACCESS_DESCR (SENS_0, SENS_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SENS_1, SENS_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SENS_2, SENS_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SENS_3, SENS_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0150 */ ACCESS_DESCR (SENS_4, SENS_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SENS_5, SENS_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SENS_6, SENS_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SENS_7, SENS_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0160 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0170 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0180 */ ACCESS_DESCR (SENS_SET_0, SENS_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_SET_1, SENS_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_SET_2, SENS_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_SET_3, SENS_SET_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0190 */ ACCESS_DESCR (SENS_SET_4, SENS_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_SET_5, SENS_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_SET_6, SENS_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_SET_7, SENS_SET_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x01a0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x01b0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x01c0 */ ACCESS_DESCR (SENS_CLR_0, SENS_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_CLR_1, SENS_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_CLR_2, SENS_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_CLR_3, SENS_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x01d0 */ ACCESS_DESCR (SENS_CLR_4, SENS_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_CLR_5, SENS_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_CLR_6, SENS_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SENS_CLR_7, SENS_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x01e0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x01f0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0200 */ ACCESS_DESCR (SOFT_0, SOFT_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SOFT_1, SOFT_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SOFT_2, SOFT_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SOFT_3, SOFT_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0210 */ ACCESS_DESCR (SOFT_4, SOFT_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SOFT_5, SOFT_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SOFT_6, SOFT_0, ACCESS_RO, ACCESS_ONE_BIT),         ACCESS_DESCR (SOFT_7, SOFT_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0220 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0230 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0240 */ ACCESS_DESCR (SOFT_SET_0, SOFT_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_SET_1, SOFT_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_SET_2, SOFT_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_SET_3, SOFT_SET_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0250 */ ACCESS_DESCR (SOFT_SET_4, SOFT_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_SET_5, SOFT_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_SET_6, SOFT_SET_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_SET_7, SOFT_SET_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0260 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0270 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0280 */ ACCESS_DESCR (SOFT_CLR_0, SOFT_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_CLR_1, SOFT_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_CLR_2, SOFT_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_CLR_3, SOFT_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0290 */ ACCESS_DESCR (SOFT_CLR_4, SOFT_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_CLR_5, SOFT_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_CLR_6, SOFT_CLR_0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR (SOFT_CLR_7, SOFT_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x02a0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x02b0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x02c0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x02d0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x02e0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x02f0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0300 */ ACCESS_DESCR (POL_0, POL_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (POL_1, POL_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (POL_2, POL_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (POL_3, POL_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0310 */ ACCESS_DESCR (POL_4, POL_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (POL_5, POL_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (POL_6, POL_0, ACCESS_RO, ACCESS_ONE_BIT),           ACCESS_DESCR (POL_7, POL_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0320 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0330 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0340 */ ACCESS_DESCR (POL_SET_0, POL_SET_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_SET_1, POL_SET_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_SET_2, POL_SET_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_SET_3, POL_SET_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0350 */ ACCESS_DESCR (POL_SET_4, POL_SET_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_SET_5, POL_SET_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_SET_6, POL_SET_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_SET_7, POL_SET_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0360 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0370 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0380 */ ACCESS_DESCR (POL_CLR_0, POL_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_CLR_1, POL_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_CLR_2, POL_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_CLR_3, POL_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0390 */ ACCESS_DESCR (POL_CLR_4, POL_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_CLR_5, POL_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_CLR_6, POL_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),   ACCESS_DESCR (POL_CLR_7, POL_CLR_0, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x03a0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x03b0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x03c0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x03d0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x03e0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x03f0 */ ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,                                               ACCESS_DESCR_EMPTY,
    /* 0x0380 */ ACCESS_DESCR (D0EN_0, D0EN_0, ACCESS_RW, ACCESS_ONE_BIT),         ACCESS_DESCR (D0EN_1, D0EN_0, ACCESS_RW, ACCESS_ONE_BIT),         ACCESS_DESCR (D0EN_2, D0EN_0, ACCESS_RW, ACCESS_ONE_BIT),         ACCESS_DESCR (D0EN_3, D0EN_0, ACCESS_RW, ACCESS_ONE_BIT),
    /* 0x0390 */ ACCESS_DESCR (D0EN_4, D0EN_0, ACCESS_RW, ACCESS_ONE_BIT),         ACCESS_DESCR (D0EN_5, D0EN_0, ACCESS_RW, ACCESS_ONE_BIT),         ACCESS_DESCR (D0EN_6, D0EN_0, ACCESS_RW, ACCESS_ONE_BIT),         ACCESS_DESCR_EMPTY,
};

/* Access descriptor for [EINT_OFFSET_RAW_STA_0 .. EINT_OFFSET_RAW_STA_7] */
static const access_descr_t eint_access_descr_1 [] =
{
    /* 0x0a00 */ ACCESS_DESCR (RAW_STA_0, RAW_STA_0, ACCESS_RO, ACCESS_ONE_BIT), ACCESS_DESCR (RAW_STA_1, RAW_STA_0, ACCESS_RO, ACCESS_ONE_BIT), ACCESS_DESCR (RAW_STA_2, RAW_STA_0, ACCESS_RO, ACCESS_ONE_BIT), ACCESS_DESCR (RAW_STA_3, RAW_STA_0, ACCESS_RO, ACCESS_ONE_BIT),
    /* 0x0a10 */ ACCESS_DESCR (RAW_STA_4, RAW_STA_0, ACCESS_RO, ACCESS_ONE_BIT), ACCESS_DESCR (RAW_STA_5, RAW_STA_0, ACCESS_RO, ACCESS_ONE_BIT), ACCESS_DESCR (RAW_STA_6, RAW_STA_0, ACCESS_RO, ACCESS_ONE_BIT), ACCESS_DESCR (RAW_STA_7, RAW_STA_0, ACCESS_RO, ACCESS_ONE_BIT),
};

static const access_descr_map_t  eint_access_descr_map [] =
{
    {
        .reg_start    = REG_NAME (STA_0),
        .reg_end      = REG_NAME (D0EN_4) + REG_SIZE,
        .access_descr = eint_access_descr_0
    },
    {
        .reg_start    = REG_NAME (RAW_STA_0),
        .reg_end      = REG_NAME (RAW_STA_4) + REG_SIZE,
        .access_descr = eint_access_descr_1
    }
};

static const irq_descr_map_t  eint_irq_status_map [] =
{
    {
        .reg  = REG_NAME (STA_0),
        .size = REG_SIZE,
        .mask = 0xffffffff,
    },
    {
        .reg  = REG_NAME (STA_1),
        .size = REG_SIZE,
        .mask = 0xffffffff,
    },
    {
        .reg  = REG_NAME (STA_2),
        .size = REG_SIZE,
        .mask = 0xffffffff,
    },
    {
        .reg  = REG_NAME (STA_3),
        .size = REG_SIZE,
        .mask = 0xffffffff,
    },
    {
        .reg  = REG_NAME (STA_4),
        .size = REG_SIZE,
        .mask = 0xffffffff,
    },
    {
        .reg  = REG_NAME (STA_5),
        .size = REG_SIZE,
        .mask = 0xffffffff,
    },
    {
        .reg  = REG_NAME (STA_6),
        .size = REG_SIZE,
        .mask = 0xffffffff,
    },
    {
        .reg  = REG_NAME (STA_7),
        .size = REG_SIZE,
        .mask = 0xffffffff,
    },
};


static u32 one_bit_per_pin (access_descr_t  access_descr);
static u32 addr_to_bitmap (struct mmio_access*  mmio,
                           access_descr_t       access_descr);


static const eint_config_descr_t mt8188_eint_config_descr =
{
    .addr_to_bitmap = addr_to_bitmap,

    .access_descr_map = eint_access_descr_map,
    .irq_status_map   = eint_irq_status_map,

    .access_descr_map_size = ARRAY_SIZE (eint_access_descr_map),
    .irq_status_map_size   = ARRAY_SIZE (eint_irq_status_map),
    .reg_size              = EINT_SIZE,

    .irq_id = EINT_IRQ_ID,
};


static u32 one_bit_per_pin (access_descr_t  access_descr)
{
    u32          idx = get_access_dist_idx (access_descr);
    struct cell* cell = this_cell ();


    if (idx >= ARRAY_SIZE (cell->arch.eint_bitmap))
	{
        return (0);
	}
    
    return (cell->arch.eint_bitmap [idx]);
}

static u32 addr_to_bitmap (struct mmio_access*  mmio,
                           access_descr_t       access_descr)
{
	switch (get_access_num_of_bits (access_descr))
	{
        case ACCESS_ONE_BIT:
            return (one_bit_per_pin (access_descr));
            break;
										   
		default:
			break;
	}
								   
    return (0xffffffff);
}

static int mt8188_eint_cell_init (struct cell*  cell)
{
    return (mtk_eint_cell_init (cell));
}

static void mt8188_eint_cell_exit (struct cell*  cell)
{
    mtk_eint_cell_exit (cell);
}

static unsigned int mt8188_eint_mmio_count_regions (struct cell*  cell)
{
    return (mtk_eint_mmio_count_regions (cell));
}

static int mt8188_eint_init (void)
{
    return (mtk_eint_init (&mt8188_eint_config_descr));
}

static void mt8188_eint_shutdown (void)
{
    mtk_eint_shutdown ();
}

DEFINE_UNIT (mt8188_eint, "mt8188_eint");
