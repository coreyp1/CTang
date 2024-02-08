
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include "tang/astNodeUnary.h"
#include "tang/astNodeInteger.h"
#include "tang/astNodeFloat.h"
#include "tang/astNodeString.h"
#include "tang/astNodeBoolean.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_unary_vtable = {
  .name = "Unary",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_unary_destroy,
  .print = gta_ast_node_unary_print,
  .simplify = gta_ast_node_unary_simplify,
  .walk = gta_ast_node_unary_walk,
};

GTA_Ast_Node_Unary * gta_ast_node_unary_create(GTA_Ast_Node * expression, GTA_Unary_Type operator_type, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Unary * self = gcu_malloc(sizeof(GTA_Ast_Node_Unary));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_unary_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->expression = expression;
  self->operator_type = operator_type;
  return self;
}

void gta_ast_node_unary_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Unary * unary = (GTA_Ast_Node_Unary *) self;
  gta_ast_node_destroy(unary->expression);
  gcu_free(self);
}

void gta_ast_node_unary_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Unary * unary = (GTA_Ast_Node_Unary *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 3);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);
  printf("%s%s(%s):\n", indent, self->vtable->name,
    unary->operator_type == GTA_UNARY_TYPE_NEGATIVE
      ? "-"
      : unary->operator_type == GTA_UNARY_TYPE_NOT
        ? "!"
        : "unknown");
  gta_ast_node_print(unary->expression, new_indent);
  gcu_free(new_indent);
}

GTA_Ast_Node * gta_ast_node_unary_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Unary * unary = (GTA_Ast_Node_Unary *) self;
  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(unary->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(unary->expression);
    unary->expression = simplified_expression;
  }
  if (GTA_AST_IS_INTEGER(unary->expression)) {
    GTA_Ast_Node_Integer * integer = (GTA_Ast_Node_Integer *) unary->expression;
    if (unary->operator_type == GTA_UNARY_TYPE_NEGATIVE) {
      return (GTA_Ast_Node *)gta_ast_node_integer_create(-integer->value, self->location);
    }
    return (GTA_Ast_Node *)gta_ast_node_boolean_create(!integer->value, self->location);
  }
  else if (GTA_AST_IS_FLOAT(unary->expression)) {
    GTA_Ast_Node_Float * float_ = (GTA_Ast_Node_Float *) unary->expression;
    if (unary->operator_type == GTA_UNARY_TYPE_NEGATIVE) {
      return (GTA_Ast_Node *)gta_ast_node_float_create(-float_->value, self->location);
    }
    return (GTA_Ast_Node *)gta_ast_node_boolean_create(!float_->value, self->location);
  }
  else if (GTA_AST_IS_BOOLEAN(unary->expression)) {
    GTA_Ast_Node_Boolean * boolean = (GTA_Ast_Node_Boolean *) unary->expression;
    if (unary->operator_type == GTA_UNARY_TYPE_NOT) {
      return (GTA_Ast_Node *)gta_ast_node_boolean_create(!boolean->value, self->location);
    }
  }
  else if (GTA_AST_IS_STRING(unary->expression)) {
    GTA_Ast_Node_String * string = (GTA_Ast_Node_String *) unary->expression;
    if (unary->operator_type == GTA_UNARY_TYPE_NOT) {
      return (GTA_Ast_Node *)gta_ast_node_boolean_create(!string->string->byte_length, self->location);
    }
  }
  return 0;
}

void gta_ast_node_unary_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Unary * unary = (GTA_Ast_Node_Unary *) self;
  gta_ast_node_walk(unary->expression, callback, data, return_value);
}
