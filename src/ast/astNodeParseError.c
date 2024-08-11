
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeParseError.h>

GTA_Ast_Node_VTable gta_ast_node_parse_error_vtable = {
  .name = "Parse Error",
  .compile_to_bytecode = 0,
  .compile_to_binary__x86_64 = 0,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_parse_error_destroy,
  .print = gta_ast_node_parse_error_print,
  .simplify = gta_ast_node_parse_error_simplify,
  .analyze = 0,
  .walk = gta_ast_node_parse_error_walk,
};


static GTA_Ast_Node_Parse_Error gta_ast_node_parse_error_out_of_memory_singleton = {
  .base = {
    .vtable = &gta_ast_node_parse_error_vtable,
    .location = {0, 0, 0},
    .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
    .is_singleton = true,
  },
  .message = "An out of memory error ocurred when attempting to create a parse error.",
};
GTA_Ast_Node * gta_ast_node_parse_error_out_of_memory = (GTA_Ast_Node *) &gta_ast_node_parse_error_out_of_memory_singleton;


static GTA_Ast_Node_Parse_Error gta_ast_node_parse_error_function_redeclared_singleton = {
  .base = {
    .vtable = &gta_ast_node_parse_error_vtable,
    .location = {0, 0, 0},
    .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
    .is_singleton = true,
  },
  .message = "A function has been redeclared.",
};
GTA_Ast_Node * gta_ast_node_parse_error_function_redeclared = (GTA_Ast_Node *) &gta_ast_node_parse_error_function_redeclared_singleton;


static GTA_Ast_Node_Parse_Error gta_ast_node_parse_error_identifier_redeclared_singleton = {
  .base = {
    .vtable = &gta_ast_node_parse_error_vtable,
    .location = {0, 0, 0},
    .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
    .is_singleton = true,
  },
  .message = "An identifier has been redeclared.",
};
GTA_Ast_Node * gta_ast_node_parse_error_identifier_redeclared = (GTA_Ast_Node *) &gta_ast_node_parse_error_identifier_redeclared_singleton;


GTA_Ast_Node_Parse_Error * gta_ast_node_parse_error_create(const char * message, GTA_PARSER_LTYPE location) {
  assert(message);

  GTA_Ast_Node_Parse_Error * self = gcu_malloc(sizeof(GTA_Ast_Node_Parse_Error));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Parse_Error) {
    .base = {
      .vtable = &gta_ast_node_parse_error_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .message = 0,
  };

  size_t messageLength = strlen(message);
  self->message = gcu_malloc(messageLength + 1);
  if (!self->message) {
    gcu_free(self);
    return 0;
  }
  strcpy(self->message, message);

  return self;
}


void gta_ast_node_parse_error_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_PARSE_ERROR(self));
  GTA_Ast_Node_Parse_Error * parseError = (GTA_Ast_Node_Parse_Error *) self;

  gcu_free(parseError->message);
  gcu_free(parseError);
}


void gta_ast_node_parse_error_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_PARSE_ERROR(self));
  GTA_Ast_Node_Parse_Error * parseError = (GTA_Ast_Node_Parse_Error *) self;

  assert(indent);
  assert(self->vtable);
  assert(self->vtable->name);
  printf("%sParse Error: %s\n", indent, parseError->message);
}


GTA_Ast_Node * gta_ast_node_parse_error_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * parseError), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_parse_error_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  callback(self, data, return_value);
}
