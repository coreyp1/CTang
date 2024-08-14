/**
 * @file
 *
 * This file contains the definition of the class astNodeParseError.
 *
 * This class is only used to represent a parse error when trying to parse
 * code into an AST.
 */

#ifndef TANG_AST_NODE_PARSE_ERROR_H
#define TANG_AST_NODE_PARSE_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Parse_Error class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_parse_error_vtable;


/**
 * An error representing the fact that an out of memory error occurred when
 * attempting to create a parse error.
 */
extern GTA_Ast_Node * gta_ast_node_parse_error_out_of_memory;


/**
 * An error representing the fact that a function has been redeclared.
 */
extern GTA_Ast_Node * gta_ast_node_parse_error_function_redeclared;


/**
 * An error representing the fact that an identifier has been redeclared.
 */
extern GTA_Ast_Node * gta_ast_node_parse_error_identifier_redeclared;


/**
 * An error representing the fact that a global identifier has been redeclared.
 */
extern GTA_Ast_Node * gta_ast_node_parse_error_global_identifier_redeclared;


/**
 * The GTA_Ast_Node_Parse_Error class.
 */
typedef struct GTA_Ast_Node_Parse_Error {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The message of the parse error.
   */
  char * message;
} GTA_Ast_Node_Parse_Error;


/**
 * Creates a new GTA_Ast_Node_Parse_Error object.
 *
 * @param message The message of the parse error.
 * @param location The location of the parse error in the source code.
 * @return The new GTA_Ast_Node_Parse_Error object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Parse_Error * gta_ast_node_parse_error_create(const char * message, GTA_PARSER_LTYPE location);


/**
 * Destroys a GTA_Ast_Node_Parse_Error object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param parse_error The GTA_Ast_Node_Parse_Error object to destroy.
 */
void gta_ast_node_parse_error_destroy(GTA_Ast_Node * parse_error);


/**
 * Prints a GTA_Ast_Node_Parse_Error object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param parse_error The GTA_Ast_Node_Parse_Error object to print.
 * @param indent The number of spaces to indent the output.
 */
void gta_ast_node_parse_error_print(GTA_Ast_Node * parse_error, const char * indent);


/**
 * Simplifies a GTA_Ast_Node_Parse_Error object.
 *
 * @param parse_error The GTA_Ast_Node_Parse_Error object to simplify.
 * @param variable_map The variable map to use when simplifying the parse error.
 * @return The simplified GTA_Ast_Node_Parse_Error object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_parse_error_simplify(GTA_Ast_Node * parse_error, GTA_Ast_Simplify_Variable_Map * variable_map);


/**
 * Walks a GTA_Ast_Node_Parse_Error object.
 *
 * @param parse_error The GTA_Ast_Node_Parse_Error object to walk.
 * @param callback The callback to call for each node in the parse error.
 * @param data The data to pass to the callback.
 * @param return_value The value to return from the walk function.
 */
void gta_ast_node_parse_error_walk(GTA_Ast_Node * parse_error, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_AST_NODE_PARSE_ERROR_H
