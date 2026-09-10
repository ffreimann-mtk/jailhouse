/*
 * GENIO-700-EVK target - zephyr
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
	struct jailhouse_memory mem_regions[10];
	struct jailhouse_irqchip irqchips[2];
	struct jailhouse_vendor vendors[4];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.architecture = JAILHOUSE_ARM64,
		.name = "zephyr",
		.flags = JAILHOUSE_CELL_PASSIVE_COMMREG | JAILHOUSE_CELL_VIRTUAL_CONSOLE_PERMITTED,

		.cpu_set_size       = sizeof(config.cpus),
		.smc_ids_size       = ARRAY_SIZE(config.smc_ids),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
		.num_irqchips       = ARRAY_SIZE(config.irqchips),
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
		/* topckgen */
	        {
			.phys_start = 0x10000000,
			.virt_start = 0x10000000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED,
		},
		/* infra_a0 */
		{
			.phys_start = 0x10001400,
			.virt_start = 0x10001400,
			.size = 0x00000c00,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED | JAILHOUSE_MEM_IO_32,
		},
		/* toprgu */
	        {
			.phys_start = 0x10007000,
			.virt_start = 0x10007000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED,
		},
		/* apmixedsys */
	        {
			.phys_start = 0x1000c000,
			.virt_start = 0x1000c000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED,
		},
		/* afe */
	        {
			.phys_start = 0x10b10000,
			.virt_start = 0x10b10000,
			.size = 0x10000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED,
		},
		/* adsp_audio26m */
	        {
			.phys_start = 0x10b91000,
			.virt_start = 0x10b91000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED,
		},
		/* UART1 */
	        {
			.phys_start = 0x11001200,
			.virt_start = 0x11001200,
			.size = 0x0100,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED | JAILHOUSE_MEM_IO_32,
		},
		/* DMA Pool:  0x0000'0000'6000'0000 - 0x0000'0000'6110'0000 */
		{
			.phys_start = 0x61000000,
			.virt_start = 0x61000000,
			.size = 0x00800000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_ROOTSHARED,
		},
		/* Inmate memory: 0x6b000000 - 0x6b800000 */
		{
			.phys_start = 0x6b000000,
			.virt_start = CONFIG_INMATE_BASE,
			.size = 0x00800000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
		},
		/* Communication region */
		{
			.virt_start = 0x80000000,
			.size = 0x00001000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE | JAILHOUSE_MEM_COMM_REGION,
		},
	},

	.irqchips = {
		/* GIC */
		{
			.address    = 0x0c000000,
			.pin_base   = 160,
			.pin_bitmap = {
				0x00004000, 0x00000000, 0x00000000, 0x00000000	/* UART2 IRQ --> SPI 142+32 */
            /* I2S IRQ --> SPI 822 */
			},
		},
		{
			.address    = 0x0c000000,
			.pin_base   = 256,
			.pin_bitmap = {
				0x00000800, 0x00000000, 0x00000000, 0x00000000	/* EINT IRQ --> SPI 235+32 */
			},
		},
	},

	.vendors = {
		{
			.type = JAILHOUSE_VENDOR_MTK_EINT,
			.mtk_eint.address    = 0x1000b000,
			.mtk_eint.pin_base   = 32,
			.mtk_eint.pin_bitmap = {
				0x00000140, 0x00000000, 0x00000000, 0x00000000	/* GPIO 38 & 40 */
			}
		},
		{
			.type = JAILHOUSE_VENDOR_MTK_GPIO,
			.mtk_gpio.address    = 0x10005000,
			.mtk_gpio.pin_base   = 32,
			.mtk_gpio.pin_bitmap = {
				0x00000146, 0x00000000, 0x00000000, 0x00000000	/* Pins 33 & 34 for UART1; GPIO 38 & 40 */
			}
		},
		{
			.type = JAILHOUSE_VENDOR_MTK_CLK,
			.mtk_clk.address    = 0x10001000,
			.mtk_clk.clk_base   = 0,
			.mtk_clk.clk_bitmap = {
				0x00800000, 0x00000000, 0x00000000, 0x00000000	/* CLK for UART1 */
			}
		},
		{
			.type = JAILHOUSE_VENDOR_MTK_CLK,
			.mtk_clk.address    = 0x10001000,
			.mtk_clk.clk_base   = 128,
			.mtk_clk.clk_bitmap = {
				0x00000000, 0x00101000, 0x00000000, 0x00000000	/* BUS protect for AFE reset */
			}
		},
	},
};
