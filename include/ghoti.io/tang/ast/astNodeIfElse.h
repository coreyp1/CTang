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

#ifndef GHOTI_IO_GTA_AST_ASTNODEIFELSE_H
#define GHOTI_IO_GTA_AST_ASTNODEIFELSE_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <ghoti.io/tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_If_Else class.
 */
GTA_API extern GTA_Ast_Node_VTable gta_ast_node_if_else_vtable;

/**
 * The GTA_Ast_Node_If_Else class.
 */
struct GTA_Ast_Node_If_Else {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The condition of the if-else statement.
   */
  GTA_Ast_Node * condition;
  /**
   * The block of the if-else statement.
   */
  GTA_Ast_Node * ifBlock;
  /**
   * The else block of the if-else statement.
   */
  GTA_Ast_Node * elseBlock;
};

/**
 * Creates a new GTA_Ast_Node_If_Else object.
 *
 * @param condition The condition of the if-else statement.
 * @param ifBlock The block of the if-else statement.
 * @param elseBlock The else block of the if-else statement.
 * @param location The location of the if-else statement in the source code.
 * @return The new GTA_Ast_Node_If_Else object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node_If_Else * gta_ast_node_if_else_create(GTA_Ast_Node * condition, GTA_Ast_Node * ifBlock, GTA_Ast_Node * elseBlock, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_If_Else object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to destroy.
 */
GTA_API void gta_ast_node_if_else_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_If_Else object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to print.
 * @param indent The string to print before each line of output.
 */
GTA_API void gta_ast_node_if_else_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_If_Else object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_If_Else object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_if_else_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Perform pre-compilation analysis on the AST node.
 *
 * This step includes allocating constants, identifying libraries and variables
 * (global and local), and creating namespace scopes for functions.
 *
 * This function serves as a general dispatch function, and should be used in
 * preference to calling the vtable's analyze function directly.
 *
 * @see gta_ast_node_analyze()
 *
 * @param self The node to analyze.
 * @param program The program that the node is part of.
 * @param scope The current variable scope.
 * @return NULL on success, otherwise return a parse error.
 */
GTA_API GTA_Ast_Node * gta_ast_node_if_else_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

/**
 * Walks a GTA_Ast_Node_If_Else object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to walk.
 * @param callback The callback to call for each node in the tree.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
GTA_API void gta_ast_node_if_else_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compile the AST node to binary for x86_64.
 *
 * The vtable's compile_to_binary function is called to compile the node.  This
 * function serves as a general dispatch function, and should be used in
 * preference to calling the vtable's compile_to_binary function directly.
 *
 * @see gta_ast_node_compile_to_binary__x86_64
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
GTA_API bool gta_ast_node_if_else_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

/**
 * Compiles a GTA_Ast_Node_Integer object to bytecode.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_bytecode()
 * instead.
 *
 * @see gta_ast_node_compile_to_bytecode
 *
 * @param self The GTA_Ast_Node_Integer object.
 * @param context The compiler state to use for compilation.
 */
GTA_API bool gta_ast_node_if_else_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GHOTI_IO_GTA_AST_ASTNODEIFELSE_H