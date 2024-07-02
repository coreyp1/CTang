
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeBinary.h>
#include <tang/ast/astNodeFor.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/program/binary.h>


// Helper macro to determine if a location indicates that the node was
// actually present in the code (as opposed to being added by the parser as a
// default value).
#define LOCATION_IS_PRESENT(location) ((location).first_line || (location).first_column || (location).last_line || (location).last_column)


GTA_Ast_Node_VTable gta_ast_node_for_vtable = {
  .name = "For",
  .compile_to_bytecode = gta_ast_node_for_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_for_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_for_destroy,
  .print = gta_ast_node_for_print,
  .simplify = gta_ast_node_for_simplify,
  .analyze = gta_ast_node_for_analyze,
  .walk = gta_ast_node_for_walk,
};


GTA_Ast_Node_For * gta_ast_node_for_create(GTA_Ast_Node * init, GTA_Ast_Node * condition, GTA_Ast_Node * update, GTA_Ast_Node * block, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_For * self = gcu_malloc(sizeof(GTA_Ast_Node_For));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_For) {
    .base = {
      .vtable = &gta_ast_node_for_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .init = init,
    .condition = condition,
    .update = update,
    .block = block,
  };
  return self;
}


void gta_ast_node_for_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_For * for_node = (GTA_Ast_Node_For *) self;
  gta_ast_node_destroy(for_node->init);
  gta_ast_node_destroy(for_node->condition);
  gta_ast_node_destroy(for_node->update);
  gta_ast_node_destroy(for_node->block);
  gcu_free(self);
}


void gta_ast_node_for_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_For * for_node = (GTA_Ast_Node_For *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  printf("%s%s\n", indent, self->vtable->name);
  printf("%s  Init:\n", indent);
  gta_ast_node_print(for_node->init, new_indent);
  printf("%s  Condition:\n", indent);
  gta_ast_node_print(for_node->condition, new_indent);
  printf("%s  Update:\n", indent);
  gta_ast_node_print(for_node->update, new_indent);
  printf("%s  Block:\n", indent);
  gta_ast_node_print(for_node->block, new_indent);
  gcu_free(new_indent);
}


/**
 * This function is used to find all the assignments in the for() statement.
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


GTA_Ast_Node * gta_ast_node_for_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_For * for_node = (GTA_Ast_Node_For *) self;

  // The init will definitely only be executed once, so we can simplify it.
  GTA_Ast_Node * simplified_init = gta_ast_node_simplify(for_node->init, variable_map);
  if (simplified_init) {
    gta_ast_node_destroy(for_node->init);
    for_node->init = simplified_init;
  }

  // The condition, update, and block may be executed multiple times, so we
  // cannot allow any nodes to be simplified that are also assignments at any
  // point in the condition, update, or code block.
  // Use a walk function to find all the assignments in the loop.
  // If any part of this fails, we must invalidate all the assignments in the
  // variable map, because we cannot guarantee that assignments did not happen.
  GCU_Hash64 * assignments = gcu_hash64_create(0);
  bool error = false;
  if (!assignments) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    return 0;
  }

  gta_ast_node_walk(for_node->condition, findAssignments, assignments, &error);
  gta_ast_node_walk(for_node->update, findAssignments, assignments, &error);
  gta_ast_node_walk(for_node->block, findAssignments, assignments, &error);
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
  GTA_Ast_Simplify_Variable_Map * update_variable_map = gcu_hash64_clone(variable_map);
  GTA_Ast_Simplify_Variable_Map * block_variable_map = gcu_hash64_clone(variable_map);
  if (!condition_variable_map || !update_variable_map || !block_variable_map) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    gcu_hash64_destroy(assignments);
    gcu_hash64_destroy(condition_variable_map);
    gcu_hash64_destroy(update_variable_map);
    gcu_hash64_destroy(block_variable_map);
    return 0;
  }

  GTA_Ast_Node * simplified_condition = gta_ast_node_simplify(for_node->condition, condition_variable_map);
  if (simplified_condition) {
    gta_ast_node_destroy(for_node->condition);
    for_node->condition = simplified_condition;
  }
  GTA_Ast_Node * simplified_update = gta_ast_node_simplify(for_node->update, update_variable_map);
  if (simplified_update) {
    gta_ast_node_destroy(for_node->update);
    for_node->update = simplified_update;
  }
  GTA_Ast_Node * simplified_block = gta_ast_node_simplify(for_node->block, block_variable_map);
  if (simplified_block) {
    gta_ast_node_destroy(for_node->block);
    for_node->block = simplified_block;
  }

  // Synchronize the variable maps.
  gta_ast_simplify_variable_map_synchronize(variable_map, condition_variable_map);
  gta_ast_simplify_variable_map_synchronize(variable_map, update_variable_map);
  gta_ast_simplify_variable_map_synchronize(variable_map, block_variable_map);

  // Destroy the temporary variable maps.
  gcu_hash64_destroy(assignments);
  gcu_hash64_destroy(condition_variable_map);
  gcu_hash64_destroy(update_variable_map);
  gcu_hash64_destroy(block_variable_map);

  return 0;
}


void gta_ast_node_for_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_For * for_node = (GTA_Ast_Node_For *) self;
  gta_ast_node_walk(for_node->init, callback, data, return_value);
  gta_ast_node_walk(for_node->condition, callback, data, return_value);
  gta_ast_node_walk(for_node->update, callback, data, return_value);
  gta_ast_node_walk(for_node->block, callback, data, return_value);
}


GTA_Ast_Node * gta_ast_node_for_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  GTA_Ast_Node_For * for_node = (GTA_Ast_Node_For *) self;
  GTA_Ast_Node * error;
  bool has_error = false
    || (error = gta_ast_node_analyze(for_node->init, program, scope))
    || (error = gta_ast_node_analyze(for_node->condition, program, scope))
    || (error = gta_ast_node_analyze(for_node->update, program, scope))
    || (error = gta_ast_node_analyze(for_node->block, program, scope));
  return has_error ? error : 0;
}


bool gta_ast_node_for_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_For * for_node = (GTA_Ast_Node_For *) self;

  // Jump labels.
  GTA_Integer condition_start;
  GTA_Integer block_end;

  // The init, condition, and update are all optional, so account for that
  // when compiling.
  // Knowing whether the optional parts are present is determined by checking
  // the location of the nodes.  If all location values are 0, then the node
  // was not present in the code.
  bool has_init = LOCATION_IS_PRESENT(for_node->init->location);
  bool has_condition = LOCATION_IS_PRESENT(for_node->condition->location);
  bool has_update = LOCATION_IS_PRESENT(for_node->update->location);

  // Because the bytecode compilation is stack-based, and we must ensure that
  // there is always a value left on the stack even if the init, condition, or
  // update are not present, then we have to do some compilcated checks to
  // ensure that the stack is always left in a valid state.

  // Compile the for loop.
  return true
  // Create the labels.
    && ((condition_start = gta_bytecode_compiler_context_get_label(context)) >= 0)
    && ((block_end = gta_bytecode_compiler_context_get_label(context)) >= 0)
  // Compile the init.
    && (has_init
      ? (true
        && gta_ast_node_compile_to_bytecode(for_node->init, context)
      // POP
        && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
      )
      : true)
  // condition_start:
    && gta_bytecode_compiler_context_set_label(context, condition_start, context->program->bytecode->count)
  // Compile the condition.
    && (has_condition
      ? (true
        && gta_ast_node_compile_to_bytecode(for_node->condition, context)
      // JMPF block_end
        && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMPF))
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
        && gta_bytecode_compiler_context_add_label_jump(context, block_end, context->program->bytecode->count - 1)
      )
      : true)
  // Compile the block.
    && gta_ast_node_compile_to_bytecode(for_node->block, context)
  // POP
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
  // Compile the update.
    && (has_update
      ? (true
        && gta_ast_node_compile_to_bytecode(for_node->update, context)
      // POP
        && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
      )
      : true)
  // JMP condition_start
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_bytecode_compiler_context_add_label_jump(context, condition_start, context->program->bytecode->count - 1)
  // block_end:
    && gta_bytecode_compiler_context_set_label(context, block_end, context->program->bytecode->count)
  // If the condition exists, then it is left on the stack.  If it does not
  // exist, then we must push a NULL value onto the stack so that the stack is
  // always left in a valid state.
    && (has_condition
      ? true
      : (true
        && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_NULL))
      )
    )
  ;
}

bool gta_ast_node_for_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_For * for_node = (GTA_Ast_Node_For *) self;
  GCU_Vector8 * v = context->binary_vector;

  // Jump labels.
  GTA_Integer condition_start;
  GTA_Integer block_end;

  // Offsets.
  bool * is_true_offset = &((GTA_Computed_Value *)0)->is_true;

  // The init, condition, and update are all optional, so account for that
  // when compiling.
  // Knowing whether the optional parts are present is determined by checking
  // the location of the nodes.  If all location values are 0, then the node
  // was not present in the code.
  bool has_init = LOCATION_IS_PRESENT(for_node->init->location);
  bool has_condition = LOCATION_IS_PRESENT(for_node->condition->location);
  bool has_update = LOCATION_IS_PRESENT(for_node->update->location);

  // Compile the for() loop.
  return true
  // Create the labels.
    && ((condition_start = gta_compiler_context_get_label(context)) >= 0)
    && ((block_end = gta_compiler_context_get_label(context)) >= 0)
  // Compile the init.
    && (has_init
      ? gta_ast_node_compile_to_binary__x86_64(for_node->init, context)
      : true)
  // condition_start:
    && gta_compiler_context_set_label(context, condition_start, context->binary_vector->count)
  // Compile the condition.
    && (has_condition
      ? (true
        && gta_ast_node_compile_to_binary__x86_64(for_node->condition, context)
      // ; The condition result is in RAX.
      //   cmp byte ptr [rax + is_true_offset], 0
      //   je block_end
        && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (int64_t)is_true_offset, 0)
        && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
        && gta_compiler_context_add_label_jump(context, block_end, v->count - 4)
      )
      : true)
    && gta_ast_node_compile_to_binary__x86_64(for_node->condition, context)
  // Compile the block.
    && gta_ast_node_compile_to_binary__x86_64(for_node->block, context)
  // Compile the update.
    && (has_update
      ? gta_ast_node_compile_to_binary__x86_64(for_node->update, context)
      : true)
  // JMP condition_start
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, condition_start, v->count - 4)
  // block_end:
    && gta_compiler_context_set_label(context, block_end, v->count);
}
