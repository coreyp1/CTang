/**
 * @file
 */

#ifndef GTA_AST_NODE_UNARY_H
#define GTA_AST_NODE_UNARY_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Unary class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_unary_vtable;

/**
 * The types of unary operators used by GTA_Ast_Node_Unary.
 */
typedef enum GTA_Unary_Type {
  GTA_UNARY_TYPE_NEGATIVE, ///> -
  GTA_UNARY_TYPE_NOT,      ///> !
} GTA_Unary_Type;

/**
 * The GTA_Ast_Node_Unary class.
 */
struct GTA_Ast_Node_Unary {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The expression to apply the unary operation to.
   */
  GTA_Ast_Node * expression;
  /**
   * The type of unary operation.
   */
  GTA_Unary_Type operator_type;
};

/**
 * Creates a new GTA_Ast_Node_Unary object.
 *
 * @param expression The expression to apply the unary operation to.
 * @param operator_type The type of unary operation.
 * @param location The location of the unary operation in the source code.
 * @return The new GTA_Ast_Node_Unary object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Unary * gta_ast_node_unary_create(GTA_Ast_Node * expression, GTA_Unary_Type operator_type, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Unary object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Unary object to destroy.
 */
void gta_ast_node_unary_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Unary object to a string.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Unary object.
 * @param indent The indentation level.
 * @return The string or NULL on failure.
 */
void gta_ast_node_unary_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Unary object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Unary object.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Unary object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_unary_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Unary object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Unary object.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_unary_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

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
bool gta_ast_node_unary_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
bool gta_ast_node_unary_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_unary_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_UNARY_H