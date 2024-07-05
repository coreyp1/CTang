
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeMap.h>
#include <tang/ast/astNodeString.h>
#include <tang/unicodeString.h>

GTA_Ast_Node_VTable gta_ast_node_map_vtable = {
  .name = "Map",
  .compile_to_bytecode = gta_ast_node_map_compile_to_bytecode,
  .compile_to_binary__x86_64 = 0,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_map_destroy,
  .print = gta_ast_node_map_print,
  .simplify = gta_ast_node_map_simplify,
  .analyze = gta_ast_node_map_analyze,
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
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;
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
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;
  printf("%s%s\n", indent, self->vtable->name);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    printf("%s  Key:\n", indent);
    gta_ast_node_print(pair->key, new_indent);
    printf("%s  Value:\n", indent);
    gta_ast_node_print(pair->value, new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_map_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    GTA_Ast_Node * simplified = gta_ast_node_simplify(pair->value, variable_map);
    if (simplified) {
      gta_ast_node_destroy(pair->value);
      pair->value = simplified;
    }
  }
  return 0;
}


void gta_ast_node_map_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    callback(pair->key, data, return_value);
    callback(pair->value, data, return_value);
  }
}


GTA_Ast_Node * gta_ast_node_map_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(map->pairs); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    GTA_Ast_Node * error = gta_ast_node_analyze(pair->key, program, scope);
    if (error) {
      return error;
    }
    error = gta_ast_node_analyze(pair->value, program, scope);
    if (error) {
      return error;
    }
  }
  return NULL;
}


bool gta_ast_node_map_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_Map * map = (GTA_Ast_Node_Map *)self;
  bool error_free = true;
  for (size_t i = 0; error_free && (i < map->pairs->count); ++i) {
    GTA_Ast_Node_Map_Pair * pair = (GTA_Ast_Node_Map_Pair *)GTA_TYPEX_P(map->pairs->data[i]);
    error_free &= true
      && gta_ast_node_compile_to_bytecode(pair->key, context)
      && gta_ast_node_compile_to_bytecode(pair->value, context);
  }
  return error_free
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_MAP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(map->pairs->count));
}
