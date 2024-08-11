
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeBinary.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeWhile.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_while_vtable = {
  .name = "While",
  .compile_to_bytecode = gta_ast_node_while_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_while_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_while_destroy,
  .print = gta_ast_node_while_print,
  .simplify = gta_ast_node_while_simplify,
  .analyze = gta_ast_node_while_analyze,
  .walk = gta_ast_node_while_walk,
};


GTA_Ast_Node_While * gta_ast_node_while_create(GTA_Ast_Node * condition, GTA_Ast_Node * block, GTA_PARSER_LTYPE location) {
  assert(condition);
  assert(block);

  GTA_Ast_Node_While * self = gcu_malloc(sizeof(GTA_Ast_Node_While));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_While) {
    .base = {
      .vtable = &gta_ast_node_while_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .condition = condition,
    .block = block,
  };
  return self;
}


void gta_ast_node_while_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_WHILE(self));
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;

  gta_ast_node_destroy(while_node->condition);
  gta_ast_node_destroy(while_node->block);
  gcu_free(self);
}


void gta_ast_node_while_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_WHILE(self));
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;

  assert(indent);
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);

  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s\n", indent, self->vtable->name);

  printf("%s  Condition:\n", indent);
  gta_ast_node_print(while_node->condition, new_indent);

  printf("%s  Block:\n", indent);
  gta_ast_node_print(while_node->block, new_indent);
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


GTA_Ast_Node * gta_ast_node_while_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_WHILE(self));
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;

  // Both the condition and the block may be executed multiple times, so we
  // cannot allow any nodes to be simplified that are also assignments at any
  // point in the condition or code block.
  // Use a walk function to find all the assignments in the while loop.
  // If any part of this fails, we must invalidate all the assignments in the
  // variable map, because we cannot guarantee that assignments did not happen.
  GCU_Hash64 * assignments = gcu_hash64_create(0);
  bool error = false;
  if (!assignments) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    return 0;
  }

  gta_ast_node_walk(while_node->condition, findAssignments, assignments, &error);
  gta_ast_node_walk(while_node->block, findAssignments, assignments, &error);
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

  // Create copies of the variable map to pass to the condition and block.
  GTA_Ast_Simplify_Variable_Map * condition_variable_map = gcu_hash64_clone(variable_map);
  GTA_Ast_Simplify_Variable_Map * block_variable_map = gcu_hash64_clone(variable_map);
  if (!condition_variable_map || !block_variable_map) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    gcu_hash64_destroy(assignments);
    gcu_hash64_destroy(condition_variable_map);
    gcu_hash64_destroy(block_variable_map);
    return 0;
  }

  GTA_Ast_Node * simplified_condition = gta_ast_node_simplify(while_node->condition, condition_variable_map);
  if (simplified_condition) {
    gta_ast_node_destroy(while_node->condition);
    while_node->condition = simplified_condition;
  }
  GTA_Ast_Node * simplified_block = gta_ast_node_simplify(while_node->block, block_variable_map);
  if (simplified_block) {
    gta_ast_node_destroy(while_node->block);
    while_node->block = simplified_block;
  }

  // Synchronize the variable maps.
  gta_ast_simplify_variable_map_synchronize(variable_map, condition_variable_map);
  gta_ast_simplify_variable_map_synchronize(variable_map, block_variable_map);

  // Destroy the temporary variable maps.
  gcu_hash64_destroy(assignments);
  gcu_hash64_destroy(condition_variable_map);
  gcu_hash64_destroy(block_variable_map);

  return 0;
}


GTA_Ast_Node * gta_ast_node_while_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_WHILE(self));
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;

  GTA_Ast_Node * error = gta_ast_node_analyze(while_node->condition, program, scope);
  return error ? error : gta_ast_node_analyze(while_node->block, program, scope);
}


void gta_ast_node_while_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_WHILE(self));
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;

  callback(self, data, return_value);

  gta_ast_node_walk(while_node->condition, callback, data, return_value);
  gta_ast_node_walk(while_node->block, callback, data, return_value);
}


bool gta_ast_node_while_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_WHILE(self));
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);

  // Jump labels.
  GTA_Integer condition_start;
  GTA_Integer block_end;
  GTA_Integer original_break_label = context->break_label;
  GTA_Integer original_continue_label = context->continue_label;

  // Compile the while loop.
  return true
  // Create jump labels.
    && ((context->continue_label = condition_start = gta_compiler_context_get_label(context)) >= 0)
    && ((context->break_label = block_end = gta_compiler_context_get_label(context)) >= 0)
  // condition_start:        ; Start of the while loop
    && gta_compiler_context_set_label(context, condition_start, context->program->bytecode->count)
  // Compile the condition.
    && gta_ast_node_compile_to_bytecode(while_node->condition, context)
  // JMPF block_end          ; Value is not popped
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMPF))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(context, block_end, context->program->bytecode->count - 1)
  // POP                     ; Pop the condition value
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
  // Compile the code block.
    && gta_ast_node_compile_to_bytecode(while_node->block, context)
  // POP
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
  // JMP condition_start
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(context, condition_start, context->program->bytecode->count - 1)
  // block_end:
    && gta_compiler_context_set_label(context, block_end, context->program->bytecode->count)
  // Restore the original break and continue labels.
    && ((context->break_label = original_break_label) >= 0)
    && ((context->continue_label = original_continue_label) >= 0)
  ;
}


bool gta_ast_node_while_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_WHILE(self));
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  // Jump labels.
  GTA_Integer condition_start;
  GTA_Integer block_end;
  GTA_Integer original_break_label = context->break_label;
  GTA_Integer original_continue_label = context->continue_label;

  // Offsets.
  bool * is_true_offset = &((GTA_Computed_Value *)0)->is_true;

  // Compile the while loop.
  return true
  // Create jump labels.
    && ((context->continue_label = condition_start = gta_compiler_context_get_label(context)) >= 0)
    && ((context->break_label = block_end = gta_compiler_context_get_label(context)) >= 0)
  // condition_start:        ; Start of the while loop
    && gta_compiler_context_set_label(context, condition_start, v->count)
  // Compile the condition.
    && gta_ast_node_compile_to_binary__x86_64(while_node->condition, context)
  // ; The condition result is in RAX.
  //   cmp byte ptr [rax + is_true_offset], 0
  //   je block_end
    && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (int64_t)is_true_offset, 0)
    && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, block_end, v->count - 4)
  // Compile the code block.
    && gta_ast_node_compile_to_binary__x86_64(while_node->block, context)
  // jmp condition_start
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, condition_start, v->count - 4)
  // block_end:
    && gta_compiler_context_set_label(context, block_end, v->count)
  // Restore the original break and continue labels.
    && ((context->break_label = original_break_label) >= 0)
    && ((context->continue_label = original_continue_label) >= 0);
}
