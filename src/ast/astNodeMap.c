
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeMap.h>
#include <tang/ast/astNodeString.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueMap.h>
#include <tang/program/binary.h>
#include <tang/unicodeString.h>

GTA_Ast_Node_VTable gta_ast_node_map_vtable = {
  .name = "Map",
  .compile_to_bytecode = gta_ast_node_map_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_map_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_map_destroy,
  .print = gta_ast_node_map_print,
  .simplify = gta_ast_node_map_simplify,
  .analyze = gta_ast_node_map_analyze,
  .walk = gta_ast_node_map_walk,
};


GTA_Ast_Node_Map * gta_ast_node_map_create(GTA_VectorX * pairs, GTA_PARSER_LTYPE location) {
  assert(pairs);

  GTA_Ast_Node_Map * self = gcu_malloc(sizeof(GTA_Ast_Node_Map));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Map) {
    .base = {
      .vtable = &gta_ast_node_map_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .pairs = pairs,
  };
  return self;
}


void gta_ast_node_map_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_MAP(self));
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;

  GTA_VECTORX_DESTROY(map->pairs);
  gcu_free(self);
}


void gta_ast_node_map_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_MAP(self));
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;

  assert(indent);
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 5);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);

  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s\n", indent, self->vtable->name);

  assert(map->pairs);
  assert(map->pairs->count ? (bool)map->pairs->data : true);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    printf("%s  Key:\n", indent);
    gta_ast_node_print(pair->key, new_indent);
    printf("%s  Value:\n", indent);
    gta_ast_node_print(pair->value, new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_map_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_MAP(self));
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;

  assert(map->pairs);
  assert(map->pairs->count ? (bool)map->pairs->data : true);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    GTA_Ast_Node * simplified = gta_ast_node_simplify(pair->value, variable_map);
    if (simplified) {
      gta_ast_node_destroy(pair->value);
      pair->value = simplified;
    }
  }
  return 0;
}


void gta_ast_node_map_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_MAP(self));
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;

  callback(self, data, return_value);

  assert(map->pairs);
  assert(map->pairs->count ? (bool)map->pairs->data : true);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    callback(pair->key, data, return_value);
    callback(pair->value, data, return_value);
  }
}


GTA_Ast_Node * gta_ast_node_map_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_MAP(self));
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;

  assert(map->pairs);
  assert(map->pairs->count ? (bool)map->pairs->data : true);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    GTA_Ast_Node * error = gta_ast_node_analyze(pair->key, program, scope);
    if (error) {
      return error;
    }
    error = gta_ast_node_analyze(pair->value, program, scope);
    if (error) {
      return error;
    }
  }
  return NULL;
}


bool gta_ast_node_map_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_MAP(self));
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;

  bool error_free = true;

  assert(map->pairs);
  assert(map->pairs->count ? (bool)map->pairs->data : true);
  for (size_t i = 0; error_free && (i < map->pairs->count); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    error_free &= true
      && gta_ast_node_compile_to_bytecode(pair->key, context)
      && gta_ast_node_compile_to_bytecode(pair->value, context);
  }

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  return error_free
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_MAP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(map->pairs->count));
}


bool gta_ast_node_map_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_MAP(self));
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  bool * is_temporary_offset = &((GTA_Computed_Value *)0)->is_temporary;
  bool * is_singleton_offset = &((GTA_Computed_Value *)0)->is_singleton;

  // Jump labels.
  GTA_Integer end;
  GTA_Integer return_memory_error;
  GTA_Integer pop_once_then_return_memory_error;
  GTA_Integer pop_twice_then_return_memory_error;

  bool error_free = true
  // Create jump labels.
    && ((end = gta_compiler_context_get_label(context)) >= 0)
    && ((return_memory_error = gta_compiler_context_get_label(context)) >= 0)
    && ((pop_once_then_return_memory_error = gta_compiler_context_get_label(context)) >= 0)
    && ((pop_twice_then_return_memory_error = gta_compiler_context_get_label(context)) >= 0)
  // gta_computed_value_map_create(map->pairs->count, context)
  //   mov rdi, array->elements->count
  //   mov rsi, r15
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDI, map->pairs->count)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSI, GTA_REG_R15)
    && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_map_create)
  // If the array creation failed, return a memory error.
  //   test rax, rax
  //   jz return_memory_error
    && gta_test_reg_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RAX)
    && gta_jcc__x86_64(v, GTA_CC_Z, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, return_memory_error, v->count - 4)
  // No memory error.
  // If we are about to process a bunch of elements, then save the map
  // pointer.  Otherwise, jump to the end.
  //   push rax  OR  jmp end
    && map->pairs->count
      ? gta_push_reg__x86_64(v, GTA_REG_RAX)
      : (gta_jmp__x86_64(v, 0xDEADBEEF)
        && gta_compiler_context_add_label_jump(context, end, v->count - 4));

  // Compile the individual key/value pairs.
  assert(map->pairs);
  assert(map->pairs->count ? (bool)map->pairs->data : true);
  for (size_t i = 0; i < map->pairs->count; ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);

    GTA_Integer mark_not_temporary;
    error_free = error_free
    // Create jump label.
      && ((mark_not_temporary = gta_compiler_context_get_label(context)) >= 0)
    // Compile the key.
      && gta_ast_node_compile_to_binary__x86_64(pair->key, context)
    // Set the key as not temporary.
    //   mov byte ptr [rax + is_temporary_offset], 0
      && gta_mov_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)is_temporary_offset, 0)
    // Save the key to the stack.
    //   push rax
      && gta_push_reg__x86_64(v, GTA_REG_RAX)

    // Compile the value.
      && gta_ast_node_compile_to_binary__x86_64(pair->value, context)
    // If the element is temporary or is singleton, then set it as not
    // temporary and append.
    //   cmp byte ptr [rax + is_temporary_offset], 0
    //   je mark_not_temporary
      && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)is_temporary_offset, 0)
      && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
      && gta_compiler_context_add_label_jump(context, mark_not_temporary, v->count - 4)
    //   cmp byte ptr [rax + is_singleton_offset], 0
    //   je mark_not_temporary
      && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)is_singleton_offset, 0)
      && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
      && gta_compiler_context_add_label_jump(context, mark_not_temporary, v->count - 4)
    // gta_computed_value_deep_copy(element, context)
    //   mov rdi, rax
    //   mov rsi, r15
      && gta_mov_reg_reg__x86_64(v, GTA_REG_RDI, GTA_REG_RAX)
      && gta_mov_reg_reg__x86_64(v, GTA_REG_RSI, GTA_REG_R15)
      && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_deep_copy)
    // If the deep copy failed, return a memory error.
    //   test rax, rax
    //   jz pop_twice_then_return_memory_error
      && gta_test_reg_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RAX)
      && gta_jcc__x86_64(v, GTA_CC_Z, 0xDEADBEEF)
      && gta_compiler_context_add_label_jump(context, pop_twice_then_return_memory_error, v->count - 4)
    // mark_not_temporary:
      && gta_compiler_context_set_label(context, mark_not_temporary, v->count)
    //   mov byte ptr [rax + is_temporary_offset], 0
      && gta_mov_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)is_temporary_offset, 0)

    // Add the key/value pair to the map.
    // gta_computed_value_map_set_key_val(map, key, value)
    //   mov rdx, rax                     ; rdx = value
    //   pop rsi                          ; rsi = key
    //   mov rdi, [rsp]                   ; rdi = map
      && gta_mov_reg_reg__x86_64(v, GTA_REG_RDX, GTA_REG_RAX)
      && gta_pop_reg__x86_64(v, GTA_REG_RSI)
      && gta_mov_reg_ind__x86_64(v, GTA_REG_RDI, GTA_REG_RSP, GTA_REG_NONE, 0, 0)
      && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_map_set_key_val);
  }


  // The code below (above the jump targets) is always compiled, but it will
  // only be executed if the map has key/value pairs.  If the map has no pairs,
  // then the code will jump to the end and the default values will already be
  // appropriately set.
  return error_free
  // Return the array.
  //   pop rax
  //   jmp end
    && gta_pop_reg__x86_64(v, GTA_REG_RAX)
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, end, v->count - 4)
  // pop_twice_then_return_memory_error:  ; (falls through)
    && gta_compiler_context_set_label(context, pop_twice_then_return_memory_error, v->count)
    && gta_pop_reg__x86_64(v, GTA_REG_RAX)
  // pop_once_then_return_memory_error:   ; (falls through)
    && gta_compiler_context_set_label(context, pop_once_then_return_memory_error, v->count)
    && gta_pop_reg__x86_64(v, GTA_REG_RAX)
  // return_memory_error:
    && gta_compiler_context_set_label(context, return_memory_error, v->count)
  // return gta_computed_value_error_out_of_memory
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (size_t)gta_computed_value_error_out_of_memory)
  // end:
    && gta_compiler_context_set_label(context, end, v->count);
}
