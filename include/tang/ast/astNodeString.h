/**
 * @file
 */

#ifndef GTA_AST_NODE_STRING_H
#define GTA_AST_NODE_STRING_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_String class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_string_vtable;

/**
 * The GTA_Ast_Node_String class.
 */
typedef struct GTA_Ast_Node_String {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The string.
   */
  GTA_Unicode_String * string;
} GTA_Ast_Node_String;

/**
 * Creates a new GTA_Ast_Node_String object.
 *
 * @param string The string.
 * @param location The location of the string in the source code.
 * @return The new GTA_Ast_Node_String object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_String * gta_ast_node_string_create(GTA_Unicode_String * string, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_String object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_String object to destroy.
 */
void gta_ast_node_string_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_String object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_String object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_string_print(GTA_Ast_Node * self, const char * indent);

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
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_string_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

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
void gta_ast_node_string_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compile a GTA_Ast_Node_String object to bytecode.
 *
 * @see gta_ast_node_compile_to_bytecode
 *
 * @param self The GTA_Ast_Node_String object to compile.
 * @param context The bytecode compiler context.
 * @return true on success, false on failure.
 */
bool gta_ast_node_string_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

/**
 * Compile a GTA_Ast_Node_String object to binary for x86_64.
 *
 * @see gta_ast_node_compile_to_binary__x86_64
 *
 * @param self The GTA_Ast_Node_String object to compile.
 * @param context The binary compiler context.
 * @return true on success, false on failure.
 */
bool gta_ast_node_string_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_STRING_H