
#include <assert.h>
#include <tang/tangLanguage.h>
#include <tang/tangScanner.h>

#define YY_HEADER_EXPORT_START_CONDITIONS
#include "flexTangScanner.h"
#undef YY_HEADER_EXPORT_START_CONDITIONS

#include <tang/macros.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeParseError.h>

GTA_Ast_Node * gta_tang_parse(const char * source, bool is_template) {
  GTA_Ast_Node * primary = gta_tang_primary_parse(source, is_template);
  return primary;
    // NOTE: temporarily diabled because it does not pick up on global variable changes.
    // ? gta_tang_simplify(primary)
    // : 0;
}


void gta_flex_set_state(int state, void * yyscanner);


GTA_Ast_Node * gta_tang_primary_parse(const char * source, bool is_template) {
  assert(source);
  GTA_Ast_Node * ast = 0;
  GTA_Parser_Error parseError = 0;

  yyscan_t scanner;
  YY_BUFFER_STATE state;
  if (gta_flexlex_init(&scanner)) {
    return 0;
  }

  if (is_template) {
    gta_flex_set_state(TEMPLATE, scanner);
  }

  state = gta_flex_scan_string(source, scanner);
  GTA_Parser_parse(scanner, &ast, &parseError);

  gta_flex_delete_buffer(state, scanner);
  gta_flexlex_destroy(scanner);
  return ast;
}


GTA_Ast_Node * gta_tang_parse_script(const char * source) {
  return gta_tang_primary_parse(source, false);
}


GTA_Ast_Node * gta_tang_parse_template(const char * source) {
  return gta_tang_primary_parse(source, true);
}


static void count_variables(GTA_Ast_Node * self, GTA_MAYBE_UNUSED(void * data), void * return_value) {
  assert(self);
  assert(return_value);
  if (GTA_AST_IS_IDENTIFIER(self)) {
    ++*((size_t *) return_value);
  }
}


GTA_Ast_Node * gta_tang_simplify(GTA_Ast_Node * node) {
  // Count the number of identifiers in the tree in order to pre-allocate space
  // in the variable map.  Although there may be overcount (e.g. if the same
  // variable is used multiple times), we are getting a worse-case count so
  // that we can guarantee that the hash table will not need to be resized.
  assert(node);

  size_t variable_count = 0;
  gta_ast_node_walk(node, count_variables, 0, &variable_count);
  GTA_Ast_Simplify_Variable_Map * variable_map = gcu_hash64_create(variable_count);
  if (!variable_map) {
    return 0;
  }
  GTA_Ast_Node * simplified = gta_ast_node_simplify(node, variable_map);
  gcu_hash64_destroy(variable_map);
  if (simplified) {
    gta_ast_node_destroy(node);
    return simplified;
  }
  return node;
}


void count_nodes(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(void * data), void * return_value) {
  assert(return_value);
  size_t * count = (size_t *) return_value;
  (*count)++;
}


size_t gta_tang_node_count(GTA_Ast_Node * node) {
  size_t count = 0;
  if (node) {
    gta_ast_node_walk(node, count_nodes, 0, &count);
  }
  return count;
}
