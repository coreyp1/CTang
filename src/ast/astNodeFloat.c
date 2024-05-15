
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeFloat.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/program/bytecode.h>
#include <tang/program/binaryCompilerContext.h>
#include <tang/program/bytecodeCompilerContext.h>

GTA_Ast_Node_VTable gta_ast_node_float_vtable = {
  .name = "Float",
  .compile_to_bytecode = gta_ast_node_float_compile_to_bytecode,
  .compile_to_binary = gta_ast_node_float_compile_to_binary,
  .destroy = gta_ast_node_float_destroy,
  .print = gta_ast_node_float_print,
  .simplify = gta_ast_node_float_simplify,
  .analyze = 0,
  .walk = gta_ast_node_float_walk,
};


GTA_Ast_Node_Float * gta_ast_node_float_create(GCU_float64_t value, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Float * self = gcu_malloc(sizeof(GTA_Ast_Node_Float));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_float_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_FLOAT;
  self->value = value;
  return self;
}


void gta_ast_node_float_destroy(GTA_Ast_Node * self) {
  gcu_free(self);
}


void gta_ast_node_float_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Float * float_node = (GTA_Ast_Node_Float *) self;
  printf("%s%s: %f\n", indent, self->vtable->name, float_node->value);
}


GTA_Ast_Node * gta_ast_node_float_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_float_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}


bool gta_ast_node_float_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Float * float_node = (GTA_Ast_Node_Float *) self;
  return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count) && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_FLOAT))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_F(float_node->value));

  return false;
}


bool gta_ast_node_float_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  GTA_Ast_Node_Float * float_node = (GTA_Ast_Node_Float *) self;
  GCU_Vector8 * v = context->binary_vector;
  if (!gcu_vector8_reserve(v, v->count + 30)) {
    return false;
  }
#if defined(GTA_X86_64)
  // 64-bit x86
  // Assembly to call gta_computed_value_float_create(float_node->value, context):
  // context is in r15.
  //   mov rdi, r15
  //   mov rax, float_node->value
  GTA_BINARY_WRITE3(v, 0x4C, 0x89, 0xFF);
  GTA_BINARY_WRITE2(v, 0x48, 0xB8);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);

  //   mov xmm0, rax
  GTA_BINARY_WRITE5(v, 0x66, 0x48, 0x0F, 0x6E, 0xC0);
  memcpy(&v->data[v->count - 13], &float_node->value, 8);

  //   mov rax, gta_computed_value_float_create
  GTA_BINARY_WRITE2(v, 0x48, 0xB8);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  GTA_UInteger fp = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_float_create);
  memcpy(&v->data[v->count - 8], &fp, 8);

  //   call rax
  GTA_BINARY_WRITE2(v, 0xFF, 0xD0);
  return true;
#endif
  return false;
}
