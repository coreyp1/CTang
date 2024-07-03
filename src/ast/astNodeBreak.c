
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeBreak.h>
#include <tang/computedValue/computedValue.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_break_vtable = {
  .name = "Break",
  .compile_to_bytecode = gta_ast_node_break_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_break_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
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


bool gta_ast_node_break_compile_to_bytecode(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_Compiler_Context * context) {
  return true
  // NULL ; Something must be on the stack.
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_NULL))
  // JMP context->break_label
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(context, context->break_label, context->program->bytecode->count - 1);
}


bool gta_ast_node_break_compile_to_binary__x86_64(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_Compiler_Context * context) {
  GCU_Vector8 * v = context->binary_vector;
  return true
  // Something valid must be in RAX.  Use Null.
  //   mov rax, gta_computed_value_null
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (GTA_Integer)gta_computed_value_null)
  // JMP context->break_label
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, context->break_label, v->count - 4);
}
