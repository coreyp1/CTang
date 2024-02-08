/**
 * @file
 *
 * Helper functions for parsing Tang code.
 */

#ifndef GTA_TANG_LANGUAGE_H
#define GTA_TANG_LANGUAGE_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"


GTA_Ast_Node * gta_tang_parse(const char * source);
GTA_Ast_Node * gta_tang_primary_parse(const char * source);
GTA_Ast_Node * gta_tang_simplify(GTA_Ast_Node * node);
size_t gta_tang_node_count(GTA_Ast_Node * node);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_TANG_LANGUAGE_H
