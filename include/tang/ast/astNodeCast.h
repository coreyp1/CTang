/**
 * @file
 */

#ifndef GTA_AST_NODE_CAST_H
#define GTA_AST_NODE_CAST_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Cast class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_cast_vtable;

/**
 * The types of casts used by GTA_Ast_Node_Cast.
 */
typedef enum GTA_Cast_Type {
  GTA_CAST_TYPE_INTEGER, ///> int
  GTA_CAST_TYPE_FLOAT,   ///> float
  GTA_CAST_TYPE_BOOLEAN, ///> bool
  GTA_CAST_TYPE_STRING,  ///> string
} GTA_Cast_Type;

/**
 * The GTA_Ast_Node_Cast class.
 */
typedef struct GTA_Ast_Node_Cast {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The expression to cast.
   */
  GTA_Ast_Node * expression;
  /**
   * The type of cast.
   */
  GTA_Cast_Type type;
} GTA_Ast_Node_Cast;

/**
 * Creates a new GTA_Ast_Node_Cast object.
 *
 * @param expression The expression to cast.
 * @param type The type of cast.
 * @param location The location of the cast in the source code.
 * @return The new GTA_Ast_Node_Cast object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Cast * gta_ast_node_cast_create(GTA_Ast_Node * expression, GTA_Cast_Type type, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Cast object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Cast object to destroy.
 */
void gta_ast_node_cast_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Cast object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Cast object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_cast_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Cast object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Cast object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Cast object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_cast_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Cast object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Cast object to walk.
 * @param callback The callback to call for each node in the tree.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_cast_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

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
bool gta_ast_node_cast_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
bool gta_ast_node_cast_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_cast_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_CAST_H