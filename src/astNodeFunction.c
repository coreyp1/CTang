
#include "stdio.h"
#include "string.h"
#include "cutil/memory.h"
#include "tang/astNodeFunction.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_function_vtable = {
  .name = "Function",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_function_destroy,
  .print = gta_ast_node_function_print,
  .simplify = gta_ast_node_function_simplify,
  .walk = gta_ast_node_function_walk,
};

GTA_Ast_Node_Function * gta_ast_node_function_create(const char * identifier, GCU_Vector64 * parameters, GTA_Ast_Node * block, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Function * self = gcu_malloc(sizeof(GTA_Ast_Node_Function));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_function_vtable;
  self->base.location = location;
  self->identifier = identifier;
  self->parameters = parameters;
  self->block = block;
  return self;
}

void gta_ast_node_function_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *) self;
  gcu_vector64_destroy(function->parameters);
  gta_ast_node_destroy(function->block);
  gcu_free((void *)function->identifier);
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
  for (size_t i = 0; i < gcu_vector64_count(function->parameters); i++) {
    printf("%s%s\n", new_indent, (char *)function->parameters->data[i].p);
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

  GTA_Ast_Node * simplified_block = gta_ast_node_simplify(function->block, new_variable_map);
  if (simplified_block) {
    gta_ast_node_destroy(function->block);
    function->block = simplified_block;
  }

  gcu_hash64_destroy(new_variable_map);
  return 0;
}

void gta_ast_node_function_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *) self;
  gta_ast_node_walk(function->block, callback, data, return_value);
}
