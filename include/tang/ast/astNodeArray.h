/**
 * @file
 */

#ifndef GTA_AST_NODE_ARRAY_H
#define GTA_AST_NODE_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Array class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_array_vtable;

/**
 * The GTA_Ast_Node_Array class.
 */
struct GTA_Ast_Node_Array {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The elements of the array.
   */
  GTA_VectorX * elements;
};

/**
 * Creates a new GTA_Ast_Node_Array object.
 *
 * @param elements The elements of the array.
 * @param location The location of the array in the source code.
 * @return The new GTA_Ast_Node_Array object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Array * gta_ast_node_array_create(GTA_VectorX * elements, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Array object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Array object to destroy.
 */
void gta_ast_node_array_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Array object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Array object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_array_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Array object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Array object to simplify.
 * @param variable_map A map of variable names to their values.
 * @return The simplified GTA_Ast_Node_Array object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_array_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Array object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Array object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_array_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

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
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_array_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

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
bool gta_ast_node_array_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
bool gta_ast_node_array_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_ARRAY_H