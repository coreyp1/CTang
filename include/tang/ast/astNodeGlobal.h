/**
 * @file
 */

#ifndef GTA_AST_NODE_GLOBAL_H
#define GTA_AST_NODE_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Global class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_global_vtable;

/**
 * The GTA_Ast_Node_Global class.
 */
typedef struct GTA_Ast_Node_Global {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The identifier.
   */
  GTA_Ast_Node * identifier;
  /**
   * The assignment (if present).
   */
  GTA_Ast_Node * assignment;
} GTA_Ast_Node_Global;

/**
 * Creates a new GTA_Ast_Node_Global object.
 *
 * @param identifier The identifier.
 * @param assignment The assignment (if present).
 * @param location The location of the global in the source code.
 * @return The new GTA_Ast_Node_Global object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Global * gta_ast_node_global_create(GTA_Ast_Node * identifier, GTA_Ast_Node * assignment, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Global object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Global object to destroy.
 */
void gta_ast_node_global_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Global object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Global object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_global_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Global object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Global object to simplify.
 * @param variable_map The variable map.
 * @return The simplified GTA_Ast_Node_Global object.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_global_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

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
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_global_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

/**
 * Walks a GTA_Ast_Node_Global object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Global object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_global_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compiles a GTA_Ast_Node_Global object to binary for x86_64.
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
bool gta_ast_node_global_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

/**
 * Compiles a GTA_Ast_Node_Global object to bytecode.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_bytecode()
 * instead.
 *
 * @see gta_ast_node_compile_to_bytecode
 *
 * @param self The GTA_Ast_Node_Global object.
 * @param context The compiler state to use for compilation.
 */
bool gta_ast_node_global_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_GLOBAL_H
