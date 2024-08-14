
#include <assert.h>
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
#include <tang/ast/astNodeGlobal.h>
#include <tang/ast/astNodeParseError.h>
#include <tang/ast/astNodeString.h>
#include <tang/ast/astNodeUse.h>
#include <tang/program/binary.h>
#include <tang/program/variable.h>

GTA_Ast_Node_VTable gta_ast_node_identifier_vtable = {
  .name = "Identifier",
  .compile_to_binary__x86_64 = gta_ast_node_identifier_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .compile_to_bytecode = gta_ast_node_identifier_compile_to_bytecode,
  .destroy = gta_ast_node_identifier_destroy,
  .print = gta_ast_node_identifier_print,
  .simplify = gta_ast_node_identifier_simplify,
  .analyze = gta_ast_node_identifier_analyze,
  .walk = gta_ast_node_identifier_walk,
};


GTA_Ast_Node_Identifier * gta_ast_node_identifier_create(const char * identifier, GTA_PARSER_LTYPE location) {
  assert(identifier);

  GTA_Ast_Node_Identifier * self = gcu_malloc(sizeof(GTA_Ast_Node_Identifier));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Identifier) {
    .base = {
      .vtable = &gta_ast_node_identifier_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .identifier = identifier,
    .hash = GTA_STRING_HASH(identifier, strlen(identifier)),
    .mangled_name = 0,
    .mangled_name_hash = 0,
    .type = GTA_AST_NODE_IDENTIFIER_TYPE_NONE,
    .scope = 0,
  };
  return self;
}


void gta_ast_node_identifier_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_IDENTIFIER(self));
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;

  assert(identifier->identifier);
  gcu_free((void *)identifier->identifier);
  gcu_free(self);
}


void gta_ast_node_identifier_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_IDENTIFIER(self));
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;

  assert(indent);
  assert(self->vtable);
  assert(self->vtable->name);
  assert(identifier->identifier);
  printf("%s%s: %s\n", indent, self->vtable->name, identifier->identifier);
}


GTA_Ast_Node * gta_ast_node_identifier_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  // If the identifier is in the variable map, and the value is a primitive,
  // return the primitive value.
  assert(self);
  assert(GTA_AST_IS_IDENTIFIER(self));
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
  assert(self);
  assert(GTA_AST_IS_IDENTIFIER(self));
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;

  assert(scope);
  GTA_Variable_Scope * outermost_scope = scope;
  while (outermost_scope->parent_scope) {
    outermost_scope = outermost_scope->parent_scope;
  }
  identifier->scope = scope;

  // If an identifier in this scope has already been recognized, then propagate
  // the type and mangled name.
  assert(scope->identified_variables);
  assert(identifier->hash);
  GTA_HashX_Value val = GTA_HASHX_GET(scope->identified_variables, identifier->hash);
  if (val.exists) {
    if (GTA_AST_IS_IDENTIFIER(val.value.p)) {
      GTA_Ast_Node_Identifier * existing_identifier = (GTA_Ast_Node_Identifier *)val.value.p;
      identifier->mangled_name = existing_identifier->mangled_name;
      identifier->mangled_name_hash = existing_identifier->mangled_name_hash;
      identifier->type = existing_identifier->type;
      return 0;
    }
    if (GTA_AST_IS_FUNCTION(val.value.p)) {
      GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *)val.value.p;
      identifier->mangled_name = function->mangled_name;
      identifier->mangled_name_hash = function->mangled_name_hash;
      identifier->type = GTA_AST_NODE_IDENTIFIER_TYPE_FUNCTION;
      return 0;
    }
    if (GTA_AST_IS_GLOBAL(val.value.p)) {
      GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *)val.value.p;
      identifier->type = GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL;
      identifier->mangled_name = ((GTA_Ast_Node_Identifier *)global->identifier)->mangled_name;
      identifier->mangled_name_hash = ((GTA_Ast_Node_Identifier *)global->identifier)->mangled_name_hash;
      return 0;
    }
    // We should never get here.
    fprintf(stderr, "Error: Identifier type not recognized.\n");
    assert(false);
  }

  // Is the identifier a library variable?
  GTA_HashX_Value outermost_library = GTA_HASHX_GET(outermost_scope->library_declarations, identifier->hash);
  if (outermost_library.exists) {
    // The identifier is a library variable.  Has it already been seen in the
    // outermost scope?
    GTA_HashX_Value outermost_library_identifier = GTA_HASHX_GET(outermost_scope->identified_variables, identifier->hash);
    if (outermost_library_identifier.exists) {
      // The identifier has already been seen in the outermost scope.
      // Propagate the type and mangled name.
      GTA_Ast_Node_Identifier * existing_identifier = (GTA_Ast_Node_Identifier *)outermost_library_identifier.value.p;
      identifier->mangled_name = existing_identifier->mangled_name;
      identifier->mangled_name_hash = existing_identifier->mangled_name_hash;
      identifier->type = existing_identifier->type;
    }
    else {
      // The identifier has not been seen in the outermost scope.  Add it to
      // the outermost scope so that, when globals are loaded, the identifier
      // hash will be recognized.
      identifier->mangled_name = identifier->identifier;
      identifier->mangled_name_hash = identifier->hash;
      identifier->type = GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY;
      if (!GTA_HASHX_SET(outermost_scope->identified_variables, identifier->hash, GTA_TYPEX_MAKE_P(identifier))
        || !GTA_HASHX_SET(outermost_scope->variable_positions, identifier->hash, GTA_TYPEX_MAKE_UI(outermost_scope->variable_positions->entries))) {
        return gta_ast_node_parse_error_out_of_memory;
      }
    }
    if ((scope != outermost_scope)
      && !GTA_HASHX_SET(scope->identified_variables, identifier->hash, GTA_TYPEX_MAKE_P(identifier))) {
      return gta_ast_node_parse_error_out_of_memory;
    }
    return 0;
  }

  // Look for a function name in each scope, from current to outermost.
  GTA_Variable_Scope * current_scope = scope;
  assert(identifier->identifier);
  size_t identifier_length = strlen(identifier->identifier);
  while (current_scope) {
    // Compute the possible mangled name based on the current_scope.
    assert(current_scope->name);
    size_t namespace_length = strlen(current_scope->name);
    size_t mangled_length = identifier_length + namespace_length + 2;
    char * mangled_name = gcu_calloc(1, mangled_length);
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
      if (!GTA_HASHX_SET(scope->identified_variables, identifier->hash, GTA_TYPEX_MAKE_P(identifier))) {
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
  if (!GTA_HASHX_SET(scope->identified_variables, identifier->hash, GTA_TYPEX_MAKE_P(identifier))) {
    return gta_ast_node_parse_error_out_of_memory;
  }
  // Add the position of the identifier to the list of local variables.
  if (!GTA_HASHX_SET(scope->variable_positions, identifier->hash, GTA_TYPEX_MAKE_UI(scope->variable_positions->entries))) {
    return gta_ast_node_parse_error_out_of_memory;
  }
  return 0;
}


void gta_ast_node_identifier_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}


bool gta_ast_node_identifier_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_IDENTIFIER(self));
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;

  assert(context);
  if ((identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY)
    || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL)
    || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_FUNCTION)) {
    // Find the identifier's position in the global positions.
    assert(context->program);
    assert(context->program->scope);
    const char * name = (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_FUNCTION)
      ? identifier->mangled_name
      : identifier->identifier;
    GTA_Integer name_hash = (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_FUNCTION)
      ? identifier->mangled_name_hash
      : identifier->hash;
    GTA_HashX_Value val = GTA_HASHX_GET(context->program->scope->variable_positions, name_hash);
    if (!val.exists) {
      printf("Error: Identifier %s not found in global positions.\n", name);
      return false;
    }

    // Reminder: r13 is the global pointer, but it points to the end of the
    // global variables.  We need to move the pointer back to the beginning of
    // the memory address for whichever variable we're trying to access.
    int32_t index = ((int32_t)GTA_TYPEX_UI(val.value) + 1) * -8;

    // Copy the value from the global position (GTA_TYPEX_UI(val.value)) to RAX.
    //   mov rax, [r13 + index]
    return true
      && gta_mov_reg_ind__x86_64(context->binary_vector, GTA_REG_RAX, GTA_REG_R13, GTA_REG_NONE, 0, index);
  }

  if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL) {
    // Find the identifier's position in the local positions.
    GTA_HashX_Value val = GTA_HASHX_GET(identifier->scope->variable_positions, identifier->mangled_name_hash);
    if (!val.exists) {
      printf("Error: Identifier %s not found in local positions.\n", identifier->mangled_name);
      return false;
    }

    // Reminder: r12 is the local pointer, but it points to the end of the
    // local variables.  We need to move the pointer back to the beginning of
    // the memory address for whichever variable we're trying to access.
    int32_t index = ((int32_t)GTA_TYPEX_UI(val.value) + 1) * -8;

    // Copy the value from the local position (GTA_TYPEX_UI(val.value)) to RAX.
    //   mov rax, [r12 + index]
    return true
      && gta_mov_reg_ind__x86_64(context->binary_vector, GTA_REG_RAX, GTA_REG_R12, GTA_REG_NONE, 0, index);
  }

  // We should never get here.
  assert(false);
  fprintf(stderr, "Error: Identifier type not recognized.\n");
  return false;
}


bool gta_ast_node_identifier_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_IDENTIFIER(self));
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;

  assert(context);
  if ((identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY)
    || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL)
    || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_FUNCTION)) {
    assert(context->program);
    assert(context->program->scope);
    const char * name = (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_FUNCTION)
      ? identifier->mangled_name
      : identifier->identifier;
    GTA_Integer name_hash = (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_FUNCTION)
      ? identifier->mangled_name_hash
      : identifier->hash;
    GTA_HashX_Value val = GTA_HASHX_GET(context->program->scope->variable_positions, name_hash);
    if (!val.exists) {
      printf("Error: Identifier %s not found in global positions.\n", name);
      return false;
    }
    return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_PEEK_GLOBAL))
      && GTA_VECTORX_APPEND(context->program->bytecode, val.value);
  }
  if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL) {
    GTA_HashX_Value val = GTA_HASHX_GET(identifier->scope->variable_positions, identifier->mangled_name_hash);
    if (!val.exists) {
      printf("Error: Identifier %s not found in local positions.\n", identifier->mangled_name);
      return false;
    }
    assert(context->program->bytecode);
    return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_PEEK_LOCAL))
      && GTA_VECTORX_APPEND(context->program->bytecode, val.value);
  }

  // We should never get here.
  if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_NONE) {
    printf("Error: Identifier %s has not been resolved.\n", identifier->identifier);
    assert(false);
  }
  fprintf(stderr, "Error: Identifier type not recognized.\n");
  assert(false);
  return false;
}
