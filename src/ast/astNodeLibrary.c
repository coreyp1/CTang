
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeLibrary.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_library_vtable = {
  .name = "Library",
  .compile_to_bytecode = gta_ast_node_library_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_library_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_library_destroy,
  .print = gta_ast_node_library_print,
  .simplify = gta_ast_node_library_simplify,
  .analyze = 0,
  .walk = gta_ast_node_library_walk,
};


GTA_Ast_Node_Library * gta_ast_node_library_create(const char * identifier, GTA_PARSER_LTYPE location) {
  assert(identifier);

  GTA_Ast_Node_Library * self = gcu_malloc(sizeof(GTA_Ast_Node_Library));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Library) {
    .base = {
      .vtable = &gta_ast_node_library_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .identifier = identifier,
    .hash = GTA_STRING_HASH(identifier, strlen(identifier)),
  };
  return self;
}


void gta_ast_node_library_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_LIBRARY(self));
  GTA_Ast_Node_Library * library = (GTA_Ast_Node_Library *) self;

  gcu_free((void *)library->identifier);
  gcu_free(self);
}


void gta_ast_node_library_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_LIBRARY(self));
  GTA_Ast_Node_Library * library = (GTA_Ast_Node_Library *) self;

  assert(indent);
  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s: %s\n", indent, self->vtable->name, library->identifier);
}


GTA_Ast_Node * gta_ast_node_library_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_library_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  callback(self, data, return_value);
}


static GTA_Computed_Value * GTA_CALL __load_library(GTA_Execution_Context * context, GTA_UInteger hash) {
  assert(context);
  assert(context->library);

  GTA_Library_Callback func = gta_library_get_from_context(context, hash);
  if (!func) {
    return gta_computed_value_null;
  }
  GTA_Computed_Value * library_value = func(context);
  if (!library_value) {
    return gta_computed_value_error_out_of_memory;
  }
  if (!library_value->is_singleton && library_value->is_temporary) {
    // This is an assignment, so make sure that it is not temporary.
    library_value->is_temporary = false;
  }
  return library_value;
}


bool gta_ast_node_library_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_LIBRARY(self));
  GTA_Ast_Node_Library * library = (GTA_Ast_Node_Library *)self;

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_LOAD_LIBRARY))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(library->hash));
}


bool gta_ast_node_library_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_LIBRARY(self));
  GTA_Ast_Node_Library * library = (GTA_Ast_Node_Library *)self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  // Load the library.
  // TODO: JIT the __load_library function to avoid the extra function call.
  return true
  // __load_library(boolean->value, library->hash):
  //   mov rdi, r15
  //   mov rsi, library->hash
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RDI, GTA_REG_R15)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RSI, library->hash)
    && gta_binary_call__x86_64(v, (uint64_t)__load_library);
}
