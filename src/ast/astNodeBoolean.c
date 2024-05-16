
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeBoolean.h>
#include <tang/computedValue/computedValueBoolean.h>

GTA_Ast_Node_VTable gta_ast_node_boolean_vtable = {
  .name = "Boolean",
  .compile_to_bytecode = gta_ast_node_boolean_compile_to_bytecode,
  .compile_to_binary = gta_ast_node_boolean_compile_to_binary,
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


bool gta_ast_node_boolean_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Boolean * boolean = (GTA_Ast_Node_Boolean *) self;
  return GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_BOOLEAN))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_B(boolean->value));
}


bool gta_ast_node_boolean_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  GTA_Ast_Node_Boolean * boolean = (GTA_Ast_Node_Boolean *) self;
  GCU_Vector8 * v = context->binary_vector;
  if (!gcu_vector8_reserve(v, v->count + 25)) {
    return false;
  }
#if defined(GTA_X86_64)
  // 64-bit x86
  // TODO: Replace with a branch-free version using cmov and the singleton
  //   objects directly (rather than calling a function).
  // Assembly to call gta_computed_value_boolean_create(boolean->value, context):
  // context is in r15
  //   mov rdi, r15
  GTA_BINARY_WRITE3(v, 0x4C, 0x89, 0xFF);
  //   mov rsi, boolean->value
  GTA_BINARY_WRITE2(v, 0x48, 0xBF);
  GTA_BINARY_WRITE8(v, boolean->value ? 0x01 : 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  //   mov rax, gta_computed_value_boolean_create
  GTA_BINARY_WRITE2(v, 0x48, 0xB8);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  GTA_UInteger fp = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_boolean_create);
  memcpy(&v->data[v->count - 8], &fp, 8);
  //   call rax
  GTA_BINARY_WRITE2(v, 0xFF, 0xD0);
  return true;
#endif
  return false;
}