
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeBreak.h>

GTA_Ast_Node_VTable gta_ast_node_break_vtable = {
  .name = "Break",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_break_destroy,
  .print = gta_ast_node_break_print,
  .simplify = gta_ast_node_break_simplify,
  .analyze = 0,
  .walk = gta_ast_node_break_walk,
};


GTA_Ast_Node_Break * gta_ast_node_break_create(GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Break * self = gcu_malloc(sizeof(GTA_Ast_Node_Break));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Break) {
    .base = {
      .vtable = &gta_ast_node_break_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
  };
  return self;
}


void gta_ast_node_break_destroy(GTA_Ast_Node * self) {
  gcu_free(self);
}


void gta_ast_node_break_print(GTA_Ast_Node * self, const char * indent) {
  printf("%s%s\n", indent, self->vtable->name);
}


GTA_Ast_Node * gta_ast_node_break_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_break_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}
