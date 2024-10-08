
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <cutil/hash.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeBinary.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeRangedFor.h>
#include <tang/computedValue/computedValueIterator.h>
#include <tang/program/binary.h>
#include <tang/program/variable.h>

GTA_Ast_Node_VTable gta_ast_node_ranged_for_vtable = {
  .name = "RangedFor",
  .compile_to_bytecode = gta_ast_node_ranged_for_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_ranged_for_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_ranged_for_destroy,
  .print = gta_ast_node_ranged_for_print,
  .simplify = gta_ast_node_ranged_for_simplify,
  .analyze = gta_ast_node_ranged_for_analyze,
  .walk = gta_ast_node_ranged_for_walk,
};


GTA_Ast_Node_Ranged_For * gta_ast_node_ranged_for_create(const char * identifier, GTA_Ast_Node * expression, GTA_Ast_Node * block, GTA_PARSER_LTYPE location) {
  assert(identifier);
  assert(expression);
  assert(block);

  // Perform all of the necessary allocations or fail.
  // Allocate space for the ranged-for node.
  GTA_Ast_Node_Ranged_For * self = gcu_malloc(sizeof(GTA_Ast_Node_Ranged_For));
  if (!self) {
    goto SELF_CREATE_FAILED;
  }
  // Create an identifier node for the ranged-for loop variable.
  GTA_Ast_Node * identifier_node = (GTA_Ast_Node *)gta_ast_node_identifier_create(identifier, location);
  if (!identifier_node) {
    goto IDENTIFIER_CREATE_FAILED;
  }
  // Create a unique name for the iterator variable.
  char * iterator_name = gcu_malloc(32);
  if (!iterator_name) {
    goto ITERATOR_NAME_CREATE_FAILED;
  }
  snprintf(iterator_name, 31, "iterator::%p", (void *)self);
  // Create an identifier node for the iterator variable.
  GTA_Ast_Node * iterator_node = (GTA_Ast_Node *)gta_ast_node_identifier_create(iterator_name, location);
  if (!iterator_node) {
    goto ITERATOR_IDENTIFIER_CREATE_FAILED;
  }

  // All allocations are successful, so initialize the ranged-for node.
  *self = (GTA_Ast_Node_Ranged_For) {
    .base = {
      .vtable = &gta_ast_node_ranged_for_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .identifier = identifier_node,
    .expression = expression,
    .iterator = iterator_node,
    .block = block,
  };
  return self;

  // Failure cleanup.
ITERATOR_IDENTIFIER_CREATE_FAILED:
  gcu_free(iterator_name);
ITERATOR_NAME_CREATE_FAILED:
  gta_ast_node_destroy(identifier_node);
IDENTIFIER_CREATE_FAILED:
  gcu_free(self);
SELF_CREATE_FAILED:
  return 0;
}


void gta_ast_node_ranged_for_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_RANGED_FOR(self));
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *) self;

  gta_ast_node_destroy(ranged_for->identifier);
  gta_ast_node_destroy(ranged_for->expression);
  gta_ast_node_destroy(ranged_for->iterator);
  gta_ast_node_destroy(ranged_for->block);
  gcu_free(self);
}


void gta_ast_node_ranged_for_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_RANGED_FOR(self));
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *) self;

  assert(indent);
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 5);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);

  gta_ast_node_print(ranged_for->identifier, indent);

  printf("%s  Expression:\n", indent);
  gta_ast_node_print(ranged_for->expression, new_indent);

  printf("%s  Block:\n", indent);
  gta_ast_node_print(ranged_for->block, new_indent);
  gcu_free(new_indent);
}


/**
 * This function is used to find all the assignments in the while loop.
 */
static void findAssignments(GTA_Ast_Node * self, void * data, void * error) {
  assert(self);
  assert(data);
  assert(error);
  GCU_Hash64 * assignments = (GCU_Hash64 *)data;
  if (GTA_AST_IS_ASSIGN(self)) {
    GTA_Ast_Node_Assign * assignment = (GTA_Ast_Node_Assign *)self;
    if (GTA_AST_IS_IDENTIFIER(assignment->lhs)) {
      GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *)assignment->lhs;
      if (!gcu_hash64_set(assignments, gcu_string_hash_64(identifier->identifier, strlen(identifier->identifier)), GCU_TYPE64_B(true))) {
        *(bool *)error = true;
      }
    }
  }
}


GTA_Ast_Node * gta_ast_node_ranged_for_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_RANGED_FOR(self));
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *) self;

  // The expression is always executed once, so we can simplify it.
  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(ranged_for->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(ranged_for->expression);
    ranged_for->expression = simplified_expression;
  }

  // The block may be executed multiple times, so we cannot allow any nodes to
  // be simplified that are also assignments at any point in the code block.
  // Use a walk function to find all the assignments in the loop.
  // If any part of this fails, we must invalidate all the assignments in the
  // variable map, because we cannot guarantee that assignments did not happen.
  GCU_Hash64 * assignments = gcu_hash64_create(32);
  bool error = false;
  if (!assignments) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    return 0;
  }

  // The ranged-for identifier is always assigned, and so should be removed from
  // the variable map.  We do not need to check for failure here, because the
  // variable map was successfully created with 32 slots.
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *)ranged_for->identifier;
  gcu_hash64_set(assignments, identifier->hash, GCU_TYPE64_B(true));

  // Find all the assignments in the block.
  gta_ast_node_walk(ranged_for->block, findAssignments, assignments, &error);
  if (error) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    gcu_hash64_destroy(assignments);
    return 0;
  }

  // Remove the assignments from the variable map.
  GCU_Hash64_Iterator iterator = gcu_hash64_iterator_get(assignments);
  while (iterator.exists) {
    gcu_hash64_remove(variable_map, iterator.hash);
    iterator = gcu_hash64_iterator_next(iterator);
  }

  // Create copies of the variable map to pass to the block.
  GTA_Ast_Simplify_Variable_Map * block_variable_map = gcu_hash64_clone(variable_map);
  if (!block_variable_map) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    gcu_hash64_destroy(assignments);
    gcu_hash64_destroy(block_variable_map);
    return 0;
  }

  GTA_Ast_Node * simplified_block = gta_ast_node_simplify(ranged_for->block, block_variable_map);
  if (simplified_block) {
    gta_ast_node_destroy(ranged_for->block);
    ranged_for->block = simplified_block;
  }

  // Synchronize the variable map with the block variable map.
  gta_ast_simplify_variable_map_synchronize(variable_map, block_variable_map);

  // Destroy the temporary variable maps.
  gcu_hash64_destroy(assignments);
  gcu_hash64_destroy(block_variable_map);

  return 0;
}


void gta_ast_node_ranged_for_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_RANGED_FOR(self));
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *) self;

  callback(self, data, return_value);

  gta_ast_node_walk(ranged_for->identifier, callback, data, return_value);
  gta_ast_node_walk(ranged_for->expression, callback, data, return_value);
  gta_ast_node_walk(ranged_for->iterator, callback, data, return_value);
  gta_ast_node_walk(ranged_for->block, callback, data, return_value);
}


GTA_Ast_Node * gta_ast_node_ranged_for_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_RANGED_FOR(self));
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *) self;

  GTA_Ast_Node * items[] = {ranged_for->identifier, ranged_for->expression, ranged_for->iterator, ranged_for->block};
  for (size_t i = 0; i < 4; ++i) {
    GTA_Ast_Node * error = gta_ast_node_analyze(items[i], program, scope);
    if (error) {
      return error;
    }
  }
  return NULL;
}


bool gta_ast_node_ranged_for_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_RANGED_FOR(self));
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *)self;

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);

  // Find where the iterator is stored.  It will always be local.
  GTA_Ast_Node_Identifier * iterator = (GTA_Ast_Node_Identifier *)ranged_for->iterator;
  GTA_HashX_Value iterator_stack_location = GTA_HASHX_GET(iterator->scope->variable_positions, iterator->mangled_name_hash);
  if (!iterator_stack_location.exists) {
    printf("Error: Identifier %s not found in local positions.\n", iterator->mangled_name);
    return false;
  }

  // Find where the ranged-for variable is stored.  It may not be local.
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *)ranged_for->identifier;
  GTA_HashX_Value identifier_stack_location = GTA_HASHX_GET(identifier->scope->variable_positions, identifier->mangled_name_hash);
  bool identifier_is_local = true;
  if (!identifier_stack_location.exists) {
    identifier_is_local = false;
    identifier_stack_location = GTA_HASHX_GET(identifier->scope->variable_positions, identifier->mangled_name_hash);
    if (!identifier_stack_location.exists) {
      printf("Error: Identifier %s not found in local or global positions.\n", identifier->mangled_name);
      return false;
    }
  }

  // Jump labels.
  GTA_Integer get_next_iterator_value;
  GTA_Integer end_of_loop;
  GTA_Integer original_break_label = context->break_label;
  GTA_Integer original_continue_label = context->continue_label;

  // Compile the expression.
  return true
  // Create jump labels.
    && ((get_next_iterator_value = gta_compiler_context_get_label(context)) >= 0)
    && ((end_of_loop = gta_compiler_context_get_label(context)) >= 0)
    && ((context->continue_label = gta_compiler_context_get_label(context)) >= 0)
    && ((context->break_label = gta_compiler_context_get_label(context)) >= 0)

  // Compile the iterator expression.
    && gta_ast_node_compile_to_bytecode(ranged_for->expression, context)
  // ITERATOR ; pushes the iterator, pushes true if the iterator is valid,
  //            otherwise false.
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_ITERATOR))
  // JMPF end_of_loop ; jump to the end of the loop if the iterator is invalid.
  //                    'false' and iterator error will be on stack.
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMPF))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(context, end_of_loop, context->program->bytecode->count - 1)
  // POP ; pop the iterator T/F value.
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
  // Iterator is on top of the stack.
  // POKE_LOCAL (fp + iterator offset) ; store the iterator in the local variable.
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POKE_LOCAL))
    && GTA_VECTORX_APPEND(context->program->bytecode, iterator_stack_location.value)

  // The iterator is on the top of the stack.
  // get_next_iterator_value:
  //   ITERATOR_NEXT
  //   JMPF end_of_loop ; jump to the end of the loop if the iterator is
  //                     ; invalid.  'false' and iterator error left on stack.
    && gta_compiler_context_set_label(context, get_next_iterator_value, context->program->bytecode->count)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_ITERATOR_NEXT))
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMPF))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(context, end_of_loop, context->program->bytecode->count - 1)
  //   POP ; pop the iterator T/F value.  Iterator current value left on stack.
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))

  // Assign the current iterator value to the iterator identifier variable.
  //   ADOPT
  //   POKE_LOCAL/GLOBAL (fp + identifier offset)
  //   POP
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_ADOPT))
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(identifier_is_local ? GTA_BYTECODE_POKE_LOCAL : GTA_BYTECODE_POKE_GLOBAL))
    && GTA_VECTORX_APPEND(context->program->bytecode, identifier_stack_location.value)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))

  // Compile the body of the loop.
    && gta_ast_node_compile_to_bytecode(ranged_for->block, context)
  //    POP
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))

  // Load the iterator value.
  // continue_label:
  //   PEEK_LOCAL (fp + iterator offset)
  //   JMP get_next_iterator_value
    && gta_compiler_context_set_label(context, context->continue_label, context->program->bytecode->count)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_PEEK_LOCAL))
    && GTA_VECTORX_APPEND(context->program->bytecode, iterator_stack_location.value)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(context, get_next_iterator_value, context->program->bytecode->count - 1)

  // end_of_loop:
  //   POP                   ; pop the iterator T/F value.
    && gta_compiler_context_set_label(context, end_of_loop, context->program->bytecode->count)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
  // context->break_label:
    && gta_compiler_context_set_label(context, context->break_label, context->program->bytecode->count)
  // Restore the original break and continue labels.
    && ((context->break_label = original_break_label) >= 0)
    && ((context->continue_label = original_continue_label) >= 0);
}


bool gta_ast_node_ranged_for_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_RANGED_FOR(self));
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *)self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  // Offsets.
  void * vtable_offset = &((GTA_Computed_Value *)0)->vtable;

  // Find where the iterator is stored.  It will always be local.
  GTA_Ast_Node_Identifier * iterator = (GTA_Ast_Node_Identifier *)ranged_for->iterator;
  GTA_HashX_Value iterator_stack_location = GTA_HASHX_GET(iterator->scope->variable_positions, iterator->mangled_name_hash);
  if (!iterator_stack_location.exists) {
    printf("Error: Identifier %s not found in local positions.\n", iterator->mangled_name);
    return false;
  }
  int32_t iterator_stack_location_offset = ((int32_t)GTA_TYPEX_UI(iterator_stack_location.value) + 1) * -8;

  // Find where the ranged-for variable is stored.  It may not be local.
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *)ranged_for->identifier;
  GTA_HashX_Value identifier_stack_location = GTA_HASHX_GET(identifier->scope->variable_positions, identifier->mangled_name_hash);
  bool identifier_is_local = true;
  if (!identifier_stack_location.exists) {
    identifier_is_local = false;
    identifier_stack_location = GTA_HASHX_GET(identifier->scope->variable_positions, identifier->mangled_name_hash);
    if (!identifier_stack_location.exists) {
      printf("Error: Identifier %s not found in local or global positions.\n", identifier->mangled_name);
      return false;
    }
  }
  int32_t identifier_stack_location_offset = ((int32_t)GTA_TYPEX_UI(identifier_stack_location.value) + 1) * -8;

  // Jump labels.
  GTA_Integer top_of_loop;
  GTA_Integer get_next_iterator_value;
  GTA_Integer end_of_loop;
  GTA_Integer original_break_label = context->break_label;
  GTA_Integer original_continue_label = context->continue_label;

  // This is long and messy.  Example code and psudocode is below.
  // for (a:<expression>) {<block>}
  //   1. Evaluate the expression.
  //   2. Get the iterator.
  //   3. If the iterator fails, jump to the end of the loop.
  //   4. Save the iterator.
  //   5. Call the iterator next.
  //   6. If the iterator next fails, jump to the end of the loop.
  //   7. Adopt the iterator value.
  //   8. Assign the iterator value to the ranged-for variable.
  //   9. Execute the block.
  //   10. Load the iterator.
  //   11. Jump to 5.

  // Compile the expression.
  return true
  // Create jump labels.
    && ((top_of_loop = gta_compiler_context_get_label(context)) >= 0)
    && ((context->continue_label = get_next_iterator_value = gta_compiler_context_get_label(context)) >= 0)
    && ((context->break_label = end_of_loop = gta_compiler_context_get_label(context)) >= 0)

  // 1. Compile the iterator expression.  Result in RAX.
    && gta_ast_node_compile_to_binary__x86_64(ranged_for->expression, context)

  // 2. gta_computed_value_iterator_get(RAX, context). Result in RAX.
  //   mov GTA_X86_64_R1, rax
  //   mov GTA_X86_64_R2, context
    && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R1, GTA_REG_RAX)
    && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R2, GTA_REG_R15)
    && gta_binary_call__x86_64(v, (uint64_t)&gta_computed_value_iterator_get)

  // 3. If the iterator fails, jump to the end of the loop.
  //   mov GTA_X86_64_R1, [rax + vtable_offset]
  //   mov GTA_X86_64_R2, &gta_computed_value_iterator_vtable
  //   cmp GTA_X86_64_R1, GTA_X86_64_R2
  //   jne end_of_loop
    && gta_mov_reg_ind__x86_64(v, GTA_X86_64_R1, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)vtable_offset)
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_R2, (int64_t)&gta_computed_value_iterator_vtable)
    && gta_cmp_reg_reg__x86_64(v, GTA_X86_64_R1, GTA_X86_64_R2)
    && gta_jcc__x86_64(v, GTA_CC_NE, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, end_of_loop, v->count - 4)

  // 4. Save the iterator.
  //   mov [r12 + iterator_stack_location_offset], rax
    && gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, iterator_stack_location_offset, GTA_REG_RAX)

  // 5. Call the iterator next.
  // gta_computed_value_iterator_iterator_next(RAX, context). Result in RAX.
  //   mov GTA_X86_64_R1, rax
  // top_of_loop:
  //   mov GTA_X86_64_R2, context
    && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R1, GTA_REG_RAX)
    && gta_compiler_context_set_label(context, top_of_loop, v->count)
    && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R2, GTA_REG_R15)
    && gta_binary_call__x86_64(v, (uint64_t)&gta_computed_value_iterator_iterator_next)

  // 6. If the iterator next fails, jump to the end of the loop.
  //  mov GTA_X86_64_R1, gta_computed_value_error_iterator_end
  //  cmp rax, GTA_X86_64_R1
  //  je end_of_loop
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_R1, (int64_t)gta_computed_value_error_iterator_end)
    && gta_cmp_reg_reg__x86_64(v, GTA_REG_RAX, GTA_X86_64_R1)
    && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, end_of_loop, v->count - 4)

  // 7. Adopt the value.
    && gta_binary_adopt__x86_64(context, GTA_REG_RAX, GTA_REG_RDX, GTA_REG_R8, GTA_REG_R9)

  // 8. Assign the iterator value to the ranged-for variable.
  //   mov [REG(12 or 13) + identifier_stack_location_offset], rax
    && gta_mov_ind_reg__x86_64(v, identifier_is_local ? GTA_REG_R12 : GTA_REG_R13, GTA_REG_NONE, 0, identifier_stack_location_offset, GTA_REG_RAX)

  // 9. Execute the block.
    && gta_ast_node_compile_to_binary__x86_64(ranged_for->block, context)

  // 10. Load the iterator.
  // get_next_iterator_value:
  //   mov GTA_X86_64_R1, [r12 + iterator_stack_location_offset]
    && gta_compiler_context_set_label(context, get_next_iterator_value, v->count)
    && gta_mov_reg_ind__x86_64(v, GTA_X86_64_R1, GTA_REG_R12, GTA_REG_NONE, 0, iterator_stack_location_offset)

  // 11. Jump to 5.
  //   jmp top_of_loop
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, top_of_loop, v->count - 4)

  // end_of_loop:
    && gta_compiler_context_set_label(context, end_of_loop, v->count)
  // Restore the original break and continue labels.
    && ((context->break_label = original_break_label) >= 0)
    && ((context->continue_label = original_continue_label) >= 0);
}
