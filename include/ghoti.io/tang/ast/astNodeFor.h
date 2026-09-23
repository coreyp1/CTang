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

#ifndef GHOTI_IO_GTA_AST_ASTNODEFOR_H
#define GHOTI_IO_GTA_AST_ASTNODEFOR_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <ghoti.io/tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_For class.
 */
GTA_API extern GTA_Ast_Node_VTable gta_ast_node_for_vtable;

/**
 * The GTA_Ast_Node_For class.
 */
struct GTA_Ast_Node_For {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The initialization of the for loop.
   */
  GTA_Ast_Node * init;
  /**
   * The condition of the for loop.
   */
  GTA_Ast_Node * condition;
  /**
   * The update of the for loop.
   */
  GTA_Ast_Node * update;
  /**
   * The block of the for loop.
   */
  GTA_Ast_Node * block;
};

/**
 * Creates a new GTA_Ast_Node_For object.
 *
 * @param init The initialization of the for loop.
 * @param condition The condition of the for loop.
 * @param update The update of the for loop.
 * @param block The block of the for loop.
 * @param location The location of the for loop in the source code.
 * @return The new GTA_Ast_Node_For object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node_For * gta_ast_node_for_create(GTA_Ast_Node * init, GTA_Ast_Node * condition, GTA_Ast_Node * update, GTA_Ast_Node * block, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_For object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_For object to destroy.
 */
GTA_API void gta_ast_node_for_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_For object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_For object to print.
 * @param indent The string to print before each line of output.
 */
GTA_API void gta_ast_node_for_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_For object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_For object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_For object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_for_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_For object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_For object to walk.
 * @param callback The callback function to call for each node in the tree.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
GTA_API void gta_ast_node_for_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Perform pre-compilation analysis on the AST node.
 *
 * This step includes allocating constants, identifying libraries and variables
 * (global and local), and creating namespace scopes for functions.
 *
 * This function should not be called directly. Use gta_ast_node_analyze()
 * instead.
 *
 * @param self The node to analyze.
 * @param program The program that the node is part of.
 * @return NULL on success, otherwise return a parse error.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_for_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

/**
 * Compile the AST node to binary for x86_64.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_binary()
 * instead.
 *
 * @see gta_ast_node_compile_to_binary__x86_64
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
GTA_API bool gta_ast_node_for_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

/**
 * Compiles the AST node to bytecode.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_bytecode()
 * instead.
 *
 * @see gta_ast_node_compile_to_bytecode
 *
 * @param self The node to compile.
 * @param context The compiler state to use for compilation.
 */
GTA_API bool gta_ast_node_for_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GHOTI_IO_GTA_AST_ASTNODEFOR_H
