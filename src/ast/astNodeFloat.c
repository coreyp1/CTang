
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeFloat.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/program/binary.h>
#include <tang/program/binaryCompilerContext.h>
#include <tang/program/bytecode.h>
#include <tang/program/bytecodeCompilerContext.h>

GTA_Ast_Node_VTable gta_ast_node_float_vtable = {
  .name = "Float",
  .compile_to_bytecode = gta_ast_node_float_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_float_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
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
  *self = (GTA_Ast_Node_Float) {
    .base = {
      .vtable = &gta_ast_node_float_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_FLOAT,
      .is_singleton = false,
    },
    .value = value,
  };
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


bool gta_ast_node_float_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  GTA_Ast_Node_Float * float_node = (GTA_Ast_Node_Float *) self;
  GCU_Vector8 * v = context->binary_vector;

  return true
  // Set up for a function call.
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFFFFFFFFFF0
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, (int32_t)0xFFFFFFF0)
  // context is in r15.
  // gta_computed_value_float_create(float_node->value, context):
  //   mov rdi, r15
  //   mov rax, float_node->value
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RDI, GTA_REG_R15)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, GTA_TYPEX_MAKE_F(float_node->value).i64)

  //   movq xmm0, rax
    && gta_movq_reg_reg__x86_64(v, GTA_REG_XMM0, GTA_REG_RAX)

  //   mov rax, gta_computed_value_float_create
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (GTA_UInteger)gta_computed_value_float_create)
  // GTA_UInteger fp = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_float_create);

  //   call rax
    && gta_call_reg__x86_64(v, GTA_REG_RAX)
  // Tear down the function call.
  //   mov rsp, rbp
  //   pop rbp
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP);
}
