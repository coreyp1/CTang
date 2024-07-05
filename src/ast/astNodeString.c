
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeString.h>
#include <tang/program/binary.h>
#include <tang/computedValue/computedValueString.h>

GTA_Ast_Node_VTable gta_ast_node_string_vtable = {
  .name = "String",
  .compile_to_bytecode = gta_ast_node_string_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_string_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
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
  *self = (GTA_Ast_Node_String) {
    .base = {
      .vtable = &gta_ast_node_string_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_STRING,
      .is_singleton = false,
    },
    .string = string,
  };
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


bool gta_ast_node_string_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_String * string = (GTA_Ast_Node_String *)self;
  return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_STRING))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_P(string->string));
}


bool gta_ast_node_string_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_MAYBE_UNUSED(GTA_Compiler_Context * context)) {
  GTA_Ast_Node_String * string = (GTA_Ast_Node_String *)self;
  GCU_Vector8 * v = context->binary_vector;

  return true
  // Set up for a function call.
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFFFFFFFFFF0
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, 0xFFFFFFF0)
  // gta_computed_value_string_create(&string->string, 0, context):
  //   mov rdi, string->string
  //   mov rsi, 0x0
  //   mov rdx, r15
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDI, (int64_t)string->string)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RSI, 0)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RDX, GTA_REG_R15)
  //   mov rax, gta_computed_value_string_create
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (int64_t)gta_computed_value_string_create)
  //   call rax
    && gta_call_reg__x86_64(v, GTA_REG_RAX)
  // Tear down the function call.
  //   mov rsp, rbp
  //   pop rbp
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP);
}
