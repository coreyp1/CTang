
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include "tang/astNodeInteger.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_integer_vtable = {
  .name = "Integer",
  .compile_to_bytecode = gta_ast_node_integer_compile_to_bytecode,
  .destroy = gta_ast_node_integer_destroy,
  .print = gta_ast_node_integer_print,
  .simplify = gta_ast_node_integer_simplify,
  .walk = gta_ast_node_integer_walk,
};

GTA_Ast_Node_Integer * gta_ast_node_integer_create(int64_t integer, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Integer * self = gcu_malloc(sizeof(GTA_Ast_Node_Integer));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Integer) {
    .base = {
      .vtable = &gta_ast_node_integer_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_INTEGER,
    },
    .value = integer,
  };
  return self;
}

void gta_ast_node_integer_destroy(GTA_Ast_Node * self) {
  gcu_free(self);
}

void gta_ast_node_integer_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Integer * integer = (GTA_Ast_Node_Integer *) self;
  printf("%s%s: %ld\n", indent, self->vtable->name, integer->value);
}

GTA_Ast_Node * gta_ast_node_integer_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}

void gta_ast_node_integer_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}

bool gta_ast_node_integer_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Integer * integer = (GTA_Ast_Node_Integer *) self;
  return GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_INTEGER))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_I(integer->value));
}
