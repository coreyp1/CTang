
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/hash.h>
#include <cutil/string.h>
#include "tang/astNodeIdentifier.h"
#include "tang/astNodeInteger.h"
#include "tang/astNodeFloat.h"
#include "tang/astNodeBoolean.h"
#include "tang/astNodeString.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_identifier_vtable = {
  .name = "Identifier",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_identifier_destroy,
  .print = gta_ast_node_identifier_print,
  .simplify = gta_ast_node_identifier_simplify,
  .walk = gta_ast_node_identifier_walk,
};

GTA_Ast_Node_Identifier * gta_ast_node_identifier_create(const char * identifier, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Identifier * self = gcu_malloc(sizeof(GTA_Ast_Node_Identifier));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_identifier_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->identifier = identifier;
  self->hash = gcu_string_hash_64(identifier, strlen(identifier));
  return self;
}

void gta_ast_node_identifier_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;
  gcu_free((void *)identifier->identifier);
  gcu_free(self);
}

void gta_ast_node_identifier_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) self;
  printf("%s%s: %s\n", indent, self->vtable->name, identifier->identifier);
}

GTA_Ast_Node * gta_ast_node_identifier_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  // If the identifier is in the variable map, and the value is a primitive,
  // return the primitive value.
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

void gta_ast_node_identifier_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}
