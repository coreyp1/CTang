
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/hash.h>
#include <cutil/string.h>
#include <tang/ast/astNodeBoolean.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeInteger.h>
#include <tang/ast/astNodeFloat.h>
#include <tang/ast/astNodeFunction.h>
#include <tang/ast/astNodeParseError.h>
#include <tang/ast/astNodeString.h>
#include <tang/ast/astNodeUse.h>
#include <tang/program/variable.h>

GTA_Ast_Node_VTable gta_ast_node_identifier_vtable = {
  .name = "Identifier",
  .compile_to_bytecode = gta_ast_node_identifier_compile_to_bytecode,
  .destroy = gta_ast_node_identifier_destroy,
  .print = gta_ast_node_identifier_print,
  .simplify = gta_ast_node_identifier_simplify,
  .analyze = gta_ast_node_identifier_analyze,
  .walk = gta_ast_node_identifier_walk,
};


GTA_Ast_Node_Identifier * gta_ast_node_identifier_create(const char * identifier, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Identifier * self = gcu_malloc(sizeof(GTA_Ast_Node_Identifier));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_identifier_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->identifier = identifier;
  self->hash = gcu_string_hash_64(identifier, strlen(identifier));
  self->mangled_name = 0;
  self->mangled_name_hash = 0;
  return self;
}


void gta_ast_node_identifier_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;
  gcu_free((void *)identifier->identifier);
  gcu_free(self);
}


void gta_ast_node_identifier_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;
  printf("%s%s: %s\n", indent, self->vtable->name, identifier->identifier);
}


GTA_Ast_Node * gta_ast_node_identifier_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  // If the identifier is in the variable map, and the value is a primitive,
  // return the primitive value.
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;
  GCU_Hash64_Value val = gcu_hash64_get(variable_map, identifier->hash);
  if (val.exists) {
    GTA_Ast_Node * node = (GTA_Ast_Node *)val.value.p;
    if (GTA_AST_IS_PRIMITIVE(node) && !GTA_AST_IS_STRING(node)) {
      // We're not doing this for strings, because it could cause a lot of
      // unnecessary copying.
      GTA_Ast_Node * new_node =
        GTA_AST_IS_INTEGER(node)
          ? (GTA_Ast_Node *)gta_ast_node_integer_create(((GTA_Ast_Node_Integer *)node)->value, self->location)
          : GTA_AST_IS_FLOAT(node)
            ? (GTA_Ast_Node *)gta_ast_node_float_create(((GTA_Ast_Node_Float *)node)->value, self->location)
            : GTA_AST_IS_BOOLEAN(node)
              ? (GTA_Ast_Node *)gta_ast_node_boolean_create(((GTA_Ast_Node_Boolean *)node)->value, self->location)
              : (GTA_Ast_Node *)gta_ast_node_create(self->location);
      if (new_node) {
        return new_node;
      }
    }
  }
  return 0;
}


GTA_Ast_Node * gta_ast_node_identifier_analyze(GTA_Ast_Node * self, GTA_MAYBE_UNUSED(GTA_Program * program), GTA_Variable_Scope * scope) {
  GTA_Variable_Scope * outermost_scope = scope;
  while (outermost_scope->parent_scope) {
    outermost_scope = outermost_scope->parent_scope;
  }
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;

  // If the identifier has already been declared as in the local scope, then
  // propagate the type and mangled name.
  GTA_HashX_Value val = GTA_HASHX_GET(scope->local_declarations, identifier->hash);
  if (val.exists) {
    // The identifier is already declared in the local scope.
    GTA_Ast_Node_Identifier * existing_identifier = (GTA_Ast_Node_Identifier *)val.value.p;
    identifier->mangled_name = existing_identifier->mangled_name;
    identifier->mangled_name_hash = existing_identifier->mangled_name_hash;
    identifier->type = existing_identifier->type;
    return 0;
  }

  // If the identifier has already been declared as a global variable, then
  // propagate the type and mangled name.
  val = GTA_HASHX_GET(scope->global_declarations, identifier->hash);
  if (val.exists) {
    // The identifier is already declared as a global variable.
    GTA_Ast_Node_Identifier * existing_identifier = (GTA_Ast_Node_Identifier *)val.value.p;
    identifier->mangled_name = existing_identifier->mangled_name;
    identifier->mangled_name_hash = existing_identifier->mangled_name_hash;
    identifier->type = existing_identifier->type;
    return 0;
  }

  // If the identifier has already been declared as a library function, then
  // propagate the type and mangled name.
  val = GTA_HASHX_GET(outermost_scope->library_declarations, identifier->hash);
  if (val.exists) {
    // Because we did not see the identifier in the local or global scopes,
    // we know that this is the first time that we have seen this
    // identifier.  Be sure to add it to the globals so that, when globals
    // are loaded, the identifier hash will be recognized.
    GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)val.value.p;
    identifier->mangled_name = use->identifier;
    identifier->mangled_name_hash = use->hash;
    identifier->type = GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY;
    if (!GTA_HASHX_SET(outermost_scope->global_declarations, identifier->hash, GTA_TYPEX_MAKE_P(use))
      || !GTA_HASHX_SET(outermost_scope->global_positions, identifier->hash, GTA_TYPEX_MAKE_UI(outermost_scope->global_positions->entries))) {
      return gta_ast_node_parse_error_out_of_memory;
    }
    return 0;
  }

  // Look for a function name in each scope, from current to outermost.
  GTA_Variable_Scope * current_scope = scope;
  size_t identifier_length = strlen(identifier->identifier);
  while (current_scope) {
    // Compute the possible mangled name based on the current_scope.
    size_t namespace_length = strlen(current_scope->name);
    size_t mangled_length = identifier_length + namespace_length + 2;
    char * mangled_name = gcu_malloc(mangled_length);
    if (!mangled_name) {
      return gta_ast_node_parse_error_out_of_memory;
    }
    snprintf(mangled_name, mangled_length, "%s/%s", current_scope->name, identifier->identifier);
    GTA_Integer mangled_name_hash = GTA_STRING_HASH(mangled_name, mangled_length);

    // If the identifier has already been declared as a function, then
    // propagate the type and mangled name.
    val = GTA_HASHX_GET(current_scope->function_scopes, mangled_name_hash);
    if (val.exists) {
      GTA_Variable_Scope * function_scope = (GTA_Variable_Scope *)val.value.p;
      GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *)function_scope->ast_node;
      identifier->mangled_name = function->mangled_name;
      identifier->mangled_name_hash = function->mangled_name_hash;
      identifier->type = GTA_AST_NODE_IDENTIFIER_TYPE_FUNCTION;
      gcu_free(mangled_name);
      // Add the identifier to the local declarations.
      // This way, subsequent lookups in this scope will be faster.
      if (!GTA_HASHX_SET(scope->local_declarations, identifier->hash, GTA_TYPEX_MAKE_P(identifier))) {
        return gta_ast_node_parse_error_out_of_memory;
      }
      return 0;
    }

    // Cleanup the current mangled name and continue the search.
    gcu_free(mangled_name);
    current_scope = current_scope->parent_scope;
  }

  // If we made it here, then the identifier is a local variable.
  identifier->mangled_name = identifier->identifier;
  identifier->mangled_name_hash = identifier->hash;
  identifier->type = GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL;

  // Add the identifier to the local declarations.
  // This way, subsequent lookups in this scope will be faster.
  if (!GTA_HASHX_SET(scope->local_declarations, identifier->hash, GTA_TYPEX_MAKE_P(identifier))) {
    return gta_ast_node_parse_error_out_of_memory;
  }
  // Add the position of the identifier to the list of local variables.
  if (!GTA_HASHX_SET(scope->local_positions, identifier->hash, GTA_TYPEX_MAKE_UI(scope->local_positions->entries))) {
    return gta_ast_node_parse_error_out_of_memory;
  }
  return 0;
}


void gta_ast_node_identifier_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}


bool gta_ast_node_identifier_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;
  if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY) {
    GTA_HashX_Value val = GTA_HASHX_GET(context->program->scope->global_positions, identifier->mangled_name_hash);
    if (!val.exists) {
      printf("Error: Identifier %s not found in global positions.\n", identifier->mangled_name);
      return false;
    }
    return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_PEEK_BP))
      && GTA_VECTORX_APPEND(context->program->bytecode, val.value);
  }
  return false;
}
