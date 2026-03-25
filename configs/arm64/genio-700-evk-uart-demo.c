/*
 * GENIO-700-EVK target - uart-demo
 *
 * Copyright 2025 MediaTek
 *
 * Authors:
 *   Felix Freimann <felix.freimann@mediatek.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <jailhouse/types.h>
#include <jailhouse/cell-config.h>

struct {
	struct jailhouse_cell_desc cell;
	__u64 cpus[1];
    __u32 smc_ids [1];
	struct jailhouse_memory mem_regions[3];
	struct jailhouse_vendor vendors[1];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.architecture = JAILHOUSE_ARM64,
		.name = "uart-demo",
		.flags = JAILHOUSE_CELL_PASSIVE_COMMREG | JAILHOUSE_CELL_VIRTUAL_CONSOLE_PERMITTED,

		.cpu_set_size = sizeof(config.cpus),
		.smc_ids_size = ARRAY_SIZE(config.smc_ids),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
		.num_vendors        = ARRAY_SIZE(config.vendors),

		.cpu_reset_address = CONFIG_INMATE_BASE,

		.console = {
			.address = 0x11001200,
			.divider = 0x2a,			/* baudrate = 38400 */
			.type = JAILHOUSE_CON_TYPE_8250,
			.flags = JAILHOUSE_CON_ACCESS_MMIO | JAILHOUSE_CON_REGDIST_4,
		},
	},

	.cpus = {
		0b00001000,
	},

    .smc_ids = {
        0xc2000517,     /* MTK_SIP_AUDIO_CONTROL */
    },

	.mem_regions = {
		/* UART1 */
        {
			.phys_start = 0x11001200,
			.virt_start = 0x11001200,
			.size = 0x0100,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED | JAILHOUSE_MEM_IO_32,
		},
		/* Inmate memory: 0x6b000000 - 0x6b010000 */
		{
			.phys_start = 0x6b000000,
			.virt_start = CONFIG_INMATE_BASE,
			.size = 0x00010000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
		},
		/* Communication region */
		{
			.virt_start = 0x80000000,
			.size = 0x00001000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_COMM_REGION,
		},
	},
	.vendors = {
		{
			.type = JAILHOUSE_VENDOR_MTK_CLK,
			.mtk_clk.address    = 0x10001000,
			.mtk_clk.clk_base   = 0,
			.mtk_clk.clk_bitmap = {
				0x00800000, 0x00000000, 0x00000000, 0x00000000	/* CLK for UART1 */
			}
		}
	}
};
