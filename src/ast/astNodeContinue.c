
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeContinue.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_continue_vtable = {
  .name = "Continue",
  .compile_to_bytecode = gta_ast_node_continue_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_continue_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_continue_destroy,
  .print = gta_ast_node_continue_print,
  .simplify = gta_ast_node_continue_simplify,
  .analyze = 0,
  .walk = gta_ast_node_continue_walk,
};


GTA_Ast_Node_Continue * gta_ast_node_continue_create(GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Continue * self = gcu_malloc(sizeof(GTA_Ast_Node_Continue));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Continue) {
    .base = {
      .vtable = &gta_ast_node_continue_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
  };
  return self;
}


void gta_ast_node_continue_destroy(GTA_Ast_Node * self) {
  assert(self);
  gcu_free(self);
}


void gta_ast_node_continue_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_CONTINUE(self));
  assert(indent);
  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s\n", indent, self->vtable->name);
}


GTA_Ast_Node * gta_ast_node_continue_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_continue_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  callback(self, data, return_value);
}


bool gta_ast_node_continue_compile_to_bytecode(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_Compiler_Context * context) {
  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  return true
  // JMP context->continue_label
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(context, context->continue_label, context->program->bytecode->count - 1);
}


bool gta_ast_node_continue_compile_to_binary__x86_64(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_Compiler_Context * context) {
  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  return true
  // JMP context->continue_label
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, context->continue_label, v->count - 4);
}
