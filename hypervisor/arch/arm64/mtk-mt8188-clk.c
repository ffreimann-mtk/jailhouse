/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) 2025 MediaTek
 *
 * Sharing of CLK between root and other inmate cells and control
 * concurrent access to shared registers.
 *
 * Authors:
 *   Felix Freimann <felix.freimann@mediatek.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <asm/mtk-common-clk.h>
#include <jailhouse/control.h>
#include <jailhouse/unit.h>


#define CLK_OFFSET_A_0_SET  0x0080
#define CLK_OFFSET_A_0_CLR  0x0084
#define CLK_OFFSET_A_1_SET  0x0088
#define CLK_OFFSET_A_1_CLR  0x008c
#define CLK_OFFSET_A_0      0x0090
#define CLK_OFFSET_A_1      0x0094
#define CLK_OFFSET_A_2_SET  0x00a4
#define CLK_OFFSET_A_2_CLR  0x00a8
#define CLK_OFFSET_A_2      0x00ac
#define CLK_OFFSET_A_3_SET  0x00c0
#define CLK_OFFSET_A_3_CLR  0x00c4
#define CLK_OFFSET_A_3      0x00c8
#define CLK_OFFSET_A_4_SET  0x00e0
#define CLK_OFFSET_A_4_CLR  0x00e4
#define CLK_OFFSET_A_4      0x00e8


#define CLK_SIZE   (0x00000400)
#define REG_SIZE   (sizeof (u32))

/* The following two macro's are required to customize the common macro's. */
#define REG_DIST_IDX_SHIFT  (2)
#define REG_NAME(_name)     CLK_OFFSET_ ## _name


/* Access descriptor for [CLK_OFFSET_A_0_SET .. CLK_OFFSET_A_4] */
static const access_descr_t clk_access_descr_0 [] =
{
    /* 0x0080 */ ACCESS_DESCR_WITH_IDX (A_0_SET, 0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_0_CLR, 0, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_1_SET, 1, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_1_CLR, 1, ACCESS_WO, ACCESS_ONE_BIT),
    /* 0x0090 */ ACCESS_DESCR_WITH_IDX (A_0,     1, ACCESS_RW, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_1,     1, ACCESS_RW, ACCESS_ONE_BIT), ACCESS_DESCR_EMPTY,                                            ACCESS_DESCR_EMPTY,
    /* 0x00a0 */ ACCESS_DESCR_EMPTY,                                            ACCESS_DESCR_WITH_IDX (A_2_SET, 2, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_2_CLR, 2, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_2,     2, ACCESS_RW, ACCESS_ONE_BIT),
    /* 0x00b0 */ ACCESS_DESCR_EMPTY,                                            ACCESS_DESCR_EMPTY,                                            ACCESS_DESCR_EMPTY,                                            ACCESS_DESCR_EMPTY,
    /* 0x00c0 */ ACCESS_DESCR_WITH_IDX (A_3_SET, 3, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_3_CLR, 3, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_3,     3, ACCESS_RW, ACCESS_ONE_BIT), ACCESS_DESCR_EMPTY,
    /* 0x00d0 */ ACCESS_DESCR_EMPTY,                                            ACCESS_DESCR_EMPTY,                                            ACCESS_DESCR_EMPTY,                                            ACCESS_DESCR_EMPTY,
    /* 0x00e0 */ ACCESS_DESCR_WITH_IDX (A_4_SET, 4, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_4_CLR, 4, ACCESS_WO, ACCESS_ONE_BIT), ACCESS_DESCR_WITH_IDX (A_4,     4, ACCESS_RW, ACCESS_ONE_BIT), ACCESS_DESCR_EMPTY,
};

static const access_descr_map_t  clk_access_descr_map [] =
{
    {
        .reg_start    = REG_NAME (A_0_SET),
        .reg_end      = REG_NAME (A_4) + REG_SIZE,
        .access_descr = clk_access_descr_0
    }
};


static u32 one_bit_per_pin (access_descr_t  access_descr);
static u32 addr_to_bitmap (struct mmio_access*  mmio,
                           access_descr_t       access_descr);


static const clk_config_descr_t mt8188_clk_config_descr =
{
    .addr_to_bitmap = addr_to_bitmap,

    .access_descr_map = clk_access_descr_map,

    .access_descr_map_size = ARRAY_SIZE (clk_access_descr_map),
    .reg_size              = CLK_SIZE,
};


static u32 one_bit_per_pin (access_descr_t  access_descr)
{
    u32          idx = get_access_dist_idx (access_descr);
    struct cell* cell = this_cell ();


    if (idx >= ARRAY_SIZE (cell->arch.clk_bitmap))
	{
        return (0);
	}
    
    return (cell->arch.clk_bitmap [idx]);
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

static int mt8188_clk_cell_init (struct cell*  cell)
{
    return (mtk_clk_cell_init (cell));
}

static void mt8188_clk_cell_exit (struct cell*  cell)
{
    mtk_clk_cell_exit (cell);
}

static unsigned int mt8188_clk_mmio_count_regions (struct cell*  cell)
{
    return (mtk_clk_mmio_count_regions (cell));
}

static int mt8188_clk_init (void)
{
    return (mtk_clk_init (&mt8188_clk_config_descr));
}

static void mt8188_clk_shutdown (void)
{
    mtk_clk_shutdown ();
}

DEFINE_UNIT (mt8188_clk, "mt8188_clk");
