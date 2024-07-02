
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeIndex.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_index_vtable = {
  .name = "Index",
  .compile_to_bytecode = gta_ast_node_index_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_index_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_index_destroy,
  .print = gta_ast_node_index_print,
  .simplify = gta_ast_node_index_simplify,
  .analyze = gta_ast_node_index_analyze,
  .walk = gta_ast_node_index_walk,
};


GTA_Ast_Node_Index * gta_ast_node_index_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Index * self = gcu_malloc(sizeof(GTA_Ast_Node_Index));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Index) {
    .base = {
      .vtable = &gta_ast_node_index_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .lhs = lhs,
    .rhs = rhs,
  };
  return self;
}


void gta_ast_node_index_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  gta_ast_node_destroy(index->lhs);
  gta_ast_node_destroy(index->rhs);
  gcu_free(self);
}


void gta_ast_node_index_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  printf("%s%s\n", indent, self->vtable->name);
  printf("%s  LHS:\n", indent);
  gta_ast_node_print(index->lhs, new_indent);
  printf("%s  RHS:\n", indent);
  gta_ast_node_print(index->rhs, new_indent);
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_index_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  GTA_Ast_Node * simplified_lhs = gta_ast_node_simplify(index->lhs, variable_map);
  if (simplified_lhs) {
    gta_ast_node_destroy(index->lhs);
    index->lhs = simplified_lhs;
  }
  GTA_Ast_Node * simplified_rhs = gta_ast_node_simplify(index->rhs, variable_map);
  if (simplified_rhs) {
    gta_ast_node_destroy(index->rhs);
    index->rhs = simplified_rhs;
  }
  return 0;
}


void gta_ast_node_index_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  gta_ast_node_walk(index->lhs, callback, data, return_value);
  gta_ast_node_walk(index->rhs, callback, data, return_value);
}


GTA_Ast_Node * gta_ast_node_index_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  GTA_Ast_Node * error = gta_ast_node_analyze(index->lhs, program, scope);
  return error
    ? error
    : gta_ast_node_analyze(index->rhs, program, scope);
}


bool gta_ast_node_index_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  return gta_ast_node_compile_to_bytecode(index->lhs, context)
    && gta_ast_node_compile_to_bytecode(index->rhs, context)
      && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_INDEX));
}


bool gta_ast_node_index_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  GCU_Vector8 * v = context->binary_vector;

  // Compile the expression.
  return true
  // Compile the collection expression.
    && gta_ast_node_compile_to_binary__x86_64(index->lhs, context)
  // Save it for future use.
  //   push rax
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
  // Compile the index expression.
    && gta_ast_node_compile_to_binary__x86_64(index->rhs, context)
  // gta_computed_value_index(collection, index, context)
  //   pop rdi
  //   mov rsi, rax
  //   mov rdx, r15
  //   mov rcx, gta_computed_value_index
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFF0
  //   call rcx
  //   mov rsp, rbp
  //   pop rbp
    && gta_pop_reg__x86_64(v, GTA_REG_RDI)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSI, GTA_REG_RAX)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RDX, GTA_REG_R15)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RCX, (size_t)gta_computed_value_index)
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, 0xFFFFFFF0)
    && gta_call_reg__x86_64(v, GTA_REG_RCX)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP);
  return false;
}
