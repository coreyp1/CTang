
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/macros.h>
#include <tang/ast/astNodeString.h>
#include <tang/computedValue/computedValueString.h>

GTA_Ast_Node_VTable gta_ast_node_string_vtable = {
  .name = "String",
  .compile_to_bytecode = gta_ast_node_string_compile_to_bytecode,
  .compile_to_binary = gta_ast_node_string_compile_to_binary,
  .destroy = gta_ast_node_string_destroy,
  .print = gta_ast_node_string_print,
  .simplify = gta_ast_node_string_simplify,
  .analyze = 0,
  .walk = gta_ast_node_string_walk,
};

GTA_Ast_Node_String * gta_ast_node_string_create(GTA_Unicode_String * string, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_String * self = gcu_malloc(sizeof(GTA_Ast_Node_String));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_string_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_STRING;
  self->string = string;
  return self;
}

void gta_ast_node_string_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_String * string = (GTA_Ast_Node_String *)self;
  gta_unicode_string_destroy(string->string);
  gcu_free(self);
}

void gta_ast_node_string_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_String * string = (GTA_Ast_Node_String *)self;
  printf("%s%s: \"%s\"\n", indent, self->vtable->name, string->string->buffer);
}

GTA_Ast_Node * gta_ast_node_string_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}

void gta_ast_node_string_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}

bool gta_ast_node_string_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_String * string = (GTA_Ast_Node_String *)self;
  return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count) && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_STRING))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_P(string->string));
}

bool gta_ast_node_string_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  (void)self;
  (void)context;
  GTA_Ast_Node_String * string = (GTA_Ast_Node_String *)self;
  GCU_Vector8 * v = context->binary_vector;
  if (!gcu_vector8_reserve(v, v->count + 32)) {
    return false;
  }
#if GTA_X86_64
  // 64-bit x86
  // Assembly to call gta_computed_value_string_create(&string->string, 0, context):
  // context is in r15.
  //   mov rdi, string->string
  GTA_BINARY_WRITE2(v, 0x48, 0xBF);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(&v->data[v->count - 8], &string->string, 8);
  //   mov rsi, 0x0
  GTA_BINARY_WRITE2(v, 0x48, 0xC7);
  GTA_BINARY_WRITE5(v, 0xC6, 0x00, 0x00, 0x00, 0x00);
  //   mov rdx, r15
  GTA_BINARY_WRITE3(v, 0x4C, 0x89, 0xFA);
  //   mov rax, gta_computed_value_string_create
  GTA_BINARY_WRITE2(v, 0x48, 0xB8);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  GTA_UInteger fp = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_string_create);
  memcpy(&v->data[v->count - 8], &fp, 8);
  //   call rax
  GTA_BINARY_WRITE2(v, 0xFF, 0xD0);
  return true;
#endif

  return false;
}
