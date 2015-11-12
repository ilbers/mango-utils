/*
 * Mango partitions configuration sctipt converter.
 *
 * Copyright (c) 2014-2015 ilbers GmbH
 * Alexander Smirnov <asmirnov@ilbers.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __MISC_SCRIPT_H__
#define __MISC_SCRIPT_H__

/* NOTE:
 *
 * the following parameters should reflect to your Mango hypervisor
 * configuration.
 */

#define MANGO_SCRIPT_MAGIC	0xaa10bb20
#define MANGO_SCRIPT_VERSION	0x04112015

#define MAX_PARTITIONS_NUMBER	8
#define MAX_PARTITION_ENTRIES	64

#define OS_NONE			0x00
#define OS_LINUX		0x10
#define OS_FREERTOS		0x20

/* The overall structure looks like the following:
 *
 * --------------------
 * |  script header   |
 * --------------------
 * | partition 1 desc |
 * --------------------
 *        ...
 * --------------------
 * | partition N desc |
 * --------------------
 * |     Raw data     | 
 * --------------------
 */

typedef struct {
    uint32_t mem_entries;
    uint32_t irq_entries;
    uint32_t os_type;
    uint32_t offset;
} script_part_t;

typedef struct {
    uint32_t magic;
    uint32_t parts_num;
    uint32_t header_size;
    uint32_t version;
} script_header_t;

#endif /* __MISC_SCRIPT_H__ */
