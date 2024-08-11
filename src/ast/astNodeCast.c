
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeBoolean.h>
#include <tang/ast/astNodeCast.h>
#include <tang/ast/astNodeFloat.h>
#include <tang/ast/astNodeInteger.h>
#include <tang/ast/astNodeString.h>
#include <tang/computedValue/computedValue.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/computedValue/computedValueString.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_cast_vtable = {
  .name = "Cast",
  .compile_to_bytecode = gta_ast_node_cast_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_cast_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_cast_destroy,
  .print = gta_ast_node_cast_print,
  .simplify = gta_ast_node_cast_simplify,
  .analyze = gta_ast_node_cast_analyze,
  .walk = gta_ast_node_cast_walk,
};


GTA_Ast_Node_Cast * gta_ast_node_cast_create(GTA_Ast_Node * expression, GTA_Cast_Type type, GTA_PARSER_LTYPE location) {
  assert(expression);

  GTA_Ast_Node_Cast * self = gcu_malloc(sizeof(GTA_Ast_Node_Cast));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Cast) {
    .base = {
      .vtable = &gta_ast_node_cast_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .expression = expression,
    .type = type,
  };
  return self;
}


void gta_ast_node_cast_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_CAST(self));
  GTA_Ast_Node_Cast * cast = (GTA_Ast_Node_Cast *) self;

  if (cast->expression) {
    gta_ast_node_destroy(cast->expression);
  }
  gcu_free(self);
}


void gta_ast_node_cast_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_CAST(self));
  GTA_Ast_Node_Cast * cast = (GTA_Ast_Node_Cast *) self;

  assert(indent);
  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s as %s:\n", indent, self->vtable->name,
    cast->type == GTA_CAST_TYPE_INTEGER
      ? "integer"
      : cast->type == GTA_CAST_TYPE_FLOAT
        ? "float"
        : cast->type == GTA_CAST_TYPE_BOOLEAN
          ? "boolean"
          : cast->type == GTA_CAST_TYPE_STRING
            ? "string"
            : "unknown");

  size_t indent_length = strlen(indent);
  char * new_indent = gcu_malloc(indent_length + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_length + 1);
  memcpy(new_indent + indent_length, "  ", 3);
  gta_ast_node_print(cast->expression, new_indent);
  gcu_free(new_indent);
}


/**
 * Helper function to remove trailing zeroes from a string.
 *
 * If the string ends with a decimal point, it is also removed.
 *
 * @param buffer The buffer to remove trailing zeroes from.
 */
static void remove_trailing_zeroes(char * buffer) {
  assert(buffer);
  size_t length = strlen(buffer);
  // Don't do anything if there isn't a decimal point.
  if (!strchr(buffer, '.')) {
    return;
  }
  while (length > 1 && buffer[length - 1] == '0') {
    buffer[length - 1] = 0;
    length--;
  }
  if (buffer[length - 1] == '.') {
    buffer[length - 1] = 0;
  }
}


GTA_Ast_Node * gta_ast_node_cast_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_CAST(self));
  GTA_Ast_Node_Cast * cast = (GTA_Ast_Node_Cast *)self;

  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(cast->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(cast->expression);
    cast->expression = simplified_expression;
  }

  if (GTA_AST_IS_INTEGER(cast->expression)) {
    switch(cast->type) {
      case GTA_CAST_TYPE_INTEGER: {
        GTA_Ast_Node * expression = cast->expression;
        cast->expression = 0;
        return expression;
      }
      case GTA_CAST_TYPE_FLOAT:
        return (GTA_Ast_Node *)gta_ast_node_float_create(((GTA_Ast_Node_Integer *)cast->expression)->value, cast->base.location);
      case GTA_CAST_TYPE_BOOLEAN:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(((GTA_Ast_Node_Integer *)cast->expression)->value, cast->base.location);
      case GTA_CAST_TYPE_STRING: {
        // Convert the integer to a string.
        const size_t BUFFER_SIZE = 32;
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%ld", ((GTA_Ast_Node_Integer *)cast->expression)->value);
        GTA_Unicode_String * string = gta_unicode_string_create(buffer, strlen(buffer), GTA_UNICODE_STRING_TYPE_TRUSTED);
        if (!string) {
          return 0;
        }
        GTA_Ast_Node * result = (GTA_Ast_Node *)gta_ast_node_string_create(string, cast->base.location);
        if (!result) {
          gta_unicode_string_destroy(string);
        }
        return result;
      }
    }
  }
  else if (GTA_AST_IS_FLOAT(cast->expression)) {
    switch(cast->type) {
      case GTA_CAST_TYPE_INTEGER:
        return (GTA_Ast_Node *)gta_ast_node_integer_create((int64_t)((GTA_Ast_Node_Float *)cast->expression)->value, cast->base.location);
      case GTA_CAST_TYPE_FLOAT: {
        GTA_Ast_Node * expression = cast->expression;
        cast->expression = 0;
        return expression;
      }
      case GTA_CAST_TYPE_BOOLEAN:
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(((GTA_Ast_Node_Float *)cast->expression)->value, cast->base.location);
      case GTA_CAST_TYPE_STRING: {
        // Convert the float to a string.
        const size_t BUFFER_SIZE = 32;
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%f", ((GTA_Ast_Node_Float *)cast->expression)->value);
        remove_trailing_zeroes(buffer);
        GTA_Unicode_String * string = gta_unicode_string_create(buffer, strlen(buffer), GTA_UNICODE_STRING_TYPE_TRUSTED);
        if (!string) {
          return 0;
        }
        GTA_Ast_Node * result = (GTA_Ast_Node *)gta_ast_node_string_create(string, cast->base.location);
        if (!result) {
          gta_unicode_string_destroy(string);
        }
        return result;
      }
    }
  }
  else if (GTA_AST_IS_BOOLEAN(cast->expression)) {
    switch(cast->type) {
      case GTA_CAST_TYPE_INTEGER:
        return (GTA_Ast_Node *)gta_ast_node_integer_create(((GTA_Ast_Node_Boolean *)cast->expression)->value, cast->base.location);
      case GTA_CAST_TYPE_FLOAT:
        return (GTA_Ast_Node *)gta_ast_node_float_create(((GTA_Ast_Node_Boolean *)cast->expression)->value, cast->base.location);
      case GTA_CAST_TYPE_BOOLEAN: {
        GTA_Ast_Node * expression = cast->expression;
        cast->expression = 0;
        return expression;
      }
      case GTA_CAST_TYPE_STRING: {
        // Convert the boolean to a string.
        GTA_Unicode_String * string = ((GTA_Ast_Node_Boolean *)cast->expression)->value
          ? gta_unicode_string_create("true", 4, GTA_UNICODE_STRING_TYPE_TRUSTED)
          : gta_unicode_string_create("false", 5, GTA_UNICODE_STRING_TYPE_TRUSTED);
        if (!string) {
          return 0;
        }
        GTA_Ast_Node * result = (GTA_Ast_Node *)gta_ast_node_string_create(string, cast->base.location);
        if (!result) {
          gta_unicode_string_destroy(string);
        }
        return result;
      }
    }
  }
  else if (GTA_AST_IS_STRING(cast->expression)) {
    GTA_Ast_Node_String * string = (GTA_Ast_Node_String *)cast->expression;
    switch(cast->type) {
      case GTA_CAST_TYPE_INTEGER: {
        // Convert the string to an integer.
        // First, optimistically parse the string as a float.
        // Only accept it if the entire string was parsed as a float.
        char * tailptr = 0;
        GCU_float64_t float_value = strtold(string->string->buffer, &tailptr);
        if (tailptr && tailptr == string->string->buffer + string->string->byte_length) {
          // The entire string was parsed as a float.
          return (GTA_Ast_Node *)gta_ast_node_integer_create((int64_t)float_value, cast->base.location);
        }
        break;
      }
      case GTA_CAST_TYPE_FLOAT: {
        // Convert the string to a float.
        // Only accept it if the entire string was parsed as a float.
        char * tailptr = 0;
        GCU_float64_t float_value = strtold(string->string->buffer, &tailptr);
        if (tailptr && tailptr == string->string->buffer + ((GTA_Ast_Node_String *)cast->expression)->string->byte_length) {
          // The entire string was parsed as a float.
          return (GTA_Ast_Node *)gta_ast_node_float_create(float_value, cast->base.location);
        }
        break;
      }
      case GTA_CAST_TYPE_BOOLEAN: {
        // And empty string is false, everything else is true.
        return (GTA_Ast_Node *)gta_ast_node_boolean_create(string->string->byte_length, cast->base.location);
      }
      case GTA_CAST_TYPE_STRING: {
        GTA_Ast_Node * expression = cast->expression;
        cast->expression = 0;
        return expression;
      }
    }
  }
  return 0;
}


void gta_ast_node_cast_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_CAST(self));
  GTA_Ast_Node_Cast * cast = (GTA_Ast_Node_Cast *) self;

  callback(self, data, return_value);
  callback(cast->expression, data, return_value);
}


bool gta_ast_node_cast_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_CAST(self));
  GTA_Ast_Node_Cast * cast = (GTA_Ast_Node_Cast *) self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  return true
    && gta_ast_node_compile_to_binary__x86_64(cast->expression, context)
  // gta_computed_value_cast(rax, cast->type, context)
  //   mov rdi, rax
  //   mov rsi, cast->type
  //   mov rdx, context
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RDI, GTA_REG_RAX)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RSI,(GTA_UInteger)(
      cast->type == GTA_CAST_TYPE_INTEGER
        ? &gta_computed_value_integer_vtable
        : cast->type == GTA_CAST_TYPE_FLOAT
          ? &gta_computed_value_float_vtable
          : cast->type == GTA_CAST_TYPE_BOOLEAN
            ? &gta_computed_value_boolean_vtable
            : cast->type == GTA_CAST_TYPE_STRING
              ? &gta_computed_value_string_vtable
              : &gta_computed_value_null_vtable))
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RDX, GTA_REG_R15)
  // gta_computed_value_cast(RAX, cast->type, context)
    && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_cast);
}


bool gta_ast_node_cast_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_CAST(self));
  GTA_Ast_Node_Cast * cast = (GTA_Ast_Node_Cast *) self;

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  return true
    && gta_ast_node_compile_to_bytecode(cast->expression, context)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_CAST))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_P(
      cast->type == GTA_CAST_TYPE_INTEGER
        ? &gta_computed_value_integer_vtable
        : cast->type == GTA_CAST_TYPE_FLOAT
          ? &gta_computed_value_float_vtable
          : cast->type == GTA_CAST_TYPE_BOOLEAN
            ? &gta_computed_value_boolean_vtable
            : cast->type == GTA_CAST_TYPE_STRING
              ? &gta_computed_value_string_vtable
              : &gta_computed_value_null_vtable));
}


GTA_Ast_Node * gta_ast_node_cast_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_CAST(self));
  GTA_Ast_Node_Cast * cast = (GTA_Ast_Node_Cast *) self;

  return gta_ast_node_analyze(cast->expression, program, scope);
}
