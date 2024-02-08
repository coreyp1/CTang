
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include "tang/astNodeBlock.h"

GTA_Ast_Node_VTable gta_ast_node_block_vtable = {
  .name = "Block",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_block_destroy,
  .print = gta_ast_node_block_print,
  .simplify = gta_ast_node_block_simplify,
  .walk = gta_ast_node_block_walk,
};

GTA_Ast_Node_Block * gta_ast_node_block_create(GCU_Vector64 * statements, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Block * self = gcu_malloc(sizeof(GTA_Ast_Node_Block));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_block_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->statements = statements;
  return self;
}

void gta_ast_node_block_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  gcu_vector64_destroy(block->statements);
  gcu_free(self);
}

void gta_ast_node_block_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);
  printf("%s%s:\n", indent, self->vtable->name);
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  for (size_t i = 0; i < gcu_vector64_count(block->statements); ++i) {
    GTA_Ast_Node * statement = (GTA_Ast_Node *)block->statements->data[i].p;
    gta_ast_node_print(statement, new_indent);
  }
  gcu_free(new_indent);
}

GTA_Ast_Node * gta_ast_node_block_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  for (size_t i = 0; i < gcu_vector64_count(block->statements); ++i) {
    GTA_Ast_Node * simplified_statement = gta_ast_node_simplify((GTA_Ast_Node *)block->statements->data[i].p, variable_map);
    if (simplified_statement) {
      gta_ast_node_destroy((GTA_Ast_Node *)block->statements->data[i].p);
      block->statements->data[i].p = simplified_statement;
    }
  }
  return 0;
}

void gta_ast_node_block_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  for (size_t i = 0; i < gcu_vector64_count(block->statements); ++i) {
    GTA_Ast_Node * statement = (GTA_Ast_Node *)block->statements->data[i].p;
    gta_ast_node_walk(statement, callback, data, return_value);
  }
}
