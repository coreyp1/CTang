/**
 * @file
 *
 * Helper functions for parsing Tang code.
 */

#ifndef GHOTI_IO_GTA_TANGLANGUAGE_H
#define GHOTI_IO_GTA_TANGLANGUAGE_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdbool.h>
#include <ghoti.io/tang/ast/astNode.h>


GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_parse(const char * source, bool is_template);
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_parse_script(const char * source);
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_parse_template(const char * source);
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_primary_parse(const char * source, bool is_template);
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_tang_simplify(GTA_Ast_Node * node);
GTA_API size_t gta_tang_node_count(GTA_Ast_Node * node);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GHOTI_IO_GTA_TANGLANGUAGE_H
