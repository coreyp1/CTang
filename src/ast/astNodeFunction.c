
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/hash.h>
#include <tang/ast/astNodeFunction.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeParseError.h>
#include <tang/computedValue/computedValueFunction.h>
#include <tang/program/variable.h>

GTA_Ast_Node_VTable gta_ast_node_function_vtable = {
  .name = "Function",
  .compile_to_bytecode = gta_ast_node_function_compile_to_bytecode,
  .compile_to_binary__x86_64 = 0,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_function_destroy,
  .print = gta_ast_node_function_print,
  .simplify = gta_ast_node_function_simplify,
  .analyze = gta_ast_node_function_analyze,
  .walk = gta_ast_node_function_walk,
};


GTA_Ast_Node_Function * gta_ast_node_function_create(const char * identifier, GTA_VectorX * parameters, GTA_Ast_Node * block, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Function * self = gcu_malloc(sizeof(GTA_Ast_Node_Function));
  if (!self) {
    goto SELF_CREATION_FAILURE;
  }

  // Create a runtime function object.
  GTA_Computed_Value_Function * runtime_function = gta_computed_value_function_create(parameters->count, 0, 0);
  if (!runtime_function) {
    goto RUNTIME_FUNCTION_CREATION_FAILURE;
  }

  *self = (GTA_Ast_Node_Function) {
    .base = {
      .vtable = &gta_ast_node_function_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .identifier = identifier,
    .hash = GTA_STRING_HASH(identifier, strlen(identifier)),
    .mangled_name = 0,
    .mangled_name_hash = 0,
    .parameters = parameters,
    .block = block,
    .scope = 0,
    .runtime_function = runtime_function,
  };
  return self;

RUNTIME_FUNCTION_CREATION_FAILURE:
  gcu_free(self);
SELF_CREATION_FAILURE:
  return 0;
}


void gta_ast_node_function_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *) self;
  GTA_VECTORX_DESTROY(function->parameters);
  gta_ast_node_destroy(function->block);
  if (function->mangled_name && (function->mangled_name != function->identifier)) {
    gcu_free((void *)function->mangled_name);
  }
  gcu_free((void *)function->identifier);
  gta_computed_value_destroy((GTA_Computed_Value *)function->runtime_function);
  gcu_free(self);
}


void gta_ast_node_function_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);

  char * small_indent = gcu_malloc(strlen(indent) + 3);
  if (!small_indent) {
    gcu_free(new_indent);
    return;
  }
  memcpy(small_indent, indent, indent_len + 1);
  memcpy(small_indent + indent_len, "  ", 3);

  printf("%s%s: %s\n", indent, self->vtable->name, function->identifier);
  printf("%s  Parameters:\n", indent);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function->parameters); i++) {
    printf("%s%s\n", new_indent, ((GTA_Ast_Node_Identifier *)GTA_TYPEX_P(function->parameters->data[i]))->identifier);
  }
  function->block->vtable->print(function->block, small_indent);
  gcu_free(new_indent);
  gcu_free(small_indent);
}


GTA_Ast_Node * gta_ast_node_function_simplify(GTA_Ast_Node * self, GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *) self;
  // A function is a top-level node, so we need to pass it a new variable map.
  GTA_Ast_Simplify_Variable_Map * new_variable_map = gcu_hash64_create(0);
  if (!new_variable_map) {
    return 0;
  }

  // Note: We don't need to simplify the parameters because they can only be
  // identifiers.

  GTA_Ast_Node * simplified_block = gta_ast_node_simplify(function->block, new_variable_map);
  if (simplified_block) {
    gta_ast_node_destroy(function->block);
    function->block = simplified_block;
  }

  gcu_hash64_destroy(new_variable_map);
  return 0;
}


GTA_Ast_Node * gta_ast_node_function_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * parent_scope) {
  GTA_Variable_Scope * outermost_scope = parent_scope;
  while (outermost_scope->parent_scope) {
    outermost_scope = outermost_scope->parent_scope;
  }
  GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *) self;
  GTA_Ast_Node * error = 0;

  // Step 1: Compute a mangled name and mangled hash for the function.
  // To avoid name collisions with functions in other scopes, we must mangle
  // the function name using the current (parent) scope's namespace.
  size_t namespace_length = strlen(parent_scope->name);
  size_t identifier_length = strlen(function->identifier);
  size_t mangled_length = identifier_length + namespace_length + 2;
  char * mangled_name = gcu_calloc(1, mangled_length);
  if (!mangled_name) {
    error = gta_ast_node_parse_error_out_of_memory;
    goto MANGLED_NAME_CREATION_FAILURE;
  }
  snprintf(mangled_name, mangled_length, "%s/%s", parent_scope->name, function->identifier);
  function->mangled_name_hash = GTA_STRING_HASH(mangled_name, mangled_length);

  // Step 2: Create a new scope for the function.
  // If the scope creation succeeds, it will adopt the mangled name string.
  function->scope = gta_variable_scope_create(mangled_name, self, parent_scope);
  if (!function->scope) {
    // Note: For all other failures, the mangled_name has already been adopted
    // and will be properly freed by the scope when it is destroyed.
    // However, if the scope creation fails, we need to free the mangled_name
    // ourselves.
    gcu_free(mangled_name);
    error = gta_ast_node_parse_error_out_of_memory;
    goto SCOPE_CREATION_FAILURE;
  }

  // Step 3: "Register" the function in the outermost scope using the mangled
  // name hash. If a scope already exists, then error out.
  GTA_HashX_Value existing_scope = GTA_HASHX_GET(outermost_scope->function_scopes, function->mangled_name_hash);
  if (existing_scope.exists) {
    error = gta_ast_node_parse_error_function_redeclared;
  }
  if (!GTA_HASHX_SET(outermost_scope->function_scopes, function->mangled_name_hash, GTA_TYPEX_MAKE_P(function->scope))) {
    error = gta_ast_node_parse_error_out_of_memory;
  }
  if (error) {
    gta_variable_scope_destroy(function->scope);
    function->scope = 0;
    goto FUNCTION_REDECLARATION_ERROR;
  }

  GTA_HashX_Value existing_global_identified_variables = GTA_HASHX_GET(outermost_scope->identified_variables, function->mangled_name_hash);
  if (existing_global_identified_variables.exists) {
    error = gta_ast_node_parse_error_identifier_redeclared;
    goto FUNCTION_REDECLARATION_ERROR;
  }
  if (!GTA_HASHX_SET(outermost_scope->identified_variables, function->mangled_name_hash, GTA_TYPEX_MAKE_P(function))) {
    error = gta_ast_node_parse_error_out_of_memory;
    goto FUNCTION_REDECLARATION_ERROR;
  }

  // Step 4: "Register" the function in the current scope using the un-mangled
  // name.  This allows other identifiers in this scope to resolve properly.
  // If the identifier already exists, then error out.
  GTA_HashX_Value existing_scope_identified_variables = GTA_HASHX_GET(parent_scope->identified_variables, function->hash);
  if (existing_scope_identified_variables.exists) {
    error = gta_ast_node_parse_error_identifier_redeclared;
    goto FUNCTION_REDECLARATION_ERROR;
  }
  if (!GTA_HASHX_SET(parent_scope->identified_variables, function->hash, GTA_TYPEX_MAKE_P(function))) {
    error = gta_ast_node_parse_error_out_of_memory;
    goto FUNCTION_REDECLARATION_ERROR;
  }

  // Step 6: Add the parameters to the function scope.
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function->parameters); ++i) {
    GTA_Ast_Node_Identifier * parameter = (GTA_Ast_Node_Identifier *)GTA_TYPEX_P(function->parameters->data[i]);
    if ((error = gta_ast_node_analyze((GTA_Ast_Node *)parameter, program, function->scope))) {
      return error;
    }
  }

  // Step 7: Reserve a slot in the global_positions map.
  if (!GTA_HASHX_SET(outermost_scope->global_positions, function->mangled_name_hash, GTA_TYPEX_MAKE_UI(outermost_scope->global_positions->entries))) {
    error = gta_ast_node_parse_error_out_of_memory;
    goto GLOBAL_REGISTRATION_ERROR;
  }

  // Step 8: Analyze the block.
  return gta_ast_node_analyze(function->block, program, function->scope);

GLOBAL_REGISTRATION_ERROR:
FUNCTION_REDECLARATION_ERROR:
SCOPE_CREATION_FAILURE:
MANGLED_NAME_CREATION_FAILURE:
  return error;
}


void gta_ast_node_function_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *) self;

  for (size_t i = 0; i < GTA_VECTORX_COUNT(function->parameters); ++i) {
    gta_ast_node_walk((GTA_Ast_Node *)GTA_TYPEX_P(function->parameters->data[i]), callback, data, return_value);
  }

  gta_ast_node_walk(function->block, callback, data, return_value);
}


bool gta_ast_node_function_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *) self;
  GTA_VectorX * b = context->program->bytecode;
  GTA_VectorX * o = context->bytecode_offsets;

  bool error_free = true;

  // Jump labels.
  GTA_Integer function_end;

  return error_free
  // Create jump labels.
    && ((function_end = gta_compiler_context_get_label(context)) >= 0)
  // Jump over the function body.
  //   JMP function_end
    && GTA_BYTECODE_APPEND(o, b->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(context, function_end, b->count - 1)
  // Record the function's bytecode offset.
    && (function->runtime_function->pointer = GTA_VECTORX_COUNT(b))
  // Compile the function body.
    && gta_ast_node_compile_to_bytecode(function->block, context)
  // Implicit return.
    && GTA_BYTECODE_APPEND(o, b->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_RETURN))
  // End of function.
  //   function_end:
    && gta_compiler_context_set_label(context, function_end, context->program->bytecode->count)
  ;
}
