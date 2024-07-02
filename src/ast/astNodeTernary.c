
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeTernary.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_ternary_vtable = {
  .name = "Ternary",
  .compile_to_bytecode = gta_ast_node_ternary_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_ternary_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_ternary_destroy,
  .print = gta_ast_node_ternary_print,
  .simplify = gta_ast_node_ternary_simplify,
  .analyze = gta_ast_node_ternary_analyze,
  .walk = gta_ast_node_ternary_walk,
};


GTA_Ast_Node_Ternary * gta_ast_node_ternary_create(GTA_Ast_Node * condition, GTA_Ast_Node * ifTrue, GTA_Ast_Node * ifFalse, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Ternary * self = gcu_malloc(sizeof(GTA_Ast_Node_Ternary));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Ternary) {
    .base = {
      .vtable = &gta_ast_node_ternary_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .condition = condition,
    .ifTrue = ifTrue,
    .ifFalse = ifFalse,
  };
  return self;
}


void gta_ast_node_ternary_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Ternary * ternary = (GTA_Ast_Node_Ternary *)self;
  gta_ast_node_destroy(ternary->condition);
  gta_ast_node_destroy(ternary->ifTrue);
  gta_ast_node_destroy(ternary->ifFalse);
  gcu_free(self);
}


void gta_ast_node_ternary_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 5);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  GTA_Ast_Node_Ternary * ternary = (GTA_Ast_Node_Ternary *)self;
  printf("%s%s:\n", indent, self->vtable->name);
  printf("%s  Condition:\n", indent);
  gta_ast_node_print(ternary->condition, new_indent);
  printf("%s  If True:\n", indent);
  gta_ast_node_print(ternary->ifTrue, new_indent);
  printf("%s  If False:\n", indent);
  gta_ast_node_print(ternary->ifFalse, new_indent);
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_ternary_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  // The ternary operator is a bit more complicated to simplify than the other
  // nodes. Because the true and false branches can be any expression, and thus
  // could possibly contain assignments which may or may not be executed, the
  // assignments should cause the variable to be removed from the variable map.
  GTA_Ast_Node_Ternary * ternary = (GTA_Ast_Node_Ternary *)self;
  // The condition is always executed, so we don't need to worry about
  // detecting assignments in it.
  GTA_Ast_Node * simplified_condition = gta_ast_node_simplify(ternary->condition, variable_map);
  if (simplified_condition) {
    gta_ast_node_destroy(ternary->condition);
    ternary->condition = simplified_condition;
  }

  // Create copies of the variable map to pass to the true branch.
  GTA_Ast_Simplify_Variable_Map * true_variable_map = gcu_hash64_clone(variable_map);
  GTA_Ast_Simplify_Variable_Map * false_variable_map = gcu_hash64_clone(variable_map);

  // If the creation of either of the new maps failed, then we have a problem.
  // We cannot check whether or not there are assignments in the true and false
  // branches, so we cannot simplify the ternary operator.  Moreover, it is not
  // safe for the calling functions to further simplify any subsequent values
  // using the existing variable map, because some of those variables may be
  // assigned in the ternary expressions here (that we cannot check).  The only
  // safe option is to invalidate all existing values in the variable map
  // before exiting.
  if (!true_variable_map || !false_variable_map) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    if (true_variable_map) {
      gcu_hash64_destroy(true_variable_map);
    }
    if (false_variable_map) {
      gcu_hash64_destroy(false_variable_map);
    }
    return 0;
  }

  // Simplify the true branch.
  GTA_Ast_Node * simplified_ifTrue = gta_ast_node_simplify(ternary->ifTrue, true_variable_map);
  if (simplified_ifTrue) {
    gta_ast_node_destroy(ternary->ifTrue);
    ternary->ifTrue = simplified_ifTrue;
  }

  // Simplify the false branch.
  GTA_Ast_Node * simplified_ifFalse = gta_ast_node_simplify(ternary->ifFalse, false_variable_map);
  if (simplified_ifFalse) {
    gta_ast_node_destroy(ternary->ifFalse);
    ternary->ifFalse = simplified_ifFalse;
  }

  // Synchronize the variable maps.
  gta_ast_simplify_variable_map_synchronize(variable_map, true_variable_map);
  gta_ast_simplify_variable_map_synchronize(variable_map, false_variable_map);

  // Destroy the temporary variable maps.
  gcu_hash64_destroy(true_variable_map);
  gcu_hash64_destroy(false_variable_map);

  return 0;
}


void gta_ast_node_ternary_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Ternary * ternary = (GTA_Ast_Node_Ternary *)self;
  gta_ast_node_walk(ternary->condition, callback, data, return_value);
  gta_ast_node_walk(ternary->ifTrue, callback, data, return_value);
  gta_ast_node_walk(ternary->ifFalse, callback, data, return_value);
}


GTA_Ast_Node * gta_ast_node_ternary_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  GTA_Ast_Node_Ternary * ternary = (GTA_Ast_Node_Ternary *)self;
  GTA_Ast_Node * parts[] = {ternary->condition, ternary->ifTrue, ternary->ifFalse};
  for (size_t i = 0; i < 3; ++i) {
    GTA_Ast_Node * part = parts[i];
    GTA_Ast_Node * error = gta_ast_node_analyze(part, program, scope);
    if (error) {
      return error;
    }
  }
  return NULL;
}


bool gta_ast_node_ternary_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Ternary * ternary = (GTA_Ast_Node_Ternary *)self;

  // Jump labels.
  GTA_Integer false_label;
  GTA_Integer end_label;

  // Compile the expression.
  return true
  // Create jump labels.
    && ((false_label = gta_bytecode_compiler_context_get_label(context)) >= 0)
    && ((end_label = gta_bytecode_compiler_context_get_label(context)) >= 0)
  // Compile the condition.
    && gta_ast_node_compile_to_bytecode(ternary->condition, context)
  // JMPF false_label          ; Value is not popped
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMPF))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_bytecode_compiler_context_add_label_jump(context, false_label, context->program->bytecode->count - 1)
  // POP                       ; Pop the condition value
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
  // Compile the true branch.
    && gta_ast_node_compile_to_bytecode(ternary->ifTrue, context)
  // JMP end_label             ; Value is not popped
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_bytecode_compiler_context_add_label_jump(context, end_label, context->program->bytecode->count - 1)
  // false_label:
    && gta_bytecode_compiler_context_set_label(context, false_label, context->program->bytecode->count)
  // POP
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP))
  // Compile the false branch.
    && gta_ast_node_compile_to_bytecode(ternary->ifFalse, context)
  // end_label:
    && gta_bytecode_compiler_context_set_label(context, end_label, context->program->bytecode->count);
}


bool gta_ast_node_ternary_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  GTA_Ast_Node_Ternary * ternary = (GTA_Ast_Node_Ternary *)self;
  GCU_Vector8 * v = context->binary_vector;

  // Offsets.
  bool * is_true_offset = &((GTA_Computed_Value *)0)->is_true;

  // Jump labels.
  GTA_Integer false_label;
  GTA_Integer end_label;

  // Compile the expression.
  return true
  // Create jump labels.
    && ((false_label = gta_binary_compiler_context_get_label(context)) >= 0)
    && ((end_label = gta_binary_compiler_context_get_label(context)) >= 0)
  // Compile the condition.
    && gta_ast_node_compile_to_binary__x86_64(ternary->condition, context)
  // ; The condition result is in RAX.
  //   cmp byte ptr [rax + is_true_offset], 0
  //   je false_label
    && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (int64_t)is_true_offset, 0)
    && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
    && gta_binary_compiler_context_add_label_jump(context, false_label, v->count - 4)
  // Compile the true branch.
    && gta_ast_node_compile_to_binary__x86_64(ternary->ifTrue, context)
  // jmp end_label
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_binary_compiler_context_add_label_jump(context, end_label, v->count - 4)
  // false_label:
    && gta_binary_compiler_context_set_label(context, false_label, v->count)
  // Compile the false branch.
    && gta_ast_node_compile_to_binary__x86_64(ternary->ifFalse, context)
  // end_label:
    && gta_binary_compiler_context_set_label(context, end_label, v->count);
}
