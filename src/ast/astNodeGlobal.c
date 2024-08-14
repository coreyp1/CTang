
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeGlobal.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeParseError.h>
#include <tang/program/variable.h>

GTA_Ast_Node_VTable gta_ast_node_global_vtable = {
  .name = "Global",
  .compile_to_bytecode = gta_ast_node_global_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_global_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_global_destroy,
  .print = gta_ast_node_global_print,
  .simplify = gta_ast_node_global_simplify,
  .analyze = gta_ast_node_global_analyze,
  .walk = gta_ast_node_global_walk,
};


GTA_Ast_Node_Global * gta_ast_node_global_create(GTA_Ast_Node * identifier, GTA_Ast_Node * assignment, GTA_PARSER_LTYPE location) {
  assert(identifier);
  // NOTE: assignment can be NULL.

  GTA_Ast_Node_Global * self = gcu_malloc(sizeof(GTA_Ast_Node_Global));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Global) {
    .base = {
      .vtable = &gta_ast_node_global_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .identifier = identifier,
    .assignment = assignment,
  };
  return self;
}


void gta_ast_node_global_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_GLOBAL(self));
  GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *) self;

  gta_ast_node_destroy(global->identifier);
  if (global->assignment) {
    gta_ast_node_destroy(global->assignment);
  }
  gcu_free(self);
}


void gta_ast_node_global_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_GLOBAL(self));
  GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *) self;

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
  assert(global->identifier);
  assert(GTA_AST_IS_IDENTIFIER(global->identifier));
  printf("%s%s : %s\n", indent, self->vtable->name, ((GTA_Ast_Node_Identifier *)global->identifier)->identifier);
  if (global->assignment) {
    printf("%s  Assignment:\n", indent);
    gta_ast_node_print(global->assignment, new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_global_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_GLOBAL(self));
  GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *) self;

  GTA_Ast_Node * new_assignment = global->assignment ? gta_ast_node_simplify(global->assignment, variable_map) : 0;
  if (global->assignment && new_assignment) {
    gta_ast_node_destroy(global->assignment);
    global->assignment = new_assignment;
  }
  return 0;
}


GTA_Ast_Node * gta_ast_node_global_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_GLOBAL(self));
  GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *) self;

  // Find the outermost scope.
  assert(scope);
  GTA_Variable_Scope * outermost_scope = scope;
  while (outermost_scope->parent_scope) {
    outermost_scope = outermost_scope->parent_scope;
  }

  // Check if the identifier is already defined in the current scope.
  assert(global->identifier);
  assert(GTA_AST_IS_IDENTIFIER(global->identifier));
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *)global->identifier;
  identifier->mangled_name = identifier->identifier;
  identifier->mangled_name_hash = identifier->hash;
  GTA_HashX_Value val = GTA_HASHX_GET(scope->identified_variables, identifier->hash);
  if (val.exists) {
    return gta_ast_node_parse_error_global_identifier_redeclared;
  }

  // Check if the identifier is already defined in the outermost scope.
  val = GTA_HASHX_GET(outermost_scope->identified_variables, identifier->hash);
  if (val.exists) {
    // If here, the identifier has not been defined in the current scope, but
    // has been defined in the outermost scope.  Add an identification record
    // to the current scope.
    if (!GTA_HASHX_SET(scope->identified_variables, identifier->hash, GTA_TYPEX_MAKE_P(self))) {
      return gta_ast_node_parse_error_out_of_memory;
    }
    return global->assignment ? gta_ast_node_analyze(global->assignment, program, scope) : 0;
  }

  // If here, the identifier has not been defined in the current scope or the
  // outermost scope.  Add an identification record to the outermost scope.
  if (gta_ast_node_analyze(global->identifier, program, outermost_scope)) {
    return gta_ast_node_parse_error_out_of_memory;
  }

  // Add an identification record to the current scope.
  if (!GTA_HASHX_SET(scope->identified_variables, identifier->hash, GTA_TYPEX_MAKE_P(self))) {
    return gta_ast_node_parse_error_out_of_memory;
  }

  // Process the assignment, if it exists.
  return global->assignment
    ? gta_ast_node_analyze(global->assignment, program, scope)
    : 0;
}


void gta_ast_node_global_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_GLOBAL(self));
  GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *) self;

  callback(self, data, return_value);

  gta_ast_node_walk(global->identifier, callback, data, return_value);
  if (global->assignment) {
    gta_ast_node_walk(global->assignment, callback, data, return_value);
  }
}


bool gta_ast_node_global_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_GLOBAL(self));
  GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *)self;

  if (global->assignment && !gta_ast_node_compile_to_bytecode(global->assignment, context)) {
    return false;
  }
  return true;
}


bool gta_ast_node_global_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_GLOBAL(self));
  GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *)self;

  if (global->assignment && !gta_ast_node_compile_to_binary__x86_64(global->assignment, context)) {
    return false;
  }
  return true;
}
