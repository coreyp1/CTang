
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/hash.h>
#include <tang/ast/astNodeParseError.h>
#include <tang/ast/astNodeUse.h>
#include <tang/program/variable.h>


GTA_Ast_Node_VTable gta_ast_node_use_vtable = {
  .name = "Use",
  .compile_to_binary__x86_64 = gta_ast_node_use_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .compile_to_bytecode = gta_ast_node_use_compile_to_bytecode,
  .destroy = gta_ast_node_use_destroy,
  .print = gta_ast_node_use_print,
  .simplify = gta_ast_node_use_simplify,
  .analyze = gta_ast_node_use_analyze,
  .walk = gta_ast_node_use_walk,
};


GTA_Ast_Node_Use * gta_ast_node_use_create(const char * identifier, GTA_Ast_Node * expression, GTA_PARSER_LTYPE location) {
  assert(identifier);
  // Note: expression can be NULL.

  GTA_Ast_Node_Use * self = gcu_malloc(sizeof(GTA_Ast_Node_Use));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Use) {
    .base = {
      .vtable = &gta_ast_node_use_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .identifier = identifier,
    .hash = GTA_STRING_HASH(identifier, strlen(identifier)),
    .expression = expression,
  };
  return self;
}


void gta_ast_node_use_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_USE(self));
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;

  gcu_free((void *)use->identifier);
  if (use->expression) {
    gta_ast_node_destroy(use->expression);
  }
  gcu_free(self);
}


void gta_ast_node_use_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_USE(self));
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;

  assert(indent);
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);

  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s(%s):\n", indent, self->vtable->name, use->identifier);
  if (use->expression) {
    gta_ast_node_print(use->expression, new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_use_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_USE(self));
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;

  assert(variable_map);
  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(use->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(use->expression);
    use->expression = simplified_expression;
  }
  // If the use expression has an alias, then add it to the variable map.
  if (use->identifier) {
    gcu_hash64_set(variable_map, gcu_string_hash_64(use->identifier, strlen(use->identifier)), GCU_TYPE64_P(use->expression));
  }
  return 0;
}


GTA_Ast_Node * gta_ast_node_use_analyze(GTA_Ast_Node * self, GTA_MAYBE_UNUSED(GTA_Program * program), GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_USE(self));
  assert(scope);
  GTA_Variable_Scope * outermost_scope = scope;

  while (outermost_scope->parent_scope) {
    outermost_scope = outermost_scope->parent_scope;
  }
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;

  // Ensure that the "use" expression is in the outermost scope.
  if (scope->parent_scope) {
    GTA_Ast_Node * error = (GTA_Ast_Node *) gta_ast_node_parse_error_create("A use statement must be in the outermost scope.", self->location);
    return error ? error : gta_ast_node_parse_error_out_of_memory;
  }

  // Determine whether or not the identifier has already been used for a
  // library declaration.
  if (GTA_HASHX_CONTAINS(outermost_scope->library_declarations, use->hash)) {
    GTA_Ast_Node * error = (GTA_Ast_Node *) gta_ast_node_parse_error_create("The identifier in a use statement has already been declared in the library scope.", self->location);
    return error ? error : gta_ast_node_parse_error_out_of_memory;
  }

  // The identifier has not been used as a library.
  // Verify that it has not been used anywhere else.
  if (GTA_HASHX_CONTAINS(outermost_scope->identified_variables, use->hash)) {
    GTA_Ast_Node * error = (GTA_Ast_Node *) gta_ast_node_parse_error_create("The identifier in a use statement has already been declared.", self->location);
    return error ? error : gta_ast_node_parse_error_out_of_memory;
  }

  // Verify that it has not been used as a function name.
  if (GTA_HASHX_CONTAINS(outermost_scope->function_scopes, use->hash)) {
    GTA_Ast_Node * error = (GTA_Ast_Node *) gta_ast_node_parse_error_create("The identifier in a use statement has already been declared as a function name.", self->location);
    return error ? error : gta_ast_node_parse_error_out_of_memory;
  }

  // Add the identifier to the library scope.
  if (!GTA_HASHX_SET(scope->library_declarations, use->hash, GCU_TYPE64_P(use))) {
    return gta_ast_node_parse_error_out_of_memory;
  }

  return NULL;
}


void gta_ast_node_use_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_USE(self));
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;

  callback(self, data, return_value);
  gta_ast_node_walk(use->expression, callback, data, return_value);
}


bool gta_ast_node_use_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_USE(self));
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;

  return gta_ast_node_compile_to_binary__x86_64(use->expression, context);
}


bool gta_ast_node_use_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_USE(self));
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
  assert(use->expression);
  return gta_ast_node_compile_to_bytecode(use->expression, context);
}
