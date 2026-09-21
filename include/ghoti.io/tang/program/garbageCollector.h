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
 * Header file for the garbage collector functionality.
 */

#ifndef GHOTI_IO_GTA_PROGRAM_GARBAGECOLLECTOR_H
#define GHOTI_IO_GTA_PROGRAM_GARBAGECOLLECTOR_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <ghoti.io/cutil/vector.h>

/**
 * A list of allocations that have been created but not yet freed.
 *
 * Each entry is a pointer to the computedValue object.
 */
typedef GTA_VectorX GTA_Garbage_Collector_Allocations_List;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_PROGRAM_GARBAGECOLLECTOR_H