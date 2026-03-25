/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) 2025 MediaTek
 *
 * Common MediaTek macro's and definitions.
 *
 * Authors:
 *   Felix Freimann <felix.freimann@mediatek.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <jailhouse/mmio.h>
#include <jailhouse/types.h>


#define FOR_EACH_VENDOR(_vendor, _config, _cnt)                     \
	for ((_vendor) = jailhouse_cell_vendors(_config), (_cnt) = 0;   \
	     (_cnt) < (_config)->num_vendors;                           \
	     (_vendor)++, (_cnt)++)

#define BITS_PER_U32  (sizeof (u32) * 8)

/* A access descriptor is a 16 bit value. This 16 bits are arranged as follows: */
/*   [ 2..0]:  Access type (e.g. RO, WO, RW etc.)                               */
/*   [ 6..3]:  Number of bits per pin (e.g. 1, 2, 3 etc.)                       */
/*   [13..7]:  Distance from base register                                      */
/* Access types: */
#define ACCESS_ROOT  (0)
#define ACCESS_RO    (1)
#define ACCESS_WO    (2)
#define ACCESS_RW    (3)

#define ACCESS_TYPE_MASK   ((access_descr_t) 0x0007)
#define ACCESS_TYPE_SHIFT  (0)

/* Number of bits per pin: */
#define ACCESS_ONE_BIT     (1)
#define ACCESS_TWO_BITS    (2)
#define ACCESS_THREE_BITS  (3)
#define ACCESS_FOUR_BITS   (4)

#define ACCESS_NUM_OF_BITS_MASK   ((access_descr_t) 0x000f)
#define ACCESS_NUM_OF_BITS_SHIFT  (3)

/* Distance from base register: */
#define ACCESS_DIST_IDX_MASK   ((access_descr_t) 0x003f)
#define ACCESS_DIST_IDX_SHIFT  (7)

#define ACCESS_TYPE(_descr)         ((_descr >> ACCESS_TYPE_SHIFT) & ACCESS_TYPE_MASK)
#define ACCESS_NUM_OF_BITS(_descr)  ((_descr >> ACCESS_NUM_OF_BITS_SHIFT) & ACCESS_NUM_OF_BITS_MASK)
#define ACCESS_DIST_IDX(_descr)     ((_descr >> ACCESS_DIST_IDX_SHIFT) & ACCESS_DIST_IDX_MASK)


/* Note that the macros 'REG_NAME' and 'REG_DIST_IDX_SHIFT' must be */
/* defined by the source file which uses the macro ACCESS_DESCR.    */
#define ACCESS_DESCR_EMPTY  ((u16) 0)
#define ACCESS_DESCR(_name, _base, _type, _num_of_bits)                                                                                  \
    (access_descr_t) (((_type & ACCESS_TYPE_MASK) << ACCESS_TYPE_SHIFT) |                                                                \
                      ((_num_of_bits & ACCESS_NUM_OF_BITS_MASK) << ACCESS_NUM_OF_BITS_SHIFT) |                                           \
                      ((((REG_NAME (_name) - REG_NAME (_base)) >> REG_DIST_IDX_SHIFT) & ACCESS_DIST_IDX_MASK) << ACCESS_DIST_IDX_SHIFT))

#define ACCESS_DESCR_WITH_IDX(_name, _idx, _type, _num_of_bits)                                                                          \
    (access_descr_t) (((_type & ACCESS_TYPE_MASK) << ACCESS_TYPE_SHIFT) |                                                                \
                      ((_num_of_bits & ACCESS_NUM_OF_BITS_MASK) << ACCESS_NUM_OF_BITS_SHIFT) |                                           \
                      ((_idx & ACCESS_DIST_IDX_MASK) << ACCESS_DIST_IDX_SHIFT))


typedef u16  access_descr_t;

typedef struct
{
    u32  reg_start;
    u32  reg_end;

    const access_descr_t*  access_descr;
}   access_descr_map_t;

typedef struct
{
    u32  reg;
    u32  size;
    u32  mask;
}   irq_descr_map_t;


static inline u32 get_access_type (access_descr_t  access_descr)
{
    return ((u32) ACCESS_TYPE (access_descr));
}

static inline u32 get_access_num_of_bits (access_descr_t  access_descr)
{
    return ((u32) ACCESS_NUM_OF_BITS (access_descr));
}

static inline u32 get_access_dist_idx (access_descr_t  access_descr)
{
    return ((u32) ACCESS_DIST_IDX (access_descr));
}

access_descr_t get_access_descr (struct mmio_access*        mmio,
                                 const access_descr_map_t*  access_descr_map,
                                 size_t                     access_descr_map_size);
