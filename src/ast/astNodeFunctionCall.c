
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeFunctionCall.h>

GTA_Ast_Node_VTable gta_ast_node_function_call_vtable = {
  .name = "FunctionCall",
  .compile_to_bytecode = gta_ast_node_function_call_compile_to_bytecode,
  .compile_to_binary__x86_64 = 0,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_function_call_destroy,
  .print = gta_ast_node_function_call_print,
  .simplify = gta_ast_node_function_call_simplify,
  .analyze = gta_ast_node_function_call_analyze,
  .walk = gta_ast_node_function_call_walk,
};


GTA_Ast_Node_Function_Call * gta_ast_node_function_call_create(GTA_Ast_Node * lhs, GTA_VectorX * arguments, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Function_Call * self = gcu_malloc(sizeof(GTA_Ast_Node_Function_Call));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Function_Call) {
    .base = {
      .vtable = &gta_ast_node_function_call_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .lhs = lhs,
    .arguments = arguments,
  };
  return self;
}


void gta_ast_node_function_call_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;
  gta_ast_node_destroy(function_call->lhs);
  GTA_VECTORX_DESTROY(function_call->arguments);
  gcu_free(self);
}


void gta_ast_node_function_call_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  printf("%s%s\n", indent, self->vtable->name);
  printf("%s  LHS:\n", indent);
  gta_ast_node_print(function_call->lhs, new_indent);
  printf("%s  Arguments:\n", indent);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); i++) {
    printf("%s  %zu:\n", indent, i);
    gta_ast_node_print((GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]), new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_function_call_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;
  GTA_Ast_Node * simplified_lhs = gta_ast_node_simplify(function_call->lhs, variable_map);
  if (simplified_lhs) {
    gta_ast_node_destroy(function_call->lhs);
    function_call->lhs = simplified_lhs;
  }
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); ++i) {
    GTA_Ast_Node * argument = (GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]);
    GTA_Ast_Node * simplified = gta_ast_node_simplify(argument, variable_map);
    if (simplified) {
      gta_ast_node_destroy(argument);
      GTA_TYPEX_P(function_call->arguments->data[i]) = simplified;
    }
  }
  return 0;
}


void gta_ast_node_function_call_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;
  gta_ast_node_walk(function_call->lhs, callback, data, return_value);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); ++i) {
    GTA_Ast_Node * argument = (GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]);
    gta_ast_node_walk(argument, callback, data, return_value);
  }
}


GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_function_call_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;
  GTA_Ast_Node * error = gta_ast_node_analyze(function_call->lhs, program, scope);
  if (error) {
    return error;
  }
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); ++i) {
    GTA_Ast_Node * argument = (GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]);
    error = gta_ast_node_analyze(argument, program, scope);
    if (error) {
      return error;
    }
  }
  return 0;
}


bool gta_ast_node_function_call_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;
  GTA_VectorX * b = context->program->bytecode;
  GTA_VectorX * o = context->bytecode_offsets;

  // Compile the arguments.
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); ++i) {
    if (!gta_ast_node_compile_to_bytecode((GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]), context)) {
      return false;
    }
  }

  return true
  // Compile the LHS.
    && gta_ast_node_compile_to_bytecode(function_call->lhs, context)
  // Call the function
    && GTA_BYTECODE_APPEND(o, b->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_CALL))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_VECTORX_COUNT(function_call->arguments)));
}
