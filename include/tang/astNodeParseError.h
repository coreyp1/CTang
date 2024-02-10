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

#include "tang/astNode.h"

extern GTA_Ast_Node_VTable gta_ast_node_parse_error_vtable;

typedef struct astNodeParseError {
  GTA_Ast_Node base;
  char * message;
} GTA_Ast_Node_Parse_Error;

GTA_Ast_Node_Parse_Error * gta_ast_node_parse_error_create(const char * message, GTA_PARSER_LTYPE location);

void gta_ast_node_parse_error_destroy(GTA_Ast_Node * parse_error);

void gta_ast_node_parse_error_print(GTA_Ast_Node * parse_error, const char * indent);

GTA_Ast_Node * gta_ast_node_parse_error_simplify(GTA_Ast_Node * parse_error, GTA_Ast_Simplify_Variable_Map * variable_map);

void gta_ast_node_parse_error_walk(GTA_Ast_Node * parse_error, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_AST_NODE_PARSE_ERROR_H
