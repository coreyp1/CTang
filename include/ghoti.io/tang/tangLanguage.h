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
 * Helper functions for parsing Tang code.
 */

#ifndef GHOTI_IO_GTA_TANGLANGUAGE_H
#define GHOTI_IO_GTA_TANGLANGUAGE_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdbool.h>
#include <ghoti.io/tang/ast/astNode.h>


GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_parse(const char * source, bool is_template);
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_parse_script(const char * source);
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_parse_template(const char * source);
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_primary_parse(const char * source, bool is_template);
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_simplify(GTA_Ast_Node * node);
GTA_API size_t gta_tang_node_count(GTA_Ast_Node * node);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GHOTI_IO_GTA_TANGLANGUAGE_H
