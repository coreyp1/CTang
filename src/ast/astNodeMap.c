
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeMap.h>

GTA_Ast_Node_VTable gta_ast_node_map_vtable = {
  .name = "Map",
  .compile_to_bytecode = 0,
  .compile_to_binary__x86_64 = 0,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_map_destroy,
  .print = gta_ast_node_map_print,
  .simplify = gta_ast_node_map_simplify,
  .analyze = 0,
  .walk = gta_ast_node_map_walk,
};


GTA_Ast_Node_Map * gta_ast_node_map_create(GTA_VectorX * pairs, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Map * self = gcu_malloc(sizeof(GTA_Ast_Node_Map));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Map) {
    .base = {
      .vtable = &gta_ast_node_map_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .pairs = pairs,
  };
  return self;
}


void gta_ast_node_map_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *) self;
  GTA_VECTORX_DESTROY(map->pairs);
  gcu_free(self);
}


void gta_ast_node_map_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 5);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *) self;
  printf("%s%s\n", indent, self->vtable->name);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    printf("%s  Key: %s:\n", indent, pair->str);
    printf("%s  Value:\n", indent);
    gta_ast_node_print(pair->node, new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_map_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    GTA_Ast_Node * simplified = gta_ast_node_simplify(pair->node, variable_map);
    if (simplified) {
      gta_ast_node_destroy(pair->node);
      pair->node = simplified;
    }
  }
  return 0;
}


void gta_ast_node_map_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    callback(pair->node, data, return_value);
  }
}
