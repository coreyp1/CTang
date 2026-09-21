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
 * The virtual machine is used to execute the bytecode, which is the fallback
 * in the event that the JIT compiler is not available for the target platform.
 */

#ifndef GHOTI_IO_GTA_PROGRAM_VIRTUALMACHINE_H
#define GHOTI_IO_GTA_PROGRAM_VIRTUALMACHINE_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <ghoti.io/tang/program/executionContext.h>
#include <ghoti.io/tang/computedValue/computedValue.h>
#include <ghoti.io/tang/program/program.h>

GTA_API bool gta_virtual_machine_execute_bytecode(GTA_Execution_Context* context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_PROGRAM_VIRTUALMACHINE_H
