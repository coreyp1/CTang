
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/hash.h>
#include <cutil/string.h>
#include <tang/ast/astNodeParseError.h>
#include <tang/ast/astNodeUse.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/program/binaryCompilerContext.h>
#include <tang/program/variable.h>

GTA_Ast_Node_VTable gta_ast_node_use_vtable = {
  .name = "Use",
  .compile_to_binary = gta_ast_node_use_compile_to_binary,
  .compile_to_bytecode = gta_ast_node_use_compile_to_bytecode,
  .destroy = gta_ast_node_use_destroy,
  .print = gta_ast_node_use_print,
  .simplify = gta_ast_node_use_simplify,
  .analyze = gta_ast_node_use_analyze,
  .walk = gta_ast_node_use_walk,
};


GTA_Ast_Node_Use * gta_ast_node_use_create(const char * identifier, GTA_Ast_Node * expression, GTA_PARSER_LTYPE location) {
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
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
  gcu_free((void *)use->identifier);
  if (use->expression) {
    gta_ast_node_destroy(use->expression);
  }
  gcu_free(self);
}


void gta_ast_node_use_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
  printf("%s%s(%s):\n", indent, self->vtable->name, use->identifier);
  if (use->expression) {
    gta_ast_node_print(use->expression, new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_use_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
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
  callback(self, data, return_value);
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
  gta_ast_node_walk(use->expression, callback, data, return_value);
}


static GTA_Computed_Value * GTA_CALL __load_library(GTA_Execution_Context * context, GTA_UInteger hash) {
  GTA_HashX_Value func = GTA_HASHX_GET(context->globals, hash);
  if (!func.exists) {
    return gta_computed_value_null;
  }
  GTA_Computed_Value * library_value = (GTA_Function_Converter){.b = GTA_TYPEX_P(func.value)}.f(context);
  if (!library_value) {
    return gta_computed_value_error_out_of_memory;
  }
  if (!library_value->is_singleton && library_value->is_temporary) {
    // This is an assignment, so make sure that it is not temporary.
    library_value->is_temporary = false;
  }
  return library_value;
}


bool gta_ast_node_use_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  GCU_Vector8 * v = context->binary_vector;
  if (!gcu_vector8_reserve(v, v->count + 36)) {
    return false;
  }
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;

  // Load the library.
  // TODO: JIT the __load_library function to avoid the extra function call.
  if (!gcu_vector8_reserve(v, v->count + 25)) {
    return false;
  }
  // Prepare function call.
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFFFFFFFFFF0
  GTA_BINARY_WRITE1(v, 0x55);
  GTA_BINARY_WRITE3(v, 0x48, 0x89, 0xE5);
  GTA_BINARY_WRITE4(v, 0x48, 0x83, 0xE4, 0xF0);
  // __load_library(boolean->value, use->hash):
  // context is in r15
  //   mov rdi, r15
  GTA_BINARY_WRITE3(v, 0x4C, 0x89, 0xFF);
  //   mov rsi, use->hash
  GTA_BINARY_WRITE2(v, 0x48, 0xBE);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(&v->data[v->count - 8], &use->hash, 8);
  //   mov rax, __load_library
  GTA_BINARY_WRITE2(v, 0x48, 0xB8);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(&v->data[v->count - 8], &GTA_JIT_FUNCTION_CONVERTER(__load_library), 8);
  //   call rax
  GTA_BINARY_WRITE2(v, 0xFF, 0xD0);
  // Restore stack.
  //   mov rsp, rbp
  //   pop rbp
  GTA_BINARY_WRITE3(context->binary_vector, 0x48, 0x89, 0xEC);
  GTA_BINARY_WRITE1(context->binary_vector, 0x5D);
  return true;
}


bool gta_ast_node_use_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
  return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_LOAD_LIBRARY))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(use->hash));
}
