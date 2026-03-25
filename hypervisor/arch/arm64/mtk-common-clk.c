/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) 2026 MediaTek
 *
 * Common part for sharing CLK between root and other inmate cells and
 * control concurrent access to shared registers.
 *
 * Authors:
 *   Felix Freimann <felix.freimann@mediatek.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <asm/irqchip.h>
#include <asm/mtk-common-clk.h>
#include <asm/spinlock.h>
#include <jailhouse/cell.h>
#include <jailhouse/control.h>
#include <jailhouse/entry.h>
#include <jailhouse/paging.h>
#include <jailhouse/percpu.h>
#include <jailhouse/printk.h>


static spinlock_t lock;

static void* virt_addr = NULL;

static const clk_config_descr_t*  config_descr = NULL;


static void handle_ro_access (struct mmio_access*  mmio,
                              access_descr_t       access_descr);
static void handle_wo_access (struct mmio_access*  mmio,
                              access_descr_t       access_descr);
static void handle_rw_access (struct mmio_access*  mmio,
                              access_descr_t       access_descr);

static enum mmio_result clk_handle_access (void*                arg,
                                           struct mmio_access*  mmio);

static int get_phys_addr (struct cell*    cell,
                          unsigned long*  phys_addr);


static void handle_ro_access (struct mmio_access*  mmio,
                              access_descr_t       access_descr)
{
	if (! (mmio->is_write))
	{
        u32 bitmap = config_descr->addr_to_bitmap (mmio, access_descr);


        if (bitmap != 0)
        {
            mmio_perform_access (virt_addr, mmio);

      		/* Only allow the bits for which access is allowed. */
	    	mmio->value &= bitmap;
        }
        else
        {
            mmio->value = 0;
        }
    }
}

static void handle_wo_access (struct mmio_access*  mmio,
                              access_descr_t       access_descr)
{
	if (mmio->is_write)
	{
        u32 bitmap = config_descr->addr_to_bitmap (mmio, access_descr);


		/* Only allow the bits for which access is allowed. */
        if ((bitmap != 0)                            &&
            ((mmio->value & bitmap) == mmio->value))
        {
            mmio_perform_access (virt_addr, mmio);
        }
    }
}

static void handle_rw_access (struct mmio_access*  mmio,
                              access_descr_t       access_descr)
{
    if (! (mmio->is_write))
    {
        handle_ro_access (mmio, access_descr);
    }
    else
    {
    	u32 bitmap = config_descr->addr_to_bitmap (mmio, access_descr);


        if ((bitmap != 0)                            &&
            ((mmio->value & bitmap) == mmio->value))
        {
            struct mmio_access curr_mmio;


            curr_mmio.address  = mmio->address;
            curr_mmio.size     = mmio->size;
            curr_mmio.is_write = false;

    	    /* Perform a read-update-write operation. This must be done inside a lock     */
            /* since the read-update-write operation must be atomic. In addition, this    */
            /* MMIO write operation could be executed from multiple cells simmultanously. */
	        spin_lock (&lock);
	        mmio_perform_access (virt_addr, &curr_mmio);

            mmio->value |= (curr_mmio.value & ~ (bitmap));

            mmio_perform_access (virt_addr, mmio);
        	spin_unlock (&lock);
        }
    }
}

static enum mmio_result clk_handle_access (void*                arg,
							  	  	       struct mmio_access*  mmio)
{
    access_descr_t access_descr = get_access_descr (mmio, config_descr->access_descr_map, config_descr->access_descr_map_size);


	switch (get_access_type (access_descr))
	{
        case ACCESS_RO:
            handle_ro_access (mmio, access_descr);
            break;

        case ACCESS_WO:
            handle_wo_access (mmio, access_descr);
            break;

        case ACCESS_RW:
            handle_rw_access (mmio, access_descr);
            break;

        default:
	    	/* The remaing CLK accesses shall only be performed by the root cell. */
            if (this_cell () == &root_cell)
			{
				/* No access lock is required since only the root cell is allowed */
				/* to perform the MMIO accesses. Hence, the root cell is responsible */
				/* for controlling access when multiple root cell programs try to */
				/* perform MMIO read / write operations. */
				mmio_perform_access (virt_addr, mmio);
			}
			else
			{
				if (! (mmio->is_write))
				{
					/* Simply return '0' for any read operation. */
					mmio->value = 0;
				}
			}
			break;
	}

	return (MMIO_HANDLED);
}

static int get_phys_addr (struct cell*    cell,
                          unsigned long*  phys_addr)
{
	unsigned int                   cnt;
	const struct jailhouse_vendor* vendor;


    (*phys_addr) = 0;

    FOR_EACH_VENDOR (vendor, cell->config, cnt)
	{
		if (vendor->type != JAILHOUSE_VENDOR_MTK_CLK)
		{
			continue;
		}

		if (((*phys_addr) != 0)                        &&
            (vendor->mtk_clk.address != (*phys_addr)))
		{
			return (-EINVAL);
		}

        (*phys_addr) = vendor->mtk_clk.address;
	}

    return (0);
}

int mtk_clk_cell_init (struct cell*  cell)
{
	size_t                         pos;
	unsigned int                   cnt;
    unsigned long                  address = 0;
	const struct jailhouse_vendor* vendor;


    if (config_descr == NULL)
    {
        return (-EINVAL);
    }

	FOR_EACH_VENDOR (vendor, cell->config, cnt)
	{
		if (vendor->type != JAILHOUSE_VENDOR_MTK_CLK)
		{
			continue;
		}

		/* Verify that the CLK entries in the cel description are valid. */
		if (((vendor->mtk_clk.clk_base % sizeof (vendor->mtk_clk.clk_bitmap [0])) != 0)                                      ||
		    ((vendor->mtk_clk.clk_base + (sizeof (vendor->mtk_clk.clk_bitmap) * 8)) > (sizeof (cell->arch.clk_bitmap) * 8)))
		{
			return (-EINVAL);
		}

		/* Also only one CLK address is supported. */
		if ((address != 0)                         &&
		    (vendor->mtk_clk.address != address))
		{
			return (-EINVAL);
		}
	
        address = vendor->mtk_clk.address;

		/* Copy the CLK entries. */
		for (pos = 0; pos < ARRAY_SIZE (vendor->mtk_clk.clk_bitmap); pos++)
		{
			cell->arch.clk_bitmap [(vendor->mtk_clk.clk_base / (sizeof (vendor->mtk_clk.clk_bitmap [0]) * 8)) + pos] |= vendor->mtk_clk.clk_bitmap [pos];
		}
	}

	if (address != 0)
	{
	    /* Register handler. */
		mmio_region_register (cell, address, config_descr->reg_size, clk_handle_access, (void*) address);

	    /* And lastly, remove the CLK entries from the root cell. */
		if (cell != &root_cell)
		{
			for (pos = 0; pos < ARRAY_SIZE (cell->arch.clk_bitmap); pos++)
			{
				root_cell.arch.clk_bitmap [pos] &= ~(cell->arch.clk_bitmap [pos]);
			}
    	}
	}

	return (0);
}

void mtk_clk_cell_exit (struct cell*  cell)
{
	size_t                         pos;
	unsigned int                   cnt;
	const struct jailhouse_vendor* vendor;


	if (cell != &root_cell)
	{
		/* Return the CLK id entries to the root cell. */
		for (pos = 0; pos < ARRAY_SIZE (cell->arch.clk_bitmap); pos++)
		{
			root_cell.arch.clk_bitmap [pos] |= cell->arch.clk_bitmap [pos];
		}

		/* Mask out bits which were not part of the root cell. */
		FOR_EACH_VENDOR (vendor, root_cell.config, cnt)
		{
			if (vendor->type != JAILHOUSE_VENDOR_MTK_CLK)
			{
				continue;
			}	

			for (pos = 0; pos < ARRAY_SIZE (vendor->mtk_clk.clk_bitmap); pos++)
			{
				root_cell.arch.clk_bitmap [(vendor->mtk_clk.clk_base / (sizeof (vendor->mtk_clk.clk_bitmap [0]) * 8)) + pos] &= vendor->mtk_clk.clk_bitmap [pos];
			}
		}
	}
}

unsigned int mtk_clk_mmio_count_regions (struct cell*  cell)
{
	/* Only one MMIO CLK region handler per cell. */
	return (1);
}

int mtk_clk_init (const clk_config_descr_t*  clk_config_descr)
{
	int           ret = 0;
    unsigned long phys_addr = 0;


    if (clk_config_descr == NULL)
    {
        return (-EINVAL);
    }

	if (virt_addr == NULL)
	{
		/* Create hypervisor paging for CLK access.   */
        /* This is only done once with the root cell. */
        ret = get_phys_addr (&root_cell, &phys_addr);
		if (ret != 0)
		{
			return (ret);
		}

        if (phys_addr != 0)
        {
            virt_addr = paging_map_device (phys_addr, clk_config_descr->reg_size);
            if (virt_addr == NULL)
            {
                ret = -ENOMEM;
            }
        }

        config_descr = clk_config_descr;

    	ret = mtk_clk_cell_init (&root_cell);

    	if (ret != 0)
	    {
		    /* Cleanup if cell initialization failed. */
    		if (virt_addr != NULL)
	    	{
                paging_unmap_device (phys_addr, virt_addr, clk_config_descr->reg_size);

                virt_addr = NULL;
	    	}
    	}
    }

	return (ret);
}

void mtk_clk_shutdown (void)
{
    unsigned long phys_addr;


    if (config_descr != NULL)
    {
    	if (virt_addr != NULL)
	    {
            get_phys_addr (&root_cell, &phys_addr);

            paging_unmap_device (phys_addr, virt_addr, config_descr->reg_size);

		    virt_addr = NULL;
	    }

        config_descr = NULL;
    }
}