
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeSlice.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_slice_vtable = {
  .name = "Slice",
  .compile_to_bytecode = gta_ast_node_slice_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_slice_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_slice_destroy,
  .print = gta_ast_node_slice_print,
  .simplify = gta_ast_node_slice_simplify,
  .analyze = gta_ast_node_slice_analyze,
  .walk = gta_ast_node_slice_walk,
};


GTA_Ast_Node_Slice * gta_ast_node_slice_create(GTA_Ast_Node * lhs, GTA_Ast_Node * start, GTA_Ast_Node * end, GTA_Ast_Node * skip, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Slice * self = gcu_malloc(sizeof(GTA_Ast_Node_Slice));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Slice) {
    .base = {
      .vtable = &gta_ast_node_slice_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .lhs = lhs,
    .start = start,
    .end = end,
    .skip = skip,
  };
  return self;
}


void gta_ast_node_slice_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  GTA_Ast_Node * parts[] = {slice->lhs, slice->start, slice->end, slice->skip};
  for (size_t i = 0; i < 4; ++i) {
    if (parts[i]) {
      gta_ast_node_destroy(parts[i]);
    }
  }
  gcu_free(self);
}


void gta_ast_node_slice_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 5);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  printf("%s%s:\n", indent, self->vtable->name);
  printf("%s  LHS:\n", indent);
  gta_ast_node_print(slice->lhs, new_indent);
  printf("%s  Start:\n", indent);
  gta_ast_node_print(slice->start, new_indent);
  printf("%s  End:\n", indent);
  gta_ast_node_print(slice->end, new_indent);
  printf("%s  Skip:\n", indent);
  if (slice->skip) {
    gta_ast_node_print(slice->skip, new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_slice_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  GTA_Ast_Node * simplified_lhs = gta_ast_node_simplify(slice->lhs, variable_map);
  if (simplified_lhs) {
    gta_ast_node_destroy(slice->lhs);
    slice->lhs = simplified_lhs;
  }
  GTA_Ast_Node * simplified_start = gta_ast_node_simplify(slice->start, variable_map);
  if (simplified_start) {
    gta_ast_node_destroy(slice->start);
    slice->start = simplified_start;
  }
  GTA_Ast_Node * simplified_end = gta_ast_node_simplify(slice->end, variable_map);
  if (simplified_end) {
    gta_ast_node_destroy(slice->end);
    slice->end = simplified_end;
  }
  GTA_Ast_Node * simplified_skip = slice->skip
    ? gta_ast_node_simplify(slice->skip, variable_map)
    : NULL;
  if (simplified_skip) {
    gta_ast_node_destroy(slice->skip);
    slice->skip = simplified_skip;
  }
  return 0;
}


void gta_ast_node_slice_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  GTA_Ast_Node * parts[] = {slice->lhs, slice->start, slice->end, slice->skip};
  for (size_t i = 0; i < 4; ++i) {
    if (parts[i]) {
      gta_ast_node_walk(parts[i], callback, data, return_value);
    }
  }
}


GTA_Ast_Node * gta_ast_node_slice_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  GTA_Ast_Node * parts[] = {slice->lhs, slice->start, slice->end, slice->skip};
  for (size_t i = 0; i < 4; ++i) {
    GTA_Ast_Node * error = parts[i]
      ? gta_ast_node_analyze(parts[i], program, scope)
      : NULL;
    if (error) {
      return error;
    }
  }
  return NULL;
}


bool gta_ast_node_slice_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  return true
    && gta_ast_node_compile_to_bytecode(slice->lhs, context)
    && gta_ast_node_compile_to_bytecode(slice->start, context)
    && gta_ast_node_compile_to_bytecode(slice->end, context)
    && (slice->skip
      ? gta_ast_node_compile_to_bytecode(slice->skip, context)
      : gta_ast_node_null_compile_to_bytecode(0, context))
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_SLICE));
}


bool gta_ast_node_slice_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  GCU_Vector8 * v = context->binary_vector;

  // Compile the entire slice expression.
  return true
  // Compile the expression and push the result.
  //   push rax
    && gta_ast_node_compile_to_binary__x86_64(slice->lhs, context)
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
  // Compile the start index and push the result.
  //   push rax
    && gta_ast_node_compile_to_binary__x86_64(slice->start, context)
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
  // Compile the end index and push the result.
  //   push rax
    && gta_ast_node_compile_to_binary__x86_64(slice->end, context)
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
  // Compile the skip index.
    && (slice->skip
      ? gta_ast_node_compile_to_binary__x86_64(slice->skip, context)
      : gta_ast_node_null_compile_to_binary__x86_64(0, context))
  // Call the slice function.
  // gta_computed_value_slice(lhs, start, end, skip, context)
  //   mov r8, r15
  //   mov rcx, rax
  //   pop rdx
  //   pop rsi
  //   pop rdi
  //   mov rax, gta_computed_value_slice
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFF0
  //   call rax
  //   mov rsp, rbp
  //   pop rbp
    && gta_mov_reg_reg__x86_64(v, GTA_REG_R8, GTA_REG_R15)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RCX, GTA_REG_RAX)
    && gta_pop_reg__x86_64(v, GTA_REG_RDX)
    && gta_pop_reg__x86_64(v, GTA_REG_RSI)
    && gta_pop_reg__x86_64(v, GTA_REG_RDI)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (size_t)gta_computed_value_slice)
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, 0xFFFFFFF0)
    && gta_call_reg__x86_64(v, GTA_REG_RAX)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP);
}
