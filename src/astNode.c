
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/type.h>
#include "tang/astNode.h"
#include "tang/macros.h"
#include "tang/bytecode.h"
#include "tang/bytecodeCompilerContext.h"
#include "tang/program.h"
#include "tang/computedValue.h"
#include <tang/macros.h>

GTA_Ast_Node_VTable gta_ast_node_null_vtable = {
  .name = "Null",
  .compile_to_bytecode = gta_ast_node_null_compile_to_bytecode,
  .destroy = gta_ast_node_null_destroy,
  .print = gta_ast_node_null_print,
  .simplify = gta_ast_node_null_simplify,
  .walk = gta_ast_node_null_walk,
};

GTA_Ast_Node * GTA_CALL gta_ast_node_create(GTA_PARSER_LTYPE location) {
  GTA_Ast_Node * self = gcu_malloc(sizeof(GTA_Ast_Node));
  *self = (GTA_Ast_Node) {
    .vtable = &gta_ast_node_null_vtable,
    .location = location,
    .possible_type = GTA_AST_POSSIBLE_TYPE_NULL,
  };
  return self;
}

void GTA_CALL gta_ast_node_destroy(GTA_Ast_Node * self) {
  self->vtable->destroy
    ? self->vtable->destroy(self)
    : gta_ast_node_null_destroy(self);
}

bool gta_ast_node_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  return self->vtable->compile_to_binary
    ? self->vtable->compile_to_binary(self, context)
    : gta_ast_node_null_compile_to_binary(self, context);
}

bool gta_ast_node_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  return self->vtable->compile_to_bytecode
    ? self->vtable->compile_to_bytecode(self, context)
    : gta_ast_node_null_compile_to_bytecode(self, context);
}

void gta_ast_node_print(GTA_Ast_Node * self, const char * indent) {
  self->vtable->print
    ? self->vtable->print(self, indent)
    : gta_ast_node_null_print(self, indent);
}

GTA_Ast_Node * gta_ast_node_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  return self->vtable->simplify
    ? self->vtable->simplify(self, variable_map)
    : gta_ast_node_null_simplify(self, variable_map);
}

void gta_ast_node_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  self->vtable->walk
    ? self->vtable->walk(self, callback, data, return_value)
    : gta_ast_node_null_walk(self, callback, data, return_value);
}

bool gta_ast_node_null_compile_to_binary(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Binary_Compiler_Context * context)) {
  GCU_Vector8 * v = context->binary_vector;
  if (!gcu_vector8_reserve(v, v->count + 12)) {
    return false;
  }
#if GTA_X86_64
  // 64-bit x86
  // Assembly to call gta_computed_value_create():
  //   mov rax, gta_computed_value_create
  GTA_BINARY_WRITE2(v, 0x48, 0xB8);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  //   call rax
  GTA_BINARY_WRITE2(v, 0xFF, 0xD0);

  GTA_UInteger fp = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_create);
  memcpy(&v->data[v->count - 10], &fp, 8);
  return true;
#endif
  return false;
}

bool gta_ast_node_null_compile_to_bytecode(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_Bytecode_Compiler_Context * context) {
  return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count) && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_NULL));
}

void GTA_CALL gta_ast_node_null_destroy(GTA_Ast_Node * self) {
  gcu_free(self);
}

void gta_ast_node_null_print(GTA_Ast_Node * self, const char * indent) {
  printf("%s%s\n", indent, self->vtable->name);
}

GTA_Ast_Node * gta_ast_node_null_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}

void gta_ast_node_null_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}

void gta_ast_simplify_variable_map_invalidate(GTA_Ast_Simplify_Variable_Map * variable_map) {
  // Iterate through the map and remove all of the entries.
  GCU_Hash64_Iterator iterator = gcu_hash64_iterator_get(variable_map);
  while (iterator.exists) {
    gcu_hash64_remove(variable_map, iterator.hash);
    iterator = gcu_hash64_iterator_next(iterator);
  }
}

void gta_ast_simplify_variable_map_synchronize(GTA_Ast_Simplify_Variable_Map * target, GTA_Ast_Simplify_Variable_Map * source) {
  // Iterate through the target map and remove any entries that mismatch the
  // source map.
  GCU_Hash64_Iterator target_iterator = gcu_hash64_iterator_get(target);
  while (target_iterator.exists) {
    GCU_Hash64_Value source_value = gcu_hash64_get(source, target_iterator.hash);
    if (source_value.exists) {
      if (target_iterator.value.p != source_value.value.p) {
        gcu_hash64_remove(target, target_iterator.hash);
      }
    }
    else {
      gcu_hash64_remove(target, target_iterator.hash);
    }
    target_iterator = gcu_hash64_iterator_next(target_iterator);
  }
}
