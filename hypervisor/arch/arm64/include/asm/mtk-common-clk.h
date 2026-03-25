/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) 2026 MediaTek
 *
 * Common MediaTek macro's and definitions.
 *
 * Authors:
 *   Felix Freimann <felix.freimann@mediatek.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <asm/mtk-common.h>
#include <jailhouse/cell.h>
#include <jailhouse/mmio.h>


typedef u32 (*addr_to_bitmap_fct) (struct mmio_access*  mmio,
                                   access_descr_t       access_descr);

typedef struct
{
    addr_to_bitmap_fct  addr_to_bitmap;

    const access_descr_map_t*  access_descr_map;

    size_t  access_descr_map_size;

    size_t  reg_size;
}   clk_config_descr_t;


extern int mtk_clk_cell_init (struct cell*  cell);
extern void mtk_clk_cell_exit (struct cell*  cell);

extern unsigned int mtk_clk_mmio_count_regions (struct cell*  cell);

extern int mtk_clk_init (const clk_config_descr_t*  clk_config_descr);
extern void mtk_clk_shutdown (void);
