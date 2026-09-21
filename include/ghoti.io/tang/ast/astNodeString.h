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
 */

#ifndef GHOTI_IO_GTA_AST_ASTNODESTRING_H
#define GHOTI_IO_GTA_AST_ASTNODESTRING_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <ghoti.io/tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_String class.
 */
GTA_API extern GTA_Ast_Node_VTable gta_ast_node_string_vtable;

/**
 * The GTA_Ast_Node_String class.
 */
struct GTA_Ast_Node_String {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The string.
   */
  GTA_Unicode_String * string;
};

/**
 * Creates a new GTA_Ast_Node_String object.
 *
 * @param string The string.
 * @param location The location of the string in the source code.
 * @return The new GTA_Ast_Node_String object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node_String * gta_ast_node_string_create(GTA_Unicode_String * string, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_String object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_String object to destroy.
 */
GTA_API void gta_ast_node_string_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_String object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_String object to print.
 * @param indent The string to print before each line of output.
 */
GTA_API void gta_ast_node_string_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_String object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_String object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_String object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_string_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_String object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_String object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
GTA_API void gta_ast_node_string_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compile a GTA_Ast_Node_String object to bytecode.
 *
 * @see gta_ast_node_compile_to_bytecode
 *
 * @param self The GTA_Ast_Node_String object to compile.
 * @param context The bytecode compiler context.
 * @return true on success, false on failure.
 */
GTA_API bool gta_ast_node_string_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

/**
 * Compile a GTA_Ast_Node_String object to binary for x86_64.
 *
 * @see gta_ast_node_compile_to_binary__x86_64
 *
 * @param self The GTA_Ast_Node_String object to compile.
 * @param context The binary compiler context.
 * @return true on success, false on failure.
 */
GTA_API bool gta_ast_node_string_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GHOTI_IO_GTA_AST_ASTNODESTRING_H