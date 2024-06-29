
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeArray.h>
#include <tang/computedValue/computedValueArray.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_array_vtable = {
  .name = "Array",
  .compile_to_bytecode = gta_ast_node_array_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_array_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_array_destroy,
  .print = gta_ast_node_array_print,
  .simplify = gta_ast_node_array_simplify,
  .analyze = gta_ast_node_array_analyze,
  .walk = gta_ast_node_array_walk,
};


GTA_Ast_Node_Array * gta_ast_node_array_create(GTA_VectorX * elements, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Array * self = gcu_malloc(sizeof(GTA_Ast_Node_Array));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Array) {
    .base = {
      .vtable = &gta_ast_node_array_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .elements = elements,
  };
  return self;
}


void gta_ast_node_array_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  GTA_VECTORX_DESTROY(array->elements);
  gcu_free(self);
}


void gta_ast_node_array_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 3);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);
  printf("%s%s\n", indent, self->vtable->name);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(array->elements); i++) {
    gta_ast_node_print((GTA_Ast_Node *) array->elements->data[i].p, new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_array_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(array->elements); ++i) {
    GTA_Ast_Node * element = (GTA_Ast_Node *) array->elements->data[i].p;
    GTA_Ast_Node * simplified = gta_ast_node_simplify(element, variable_map);
    if (simplified) {
      gta_ast_node_destroy(element);
      array->elements->data[i].p = simplified;
    }
  }
  return 0;
}


void gta_ast_node_array_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(array->elements); ++i) {
    GTA_Ast_Node * element = (GTA_Ast_Node *) array->elements->data[i].p;
    gta_ast_node_walk(element, callback, data, return_value);
  }
}


GTA_Ast_Node * gta_ast_node_array_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(array->elements); ++i) {
    GTA_Ast_Node * result = gta_ast_node_analyze((GTA_Ast_Node *)GTA_TYPEX_P(array->elements->data[i]), program, scope);
    if (result) {
      return result;
    }
  }
  return 0;
}


bool gta_ast_node_array_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  for (size_t i = 0; i < array->elements->count; ++i) {
    if (!gta_ast_node_compile_to_bytecode((GTA_Ast_Node *)GTA_TYPEX_P(array->elements->data[i]), context)) {
      return false;
    }
  }
  return true
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_ARRAY))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(array->elements->count));
}


bool gta_ast_node_array_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  GCU_Vector8 * v = context->binary_vector;
  bool * is_temporary_offset = &((GTA_Computed_Value *)0)->is_temporary;
  bool * is_singleton_offset = &((GTA_Computed_Value *)0)->is_singleton;
  GTA_VectorX * * elements_offset = &((GTA_Computed_Value_Array *)0)->elements;
  GTA_TypeX_Union * * data_offset = &((GTA_VectorX *)0)->data;
  size_t * count_offset = &((GTA_VectorX *)0)->count;

  // Jump labels.
  GTA_Integer end;
  GTA_Integer return_memory_error;

  bool error_free = true
  // Create jump labels.
    && ((end = gta_binary_compiler_context_get_label(context)) >= 0)
    && ((return_memory_error = gta_binary_compiler_context_get_label(context)) >= 0)
  // gta_computed_value_array_create(array->elements->count, context)
  //   mov rdi, array->elements->count
  //   mov rsi, r15
  //   mov rdx, gta_computed_value_array_create
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFFFFFFFFFF0
  //   call rdx
  //   mov rsp, rbp
  //   pop rbp
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDI, array->elements->count)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSI, GTA_REG_R15)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDX, (size_t)gta_computed_value_array_create)
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, 0xFFFFFFF0)
    && gta_call_reg__x86_64(v, GTA_REG_RDX)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP)
  // If the array creation failed, return a memory error.
  //   test rax, rax
  //   jz return_memory_error
    && gta_test_reg_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RAX)
    && gta_jcc__x86_64(v, GTA_CC_Z, 0xDEADBEEF)
    && gta_binary_compiler_context_add_label_jump(context, return_memory_error, v->count - 4)
  // No memory error.
  // If we are about to process a bunch of elements, then save the array
  // pointer.  Otherwise, jump to the end.
  //   push rax  OR  jmp end
    && array->elements->count
      ? gta_push_reg__x86_64(v, GTA_REG_RAX)
      : (gta_jmp__x86_64(v, 0xDEADBEEF)
        && gta_binary_compiler_context_add_label_jump(context, end, v->count - 4));

  // Compile the individual array elements.
  for (size_t i = 0; i < array->elements->count; ++i) {
    GTA_Ast_Node * element = (GTA_Ast_Node *)GTA_TYPEX_P(array->elements->data[i]);
    GTA_Integer mark_not_temporary;
    error_free = error_free
    // Create jump label.
      && ((mark_not_temporary = gta_binary_compiler_context_get_label(context)) >= 0)
    // Compile the expression.
      && gta_ast_node_compile_to_binary__x86_64(element, context)
    // If the element is temporary or is singleton, then set it as not
    // temporary and append.
    //   cmp byte ptr [rax + is_temporary_offset], 0
    //   je mark_not_temporary
      && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)is_temporary_offset, 0)
      && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
      && gta_binary_compiler_context_add_label_jump(context, mark_not_temporary, v->count - 4)
    //   cmp byte ptr [rax + is_singleton_offset], 0
    //   je mark_not_temporary
      && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)is_singleton_offset, 0)
      && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
      && gta_binary_compiler_context_add_label_jump(context, mark_not_temporary, v->count - 4)
    // gta_computed_value_deep_copy(element, context)
    //   mov rdi, rax
    //   mov rsi, r15
    //   mov rdx, gta_computed_value_deep_copy
    //   push rbp
    //   mov rbp, rsp
    //   and rsp, 0xFFFFFFFFFFFFFFF0
    //   call rdx
    //   mov rsp, rbp
    //   pop rbp
      && gta_mov_reg_reg__x86_64(v, GTA_REG_RDI, GTA_REG_RAX)
      && gta_mov_reg_reg__x86_64(v, GTA_REG_RSI, GTA_REG_R15)
      && gta_mov_reg_imm__x86_64(v, GTA_REG_RDX, (size_t)gta_computed_value_deep_copy)
      && gta_push_reg__x86_64(v, GTA_REG_RBP)
      && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
      && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, 0xFFFFFFF0)
      && gta_call_reg__x86_64(v, GTA_REG_RDX)
      && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
      && gta_pop_reg__x86_64(v, GTA_REG_RBP)
    // If the array creation failed, return a memory error.
    //   test rax, rax
    //   jz return_memory_error
      && gta_test_reg_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RAX)
      && gta_jcc__x86_64(v, GTA_CC_Z, 0xDEADBEEF)
      && gta_binary_compiler_context_add_label_jump(context, return_memory_error, v->count - 4)
    // mark_not_temporary:
      && gta_binary_compiler_context_set_label(context, mark_not_temporary, v->count)
    //   mov byte ptr [rax + is_temporary_offset], 0
      && gta_mov_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)is_temporary_offset, 0)
    // Append the element to the array.
    //   mov rdi, [rsp]                   ; rdi = array
    //   mov rdi, [rdi + elements_offset] ; rdi = array->elements
    //   mov rbx, [rdi + data_offset]     ; rbx = array->elements->data
    //   mov [rbx + (i * sizeof(GTA_TypeX_Union))], rax
      && gta_mov_reg_ind__x86_64(v, GTA_REG_RDI, GTA_REG_RSP, GTA_REG_NONE, 0, 0)
      && gta_mov_reg_ind__x86_64(v, GTA_REG_RDI, GTA_REG_RDI, GTA_REG_NONE, 0, (GTA_Integer)elements_offset)
      && gta_mov_reg_ind__x86_64(v, GTA_REG_RBX, GTA_REG_RDI, GTA_REG_NONE, 0, (GTA_Integer)data_offset)
      && gta_mov_ind_reg__x86_64(v, GTA_REG_RBX, GTA_REG_NONE, 0, i * sizeof(GTA_TypeX_Union), GTA_REG_RAX);
  }

  // The code below (above the jump targets) is always compiled, but it will
  // only be executed if the array has elements.  If the array has no elements,
  // then the code will jump to the end and the default values will already be
  // appropriately set.
  return error_free
  // Fix the array count.
  // The memory address of the VectorX is in RDI.
  //   mov rdx, array->elements->count
  //   mov [rdi + count_offset], rdx
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDX, array->elements->count)
    && gta_mov_ind_reg__x86_64(v, GTA_REG_RDI, GTA_REG_NONE, 0, (GTA_Integer)count_offset, GTA_REG_RDX)
  // Return the array.
  //   pop rax
  //   jmp end
    && gta_pop_reg__x86_64(v, GTA_REG_RAX)
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_binary_compiler_context_add_label_jump(context, end, v->count - 4)
  // return_memory_error:
    && gta_binary_compiler_context_set_label(context, return_memory_error, v->count)
  // return gta_computed_value_error_out_of_memory
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (size_t)gta_computed_value_error_out_of_memory)
  // end:
    && gta_binary_compiler_context_set_label(context, end, v->count);
}
