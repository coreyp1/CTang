
#include <stdio.h>
#include <string.h>
#include <cutil/hash.h>
#include <cutil/memory.h>
#include <tang/ast/astNode.h>
#include <tang/program/variable.h>

/**
 * Helper function that will recursively clean up all function scopes in a
 * hash.
 * @param hash The hash to clean up.
 */
static void __gta_variable_scope_function_cleanup(GTA_HashX * hash) {
  GTA_HashX_Iterator iterator = GTA_HASHX_ITERATOR_GET(hash);
  while (iterator.exists) {
    GTA_Variable_Scope * scope = (GTA_Variable_Scope *)GTA_TYPEX_P(iterator.value);
    gta_variable_scope_destroy(scope);
    iterator = GTA_HASHX_ITERATOR_NEXT(iterator);
  }
}


/**
 * Helper function that will clean up all name hashes in a vector.
 * @param vector The vector to clean up.
 */
static void __gta_variable_scope_name_hashes_cleanup(GTA_VectorX * vector) {
  for (size_t i = 0; i < vector->count; i++) {
    gcu_free(GTA_TYPEX_P(vector->data[i]));
  }
}


GTA_Variable_Scope * gta_variable_scope_create(char * name, GTA_Ast_Node * ast_node, GTA_Variable_Scope * parent_scope) {
  GTA_Variable_Scope * scope = (GTA_Variable_Scope *)gcu_malloc(sizeof(GTA_Variable_Scope));
  if (!scope) {
    return NULL;
  }
  *scope = (GTA_Variable_Scope) {
    .name = name,
    .name_hash = GTA_STRING_HASH(name, strlen(name)),
    .parent_scope = parent_scope,
    .ast_node = ast_node,
    .library_declarations = GTA_HASHX_CREATE(32),
    .identified_variables = GTA_HASHX_CREATE(32),
    .global_positions = GTA_HASHX_CREATE(32),
    .local_positions = GTA_HASHX_CREATE(32),
    .function_scopes = GTA_HASHX_CREATE(32),
    .allocated_mangled_names = GTA_VECTORX_CREATE(32),
  };
  if (!scope->library_declarations
    || !scope->identified_variables
    || !scope->global_positions
    || !scope->local_positions
    || !scope->function_scopes
    || !scope->allocated_mangled_names) {
    gta_variable_scope_destroy(scope);
    return NULL;
  }
  scope->function_scopes->cleanup = __gta_variable_scope_function_cleanup;
  scope->allocated_mangled_names->cleanup = __gta_variable_scope_name_hashes_cleanup;
  return scope;
}


void gta_variable_scope_destroy(GTA_Variable_Scope * scope) {
  if (!scope) {
    return;
  }
  if (scope->name) {
    gcu_free(scope->name);
  }
  if (scope->library_declarations) {
    GTA_HASHX_DESTROY(scope->library_declarations);
  }
  if (scope->identified_variables) {
    GTA_HASHX_DESTROY(scope->identified_variables);
  }
  if (scope->global_positions) {
    GTA_HASHX_DESTROY(scope->global_positions);
  }
  if (scope->local_positions) {
    GTA_HASHX_DESTROY(scope->local_positions);
  }
  if (scope->function_scopes) {
    GTA_HASHX_DESTROY(scope->function_scopes);
  }
  if (scope->allocated_mangled_names) {
    GTA_VECTORX_DESTROY(scope->allocated_mangled_names);
  }
  gcu_free(scope);
}


void gta_variable_scope_print(GTA_Variable_Scope * scope, const char * indent) {
  GTA_HashX_Iterator hash_iterator;
  if (!scope) {
    return;
  }
  size_t indent_length = strlen(indent);
  char * new_indent = gcu_calloc(indent_length + 7, 1);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_length + 1);
  memcpy(new_indent + indent_length, "      ", 7);
  new_indent[indent_length + 4] = '\0';

  printf("%sScope: \"%s\"\n", indent, scope->name);
  printf("%s  Library Declarations:\n", indent);
  hash_iterator = GTA_HASHX_ITERATOR_GET(scope->library_declarations);
  while (hash_iterator.exists) {
    GTA_Ast_Node * node = (GTA_Ast_Node *)GTA_TYPEX_P(hash_iterator.value);
    gta_ast_node_print(node, new_indent);
    hash_iterator = GTA_HASHX_ITERATOR_NEXT(hash_iterator);
  }

  printf("%s  Identified Variables:\n", indent);
  hash_iterator = GTA_HASHX_ITERATOR_GET(scope->identified_variables);
  while (hash_iterator.exists) {
    GTA_Ast_Node * node = (GTA_Ast_Node *)GTA_TYPEX_P(hash_iterator.value);
    gta_ast_node_print(node, new_indent);
    hash_iterator = GTA_HASHX_ITERATOR_NEXT(hash_iterator);
  }

  printf("%s  Global Positions:\n", indent);
  new_indent[indent_length + 4] = ' ';
  hash_iterator = GTA_HASHX_ITERATOR_GET(scope->global_positions);
  while (hash_iterator.exists) {
    GTA_UInteger hash = hash_iterator.hash;
    GTA_Integer position = GTA_TYPEX_I(hash_iterator.value);
    GTA_HashX_Value val = GTA_HASHX_GET(scope->identified_variables, hash);
    if (val.exists) {
      GTA_Ast_Node * node = (GTA_Ast_Node *)GTA_TYPEX_P(val.value);
      printf("%s    %lu\n", indent, position);
      gta_ast_node_print(node, new_indent);
    }
    else {
      printf("%s    %lu\n", indent, position);
      printf("%s      (Missing declaration)\n", new_indent);
    }
    hash_iterator = GTA_HASHX_ITERATOR_NEXT(hash_iterator);
  }
  new_indent[indent_length + 4] = '\0';

  printf("%s  Local Positions:\n", indent);
  new_indent[indent_length + 4] = ' ';
  hash_iterator = GTA_HASHX_ITERATOR_GET(scope->local_positions);
  while (hash_iterator.exists) {
    GTA_UInteger hash = hash_iterator.hash;
    GTA_Integer position = GTA_TYPEX_I(hash_iterator.value);
    GTA_HashX_Value val = GTA_HASHX_GET(scope->identified_variables, hash);
    if (val.exists) {
      GTA_Ast_Node * node = (GTA_Ast_Node *)GTA_TYPEX_P(val.value);
      printf("%s    %lu\n", indent, position);
      gta_ast_node_print(node, new_indent);
    }
    hash_iterator = GTA_HASHX_ITERATOR_NEXT(hash_iterator);
  }
  new_indent[indent_length + 4] = '\0';

  printf("%s  Function Scopes:\n", indent);
  hash_iterator = GTA_HASHX_ITERATOR_GET(scope->function_scopes);
  while (hash_iterator.exists) {
    GTA_Variable_Scope * function_scope = (GTA_Variable_Scope *)GTA_TYPEX_P(hash_iterator.value);
    gta_variable_scope_print(function_scope, new_indent);
    hash_iterator = GTA_HASHX_ITERATOR_NEXT(hash_iterator);
  }

  printf("%s  Name Hashes:\n", indent);
  for (size_t i = 0; i < scope->allocated_mangled_names->count; i++) {
    printf("%s    %s\n", new_indent, (char *)GTA_TYPEX_P(scope->allocated_mangled_names->data[i]));
  }
  gcu_free(new_indent);
}
