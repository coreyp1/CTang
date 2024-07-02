
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeBinary.h>
#include <tang/ast/astNodeDoWhile.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_do_while_vtable = {
  .name = "DoWhile",
  .compile_to_bytecode = gta_ast_node_do_while_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_do_while_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_do_while_destroy,
  .print = gta_ast_node_do_while_print,
  .simplify = gta_ast_node_do_while_simplify,
  .analyze = gta_ast_node_do_while_analyze,
  .walk = gta_ast_node_do_while_walk,
};


GTA_Ast_Node_Do_While * gta_ast_node_do_while_create(GTA_Ast_Node * condition, GTA_Ast_Node * block, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Do_While * self = gcu_malloc(sizeof(GTA_Ast_Node_Do_While));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Do_While) {
    .base = {
      .vtable = &gta_ast_node_do_while_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .condition = condition,
    .block = block,
  };
  return self;
}


void gta_ast_node_do_while_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Do_While * do_while = (GTA_Ast_Node_Do_While *) self;
  gta_ast_node_destroy(do_while->condition);
  gta_ast_node_destroy(do_while->block);
  gcu_free(self);
}


void gta_ast_node_do_while_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Do_While * do_while = (GTA_Ast_Node_Do_While *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  printf("%s%s\n", indent, self->vtable->name);
  printf("%s  Condition:\n", indent);
  gta_ast_node_print(do_while->condition, new_indent);
  printf("%s  Block:\n", indent);
  gta_ast_node_print(do_while->block, new_indent);
  gcu_free(new_indent);
}


/**
 * This function is used to find all the assignments in the while loop.
 */
static void findAssignments(GTA_Ast_Node * self, void * data, void * error) {
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


GTA_Ast_Node * gta_ast_node_do_while_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Do_While * do_while = (GTA_Ast_Node_Do_While *) self;

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

  gta_ast_node_walk(do_while->condition, findAssignments, assignments, &error);
  gta_ast_node_walk(do_while->block, findAssignments, assignments, &error);
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

  GTA_Ast_Node * simplified_condition = gta_ast_node_simplify(do_while->condition, condition_variable_map);
  if (simplified_condition) {
    gta_ast_node_destroy(do_while->condition);
    do_while->condition = simplified_condition;
  }
  GTA_Ast_Node * simplified_block = gta_ast_node_simplify(do_while->block, block_variable_map);
  if (simplified_block) {
    gta_ast_node_destroy(do_while->block);
    do_while->block = simplified_block;
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


void gta_ast_node_do_while_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Do_While * do_while = (GTA_Ast_Node_Do_While *) self;
  gta_ast_node_walk(do_while->condition, callback, data, return_value);
  gta_ast_node_walk(do_while->block, callback, data, return_value);
}


GTA_Ast_Node * gta_ast_node_do_while_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  GTA_Ast_Node_Do_While * do_while = (GTA_Ast_Node_Do_While *) self;
  GTA_Ast_Node * error = gta_ast_node_analyze(do_while->condition, program, scope);
  return error ? error : gta_ast_node_analyze(do_while->block, program, scope);
}


bool gta_ast_node_do_while_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Do_While * do_while = (GTA_Ast_Node_Do_While *) self;

  // Jump labels.
  GTA_Integer start_label;

  // Compile the do-while loop.
  return true
  // Create jump labels.
    && ((start_label = gta_bytecode_compiler_context_get_label(context)) >= 0)
  // Put something on the stack so that the first instruction of the block has
  // something to pop.  When the condition is true, the condition will still be
  // on the stack and so the first instruction of the block will be to pop it.
  // NULL
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_NULL))
  // start_label:            ; Start of the do-while loop
    && gta_bytecode_compiler_context_set_label(context, start_label, context->program->bytecode->count)
  // POP
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
  // Compile the code block.
    && gta_ast_node_compile_to_bytecode(do_while->block, context)
  // POP
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
  // Compile the condition.
    && gta_ast_node_compile_to_bytecode(do_while->condition, context)
  // JMPT start_label
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMPT))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_bytecode_compiler_context_add_label_jump(context, start_label, context->program->bytecode->count - 1)
  // The condition is left on the stack.
  ;
}


bool gta_ast_node_do_while_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_Do_While * do_while_node = (GTA_Ast_Node_Do_While *) self;
  GCU_Vector8 * v = context->binary_vector;

  // Jump labels.
  GTA_Integer block_start;

  // Offsets.
  bool * is_true_offset = &((GTA_Computed_Value *)0)->is_true;

  // Compile the while loop.
  return true
  // Create jump labels.
    && ((block_start = gta_compiler_context_get_label(context)) >= 0)
  // block_start:            ; Start of the while loop
    && gta_compiler_context_set_label(context, block_start, v->count)
  // Compile the code block.
    && gta_ast_node_compile_to_binary__x86_64(do_while_node->block, context)
  // Compile the condition.
    && gta_ast_node_compile_to_binary__x86_64(do_while_node->condition, context)
  // ; The condition result is in RAX.
  //   cmp byte ptr [rax + is_true_offset], 0
  //   jne block_start
    && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (int64_t)is_true_offset, 0)
    && gta_jcc__x86_64(v, GTA_CC_NE, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, block_start, v->count - 4);
}
