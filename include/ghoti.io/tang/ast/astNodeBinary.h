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

#ifndef GHOTI_IO_GTA_AST_ASTNODEBINARY_H
#define GHOTI_IO_GTA_AST_ASTNODEBINARY_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <ghoti.io/tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Binary class.
 */
GTA_API extern GTA_Ast_Node_VTable gta_ast_node_binary_vtable;

/**
 * The types of binary operators used by GTA_Ast_Node_Binary.
 */
typedef enum GTA_Binary_Type {
  GTA_BINARY_TYPE_ADD,                ///> +
  GTA_BINARY_TYPE_SUBTRACT,           ///> -
  GTA_BINARY_TYPE_MULTIPLY,           ///> *
  GTA_BINARY_TYPE_DIVIDE,             ///> /
  GTA_BINARY_TYPE_MODULO,             ///> %
  GTA_BINARY_TYPE_LESS_THAN,          ///> <
  GTA_BINARY_TYPE_LESS_THAN_EQUAL,    ///> <=
  GTA_BINARY_TYPE_GREATER_THAN,       ///> >
  GTA_BINARY_TYPE_GREATER_THAN_EQUAL, ///> >=
  GTA_BINARY_TYPE_EQUAL,              ///> ==
  GTA_BINARY_TYPE_NOT_EQUAL,          ///> !=
  GTA_BINARY_TYPE_AND,                ///> &&
  GTA_BINARY_TYPE_OR,                 ///> ||
} GTA_Binary_Type;

/**
 * The GTA_Ast_Node_Binary class.
 */
struct GTA_Ast_Node_Binary {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The left-hand side of the binary operation.
   */
  GTA_Ast_Node * lhs;
  /**
   * The right-hand side of the binary operation.
   */
  GTA_Ast_Node * rhs;
  /**
   * The type of binary operation.
   */
  GTA_Binary_Type operator_type;
};

/**
 * Creates a new GTA_Ast_Node_Binary object.
 *
 * @param lhs The left-hand side of the binary operation.
 * @param rhs The right-hand side of the binary operation.
 * @param operator_type The type of binary operation.
 * @param location The location of the binary operation in the source code.
 * @return The new GTA_Ast_Node_Binary object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node_Binary * gta_ast_node_binary_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_Binary_Type operator_type, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Binary object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Binary object to destroy.
 */
GTA_API void gta_ast_node_binary_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Binary object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Binary object to print.
 * @param indent The string to print before each line of output.
 */
GTA_API void gta_ast_node_binary_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Binary object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Binary object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Binary object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_binary_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Binary object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Binary object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
GTA_API void gta_ast_node_binary_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compile the AST node to binary for x86_64.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_binary()
 * instead.
 *
 * @see gta_ast_node_compile_to_binary
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
GTA_API bool gta_ast_node_binary_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
GTA_API bool gta_ast_node_binary_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_binary_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GHOTI_IO_GTA_AST_ASTNODEBINARY_H