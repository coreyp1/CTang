
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeBinary.h>
#include <tang/ast/astNodeInteger.h>
#include <tang/ast/astNodeFloat.h>
#include <tang/ast/astNodeBoolean.h>
#include <tang/ast/astNodeString.h>
#include <tang/program/binary.h>
#include <tang/unicodeString.h>

GTA_Ast_Node_VTable gta_ast_node_binary_vtable = {
  .name = "Binary",
  .compile_to_bytecode = gta_ast_node_binary_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_binary_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_binary_destroy,
  .print = gta_ast_node_binary_print,
  .simplify = gta_ast_node_binary_simplify,
  .analyze = gta_ast_node_binary_analyze,
  .walk = gta_ast_node_binary_walk,
};


GTA_Ast_Node_Binary * gta_ast_node_binary_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_Binary_Type operator_type, GTA_PARSER_LTYPE location) {
  assert(lhs);
  assert(rhs);

  GTA_Ast_Node_Binary * self = gcu_malloc(sizeof(GTA_Ast_Node_Binary));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Binary) {
    .base = {
      .vtable = &gta_ast_node_binary_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .lhs = lhs,
    .rhs = rhs,
    .operator_type = operator_type,
  };
  return self;
}


void gta_ast_node_binary_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_BINARY(self));
  GTA_Ast_Node_Binary * binary = (GTA_Ast_Node_Binary *) self;

  gta_ast_node_destroy(binary->lhs);
  gta_ast_node_destroy(binary->rhs);

  gcu_free(self);
}


void gta_ast_node_binary_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_BINARY(self));
  GTA_Ast_Node_Binary * binary = (GTA_Ast_Node_Binary *) self;

  assert(indent);
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }

  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);

  const char * operator_str = 0;
  switch(binary->operator_type) {
    case GTA_BINARY_TYPE_ADD:
      operator_str = "+";
      break;
    case GTA_BINARY_TYPE_SUBTRACT:
      operator_str = "-";
      break;
    case GTA_BINARY_TYPE_MULTIPLY:
      operator_str = "*";
      break;
    case GTA_BINARY_TYPE_DIVIDE:
      operator_str = "/";
      break;
    case GTA_BINARY_TYPE_MODULO:
      operator_str = "%";
      break;
    case GTA_BINARY_TYPE_LESS_THAN:
      operator_str = "<";
      break;
    case GTA_BINARY_TYPE_LESS_THAN_EQUAL:
      operator_str = "<=";
      break;
    case GTA_BINARY_TYPE_GREATER_THAN:
      operator_str = ">";
      break;
    case GTA_BINARY_TYPE_GREATER_THAN_EQUAL:
      operator_str = ">=";
      break;
    case GTA_BINARY_TYPE_EQUAL:
      operator_str = "==";
      break;
    case GTA_BINARY_TYPE_NOT_EQUAL:
      operator_str = "!=";
      break;
    case GTA_BINARY_TYPE_AND:
      operator_str = "&&";
      break;
    case GTA_BINARY_TYPE_OR:
      operator_str = "||";
      break;
  }

  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s (%s):\n", indent, self->vtable->name, operator_str);

  printf("%s  LHS:\n", indent);
  gta_ast_node_print(binary->lhs, new_indent);

  printf("%s  RHS:\n", indent);
  gta_ast_node_print(binary->rhs, new_indent);

  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_binary_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_BINARY(self));
  GTA_Ast_Node_Binary * binary = (GTA_Ast_Node_Binary *) self;

  GTA_Ast_Node * simplified_lhs = gta_ast_node_simplify(binary->lhs, variable_map);
  if (simplified_lhs) {
    gta_ast_node_destroy(binary->lhs);
    binary->lhs = simplified_lhs;
  }

  GTA_Ast_Node * simplified_rhs = gta_ast_node_simplify(binary->rhs, variable_map);
  if (simplified_rhs) {
    gta_ast_node_destroy(binary->rhs);
    binary->rhs = simplified_rhs;
  }

  if (GTA_AST_IS_INTEGER(binary->lhs) && GTA_AST_IS_INTEGER(binary->rhs)) {
    GTA_Ast_Node_Integer * lhs = (GTA_Ast_Node_Integer *) binary->lhs;
    GTA_Ast_Node_Integer * rhs = (GTA_Ast_Node_Integer *) binary->rhs;
    switch (binary->operator_type) {
      case GTA_BINARY_TYPE_ADD:
        return (GTA_Ast_Node *)gta_ast_node_integer_create(lhs->value + rhs->value, self->location);
      case GTA_BINARY_TYPE_SUBTRACT:
        return (GTA_Ast_Node *)gta_ast_node_integer_create(lhs->value - rhs->value, self->location);
      case GTA_BINARY_TYPE_MULTIPLY:
        return (GTA_Ast_Node *)gta_ast_node_integer_create(lhs->value * rhs->value, self->location);
      case GTA_BINARY_TYPE_DIVIDE:
        if (rhs->value == 0) {
          // Let the Run-time generate an error.
          return 0;
        }
        return (GTA_Ast_Node *)gta_ast_node_integer_create(lhs->value / rhs->value, self->location);
      case GTA_BINARY_TYPE_MODULO:
        if (rhs->value == 0) {
          // Let the Run-time generate an error.
          return 0;
        }
        return (GTA_Ast_Node *)gta_ast_node_integer_create(lhs->value % rhs->value, self->location);
      case GTA_BINARY_TYPE_LESS_THAN:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs->value < rhs->value, self->location);
      case GTA_BINARY_TYPE_LESS_THAN_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs->value <= rhs->value, self->location);
      case GTA_BINARY_TYPE_GREATER_THAN:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs->value > rhs->value, self->location);
      case GTA_BINARY_TYPE_GREATER_THAN_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs->value >= rhs->value, self->location);
      case GTA_BINARY_TYPE_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs->value == rhs->value, self->location);
      case GTA_BINARY_TYPE_NOT_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs->value != rhs->value, self->location);
      case GTA_BINARY_TYPE_AND:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs->value && rhs->value, self->location);
      case GTA_BINARY_TYPE_OR:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs->value || rhs->value, self->location);
    }
  }
  else if (GTA_AST_IS_NUMERIC(binary->lhs) && GTA_AST_IS_NUMERIC(binary->rhs)) {
    GCU_float64_t lhs = GTA_AST_IS_INTEGER(binary->lhs)
      ? ((GTA_Ast_Node_Integer *) binary->lhs)->value
      : ((GTA_Ast_Node_Float *) binary->lhs)->value;
    GCU_float64_t rhs = GTA_AST_IS_INTEGER(binary->rhs)
      ? ((GTA_Ast_Node_Integer *) binary->rhs)->value
      : ((GTA_Ast_Node_Float *) binary->rhs)->value;
    switch (binary->operator_type) {
      case GTA_BINARY_TYPE_ADD:
        return (GTA_Ast_Node *)gta_ast_node_float_create(lhs + rhs, self->location);
      case GTA_BINARY_TYPE_SUBTRACT:
        return (GTA_Ast_Node *)gta_ast_node_float_create(lhs - rhs, self->location);
      case GTA_BINARY_TYPE_MULTIPLY:
        return (GTA_Ast_Node *)gta_ast_node_float_create(lhs * rhs, self->location);
      case GTA_BINARY_TYPE_DIVIDE:
        if (rhs == 0) {
          // Let the Run-time generate an error.
          return 0;
        }
        return (GTA_Ast_Node *)gta_ast_node_float_create(lhs / rhs, self->location);
      case GTA_BINARY_TYPE_MODULO:
        // Not defined for floats.
        // Let the Run-time generate an error.
        break;
      case GTA_BINARY_TYPE_LESS_THAN:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs < rhs, self->location);
      case GTA_BINARY_TYPE_LESS_THAN_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs <= rhs, self->location);
      case GTA_BINARY_TYPE_GREATER_THAN:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs > rhs, self->location);
      case GTA_BINARY_TYPE_GREATER_THAN_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs >= rhs, self->location);
      case GTA_BINARY_TYPE_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs == rhs, self->location);
      case GTA_BINARY_TYPE_NOT_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs != rhs, self->location);
      case GTA_BINARY_TYPE_AND:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs && rhs, self->location);
      case GTA_BINARY_TYPE_OR:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs || rhs, self->location);
    }
  }
  else if (GTA_AST_IS_STRING(binary->lhs) && GTA_AST_IS_STRING(binary->rhs)) {
    GTA_Ast_Node_String * lhs = (GTA_Ast_Node_String *) binary->lhs;
    GTA_Ast_Node_String * rhs = (GTA_Ast_Node_String *) binary->rhs;
    switch (binary->operator_type) {
      case GTA_BINARY_TYPE_ADD: {
        GTA_Unicode_String * new_string = gta_unicode_string_concat(lhs->string, rhs->string);
        if (!new_string) {
          return 0;
        }
        return (GTA_Ast_Node *)gta_ast_node_string_create(new_string, self->location);
      }
      case GTA_BINARY_TYPE_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(!strcmp(lhs->string->buffer, rhs->string->buffer), self->location);
      case GTA_BINARY_TYPE_NOT_EQUAL:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(strcmp(lhs->string->buffer, rhs->string->buffer), self->location);
      default:
        break;
    }
  }
  else if (GTA_AST_IS_PRIMITIVE(binary->lhs) && GTA_AST_IS_PRIMITIVE(binary->rhs)) {
    bool lhs = GTA_AST_IS_BOOLEAN(binary->lhs)
      ? ((GTA_Ast_Node_Boolean *) binary->lhs)->value
      : (GTA_AST_IS_INTEGER(binary->lhs)
        ? ((GTA_Ast_Node_Integer *) binary->lhs)->value
        : (GTA_AST_IS_FLOAT(binary->lhs)
          ? ((GTA_Ast_Node_Float *) binary->lhs)->value
          : ((GTA_Ast_Node_String *) binary->lhs)->string->grapheme_length));
    bool rhs = GTA_AST_IS_BOOLEAN(binary->rhs)
      ? ((GTA_Ast_Node_Boolean *) binary->rhs)->value
      : (GTA_AST_IS_INTEGER(binary->rhs)
        ? ((GTA_Ast_Node_Integer *) binary->rhs)->value
        : (GTA_AST_IS_FLOAT(binary->rhs)
          ? ((GTA_Ast_Node_Float *) binary->rhs)->value
          : ((GTA_Ast_Node_String *) binary->rhs)->string->grapheme_length));
    switch (binary->operator_type) {
      case GTA_BINARY_TYPE_AND:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs && rhs, self->location);
      case GTA_BINARY_TYPE_OR:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(lhs || rhs, self->location);
      default:
        break;
    }
  }
  return 0;
}


void gta_ast_node_binary_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_BINARY(self));
  GTA_Ast_Node_Binary * binary = (GTA_Ast_Node_Binary *) self;

  callback(self, data, return_value);

  gta_ast_node_walk(binary->lhs, callback, data, return_value);
  gta_ast_node_walk(binary->rhs, callback, data, return_value);
}


GTA_Ast_Node * gta_ast_node_binary_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_BINARY(self));
  GTA_Ast_Node_Binary * binary = (GTA_Ast_Node_Binary *) self;

  GTA_Ast_Node * result = gta_ast_node_analyze(binary->lhs, program, scope);
  return !result ? gta_ast_node_analyze(binary->rhs, program, scope) : result;
}


bool gta_ast_node_binary_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_BINARY(self));
  GTA_Ast_Node_Binary * binary_node = (GTA_Ast_Node_Binary *) self;

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);

  // Short-circuiting for AND.
  if (binary_node->operator_type == GTA_BINARY_TYPE_AND) {
    GTA_Integer lhs_was_false;
    return true
      && ((lhs_was_false = gta_compiler_context_get_label(context)) >= 0)
      && gta_ast_node_compile_to_bytecode(binary_node->lhs, context)
      && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMPF))
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
      && gta_compiler_context_add_label_jump(context, lhs_was_false, context->program->bytecode->count - 1)
      && gta_ast_node_compile_to_bytecode(binary_node->rhs, context)
      && gta_compiler_context_set_label(context, lhs_was_false, context->program->bytecode->count);
  }

  // Short-circuiting for OR.
  if (binary_node->operator_type == GTA_BINARY_TYPE_OR) {
    GTA_Integer lhs_was_true;
    return true
      && ((lhs_was_true = gta_compiler_context_get_label(context)) >= 0)
      && gta_ast_node_compile_to_bytecode(binary_node->lhs, context)
      && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMPT))
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
      && gta_compiler_context_add_label_jump(context, lhs_was_true, context->program->bytecode->count - 1)
      && gta_ast_node_compile_to_bytecode(binary_node->rhs, context)
      && gta_compiler_context_set_label(context, lhs_was_true, context->program->bytecode->count);
  }

  bool error_free = true
    && gta_ast_node_compile_to_bytecode(binary_node->lhs, context)
    && gta_ast_node_compile_to_bytecode(binary_node->rhs, context)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count);
  switch (binary_node->operator_type) {
    case GTA_BINARY_TYPE_ADD:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_ADD));
      break;
    case GTA_BINARY_TYPE_SUBTRACT:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_SUBTRACT));
      break;
    case GTA_BINARY_TYPE_MULTIPLY:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_MULTIPLY));
      break;
    case GTA_BINARY_TYPE_DIVIDE:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_DIVIDE));
      break;
    case GTA_BINARY_TYPE_MODULO:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_MODULO));
      break;
    case GTA_BINARY_TYPE_LESS_THAN:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_LESS_THAN));
      break;
    case GTA_BINARY_TYPE_LESS_THAN_EQUAL:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_LESS_THAN_EQUAL));
      break;
    case GTA_BINARY_TYPE_GREATER_THAN:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_GREATER_THAN));
      break;
    case GTA_BINARY_TYPE_GREATER_THAN_EQUAL:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_GREATER_THAN_EQUAL));
      break;
    case GTA_BINARY_TYPE_EQUAL:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_EQUAL));
      break;
    case GTA_BINARY_TYPE_NOT_EQUAL:
      error_free = error_free && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_NOT_EQUAL));
      break;
    default:
      error_free = false;
  }
  return error_free;
}


bool gta_ast_node_binary_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_BINARY(self));
  GTA_Ast_Node_Binary * binary_node = (GTA_Ast_Node_Binary *) self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  // Short-circuiting for AND.
  if (binary_node->operator_type == GTA_BINARY_TYPE_AND) {
    bool * is_true_offset = &((GTA_Computed_Value *)0)->is_true;
    GTA_Integer lhs_was_false;
    return true
      && ((lhs_was_false = gta_compiler_context_get_label(context)) >= 0)
      && gta_ast_node_compile_to_binary__x86_64(binary_node->lhs, context)
    // The result of the LHS is in rax.  If it is false, then we do not need to
    // evaluate the RHS.
    //   cmp byte ptr [rax + is_true_offset], 1
    //   jnz lhs_was_false
      && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (int64_t)is_true_offset, 1)
      && gta_jcc__x86_64(v, GTA_CC_NZ, 0xDEADBEEF)
      && gta_compiler_context_add_label_jump(context, lhs_was_false, v->count - 4)
    // Compile the RHS expression.  The result will be in rax.
      && gta_ast_node_compile_to_binary__x86_64(binary_node->rhs, context)
    // Set the `lhs_was_false` label to the current offset.
      && gta_compiler_context_set_label(context, lhs_was_false, v->count);
  }

  // Short-circuiting for OR.
  if (binary_node->operator_type == GTA_BINARY_TYPE_OR) {
    bool * is_true_offset = &((GTA_Computed_Value *)0)->is_true;
    GTA_Integer lhs_was_true;
    return true
      && ((lhs_was_true = gta_compiler_context_get_label(context)) >= 0)
      && gta_ast_node_compile_to_binary__x86_64(binary_node->lhs, context)
    // The result of the LHS is in rax.  If it is true, then we do not need to
    // evaluate the RHS.
    //   cmp byte ptr [rax + is_true_offset], 0
    //   jz lhs_was_true
      && gta_cmp_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (int64_t)is_true_offset, 0)
      && gta_jcc__x86_64(v, GTA_CC_NZ, 0xDEADBEEF)
      && gta_compiler_context_add_label_jump(context, lhs_was_true, v->count - 4)
    // Compile the RHS expression.  The result will be in rax.
      && gta_ast_node_compile_to_binary__x86_64(binary_node->rhs, context)
    // Set the `lhs_was_true` label to the current offset.
      && gta_compiler_context_set_label(context, lhs_was_true, v->count);
  }

  if (binary_node->operator_type < GTA_BINARY_TYPE_AND) {
    // Determine the function to call.
    GTA_Integer func = 0;
    switch(binary_node->operator_type) {
      case GTA_BINARY_TYPE_ADD:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_add);
        break;
      case GTA_BINARY_TYPE_SUBTRACT:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_subtract);
        break;
      case GTA_BINARY_TYPE_MULTIPLY:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_multiply);
        break;
      case GTA_BINARY_TYPE_DIVIDE:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_divide);
        break;
      case GTA_BINARY_TYPE_MODULO:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_modulo);
        break;
      case GTA_BINARY_TYPE_LESS_THAN:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_less_than);
        break;
      case GTA_BINARY_TYPE_LESS_THAN_EQUAL:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_less_than_equal);
        break;
      case GTA_BINARY_TYPE_GREATER_THAN:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_greater_than);
        break;
      case GTA_BINARY_TYPE_GREATER_THAN_EQUAL:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_greater_than_equal);
        break;
      case GTA_BINARY_TYPE_EQUAL:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_equal);
        break;
      case GTA_BINARY_TYPE_NOT_EQUAL:
        func = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_not_equal);
        break;
      default:
        return false;
    }

    return true
    // Compile the LHS expression.  The result will be in rax.
      && gta_ast_node_compile_to_binary__x86_64(binary_node->lhs, context)
    // "Push" the result of the LHS expression.
    //   add rsp, -16
    //   mov [rsp + GTA_SHADOW_SIZE__X86_64], rax
      && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, -16)
      && gta_mov_ind_reg__x86_64(v, GTA_REG_RSP, GTA_REG_NONE, 0, GTA_SHADOW_SIZE__X86_64, GTA_REG_RAX)
    // Compile the RHS expression.  The result will be in rax.
      && gta_ast_node_compile_to_binary__x86_64(binary_node->rhs, context)

    // Prepare registers for: func(result_from_lhs, result_from_rhs, true, is_assignment, context)
    // "Pop" the result of the LHS expression.
    // NOTE: We will not change RSP, because if compiling for Windows, it will
    // need it.  RSP will be cleaned up later in this function.
    //   mov GTA_X86_64_R1, [rsp + GTA_SHADOW_SIZE__X86_64] ; result from lhs
      && gta_mov_reg_ind__x86_64(v, GTA_X86_64_R1, GTA_REG_RSP, GTA_REG_NONE, 0, GTA_SHADOW_SIZE__X86_64)
    //   mov GTA_X86_64_R2, rax  ; result_from_rhs
    //   mov GTA_X86_64_R3, 1    ; true
    //   mov GTA_X86_64_R4, is_assignment ; is_assignment
      && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R2, GTA_REG_RAX)
      && gta_mov_reg_imm__x86_64(v, GTA_X86_64_R3, 1)
      && gta_mov_reg_imm__x86_64(v, GTA_X86_64_R4, 0)
#if defined(_WIN32) || defined(_WIN64)
    // The Windows x64 calling convention ABI requires that the fifth argument
    // be put on the stack, just above the shadow space.
    //   mov [rsp + GTA_SHADOW_SIZE__X86_64], r15 ; context (the fifth argument)
      && gta_mov_ind_reg__x86_64(v, GTA_REG_RSP, GTA_REG_NONE, 0, GTA_SHADOW_SIZE__X86_64, GTA_REG_R15)
    //   call func
      && gta_binary_call__x86_64(v, (uint64_t)func)
    // Restore the stack after the function call.
    //   add rsp, 16
      && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 16)
#else
    // Restore the stack since the extra space is not needed.
    //   add rsp, 16
      && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 16)
    //   mov r8, r15   ; context (the fifth argument)
      && gta_mov_reg_reg__x86_64(v, GTA_REG_R8, GTA_REG_R15)
    //   mov rax, func ; func
      && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (uint64_t)func)
    // Call the function.
      && gta_binary_call_reg__x86_64(v, GTA_REG_RAX)
#endif
    ;
  }
  return false;
}
