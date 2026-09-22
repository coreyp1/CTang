/*
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * Copyright (C) 2024-2026 Corey Pennycuff
 *
 * This file is part of Ghoti.io Tang.
 *
 * Ghoti.io Tang is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Ghoti.io Tang is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * This library's allocator, in the form cutil's generic containers accept.
 */

#ifndef GHOTI_IO_GTA_ALLOCATOR_H
#define GHOTI_IO_GTA_ALLOCATOR_H

#include <ghoti.io/cutil/allocator.h>
#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Get the allocator that this library allocates through.
 *
 * Every buffer Tang hands back is obtained with `gcu_malloc()` and must be
 * released with `gcu_free()`, so that the allocation counts cutil keeps under
 * `GHOTIIO_CUTIL_ENABLE_MEMORY_DEBUG` stay balanced. Those two are not
 * interchangeable with libc's `malloc()` and `free()`, and mixing them is
 * invisible until a leak check reports a leak that is not there.
 *
 * cutil's own containers default to `gcu_allocator_default()`, which is
 * stdlib-backed and therefore the wrong one for this library: a buffer taken
 * from a ::GCU_Array built on the default allocator would be `malloc()`'d, and
 * the caller Tang handed it to would `gcu_free()` it. Pass this allocator
 * wherever a cutil container is created, so that what comes out of it can be
 * released the same way as everything else Tang allocates.
 *
 * @return A pointer to this library's allocator. It is a constant with static
 *   storage duration, so it outlives any container built on it and is never
 *   freed.
 */
GTA_API const GCU_Allocator * gta_allocator(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_ALLOCATOR_H
