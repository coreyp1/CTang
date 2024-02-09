
#include <stdio.h>
#include <cutil/memory.h>
#include "tang/astNode.h"
#include "tang/macros.h"
#include "tang/bytecode.h"
#include "tang/bytecodeCompilerContext.h"
#include "tang/program.h"

GTA_Ast_Node_VTable gta_ast_node_null_vtable = {
  .name = "Null",
  .compile_to_bytecode = gta_ast_node_null_compile_to_bytecode,
  .destroy = gta_ast_node_null_destroy,
  .print = gta_ast_node_null_print,
  .simplify = gta_ast_node_null_simplify,
  .walk = gta_ast_node_null_walk,
};

GTA_Ast_Node *gta_ast_node_create(GTA_PARSER_LTYPE location) {
  GTA_Ast_Node * self = gcu_malloc(sizeof(GTA_Ast_Node));
  *self = (GTA_Ast_Node) {
    .vtable = &gta_ast_node_null_vtable,
    .location = location,
    .possible_type = GTA_AST_POSSIBLE_TYPE_NULL,
  };
  return self;
}

void gta_ast_node_destroy(GTA_Ast_Node * self) {
  self->vtable->destroy
    ? self->vtable->destroy(self)
    : gta_ast_node_null_destroy(self);
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

bool gta_ast_node_null_compile_to_bytecode(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_Bytecode_Compiler_Context * context) {
  return gcu_vector64_append(context->program->bytecode, GCU_TYPE64_UI64(GTA_BYTECODE_NULL));
}

void gta_ast_node_null_destroy(GTA_Ast_Node * self) {
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
