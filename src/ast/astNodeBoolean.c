
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeBoolean.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_boolean_vtable = {
  .name = "Boolean",
  .compile_to_bytecode = gta_ast_node_boolean_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_boolean_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_boolean_destroy,
  .print = gta_ast_node_boolean_print,
  .simplify = gta_ast_node_boolean_simplify,
  .analyze = 0,
  .walk = gta_ast_node_boolean_walk,
};


GTA_Ast_Node_Boolean * gta_ast_node_boolean_create(bool value, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Boolean * self = gcu_malloc(sizeof(GTA_Ast_Node_Boolean));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Boolean) {
    .base = {
      .vtable = &gta_ast_node_boolean_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_BOOLEAN,
      .is_singleton = false,
    },
    .value = value,
  };
  return self;
}


void gta_ast_node_boolean_destroy(GTA_Ast_Node * self) {
  gcu_free(self);
}


void gta_ast_node_boolean_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Boolean * boolean = (GTA_Ast_Node_Boolean *) self;
  printf("%s%s: %s\n", indent, self->vtable->name, boolean->value ? "true" : "false");
}


GTA_Ast_Node * gta_ast_node_boolean_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_boolean_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}


bool gta_ast_node_boolean_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_Boolean * boolean = (GTA_Ast_Node_Boolean *) self;
  return GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_BOOLEAN))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_B(boolean->value));
}


bool gta_ast_node_boolean_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_Boolean * boolean = (GTA_Ast_Node_Boolean *) self;
  GCU_Vector8 * v = context->binary_vector;

  return gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, boolean->value
    ? (uint64_t)gta_computed_value_boolean_true
    : (uint64_t)gta_computed_value_boolean_false);
}
