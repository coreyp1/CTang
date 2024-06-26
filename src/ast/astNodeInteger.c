
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeInteger.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_integer_vtable = {
  .name = "Integer",
  .compile_to_bytecode = gta_ast_node_integer_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_integer_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_integer_destroy,
  .print = gta_ast_node_integer_print,
  .simplify = gta_ast_node_integer_simplify,
  .analyze = 0,
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
      .is_singleton = false,
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
  return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_INTEGER))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_I(integer->value));
}


bool gta_ast_node_integer_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_MAYBE_UNUSED(GTA_Binary_Compiler_Context * context)) {
  GTA_Ast_Node_Integer * integer = (GTA_Ast_Node_Integer *) self;
  GCU_Vector8 * v = context->binary_vector;

  return true
    // Set up for a function call.
    //   push rbp
    //   mov rbp, rsp
    //   and rsp, 0xFFFFFFFFFFFFFFF0
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, (int32_t)0xFFFFFFF0)
    // gta_computed_value_integer_create(integer->value, context):
    // context is in r15.
    //   mov rsi, r15
    //   mov rdi, integer->value
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSI, GTA_REG_R15)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDI, integer->value)
    //   mov rax, gta_computed_value_integer_create
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_integer_create))
    //   call rax
    && gta_call_reg__x86_64(v, GTA_REG_RAX)
    // Tear down the function call.
    //   mov rsp, rbp
    //   pop rbp
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP);
}
