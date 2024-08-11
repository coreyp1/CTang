
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeReturn.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_return_vtable = {
  .name = "Return",
  .compile_to_bytecode = gta_ast_node_return_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_return_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_return_destroy,
  .print = gta_ast_node_return_print,
  .simplify = gta_ast_node_return_simplify,
  .analyze = gta_ast_node_return_analyze,
  .walk = gta_ast_node_return_walk,
};


GTA_Ast_Node_Return * gta_ast_node_return_create(GTA_Ast_Node * expression, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Return * self = gcu_malloc(sizeof(GTA_Ast_Node_Return));
  if (!self) {
    return 0;
  }

  if (!expression) {
    // No expression was supplied.  Default to NULL.
    expression = gta_ast_node_create(location);
    if (!expression) {
      gcu_free(self);
      return 0;
    }
  }

  *self = (GTA_Ast_Node_Return) {
    .base = {
      .vtable = &gta_ast_node_return_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .expression = expression,
  };
  return self;
}


void gta_ast_node_return_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_RETURN(self));
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;

  gta_ast_node_destroy(return_node->expression);

  gcu_free(self);
}


void gta_ast_node_return_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_RETURN(self));
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;

  assert(indent);
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);

  assert(return_node->expression);
  assert(self->vtable);
  assert(self->vtable->name);
  if (return_node->expression) {
    printf("%s%s:\n", indent, self->vtable->name);
    gta_ast_node_print(return_node->expression, new_indent);
  }
  else {
    printf("%s%s\n", indent, self->vtable->name);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_return_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_RETURN(self));
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;

  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(return_node->expression, variable_map);

  if (simplified_expression) {
    gta_ast_node_destroy(return_node->expression);
    return_node->expression = simplified_expression;
  }
  return 0;
}


GTA_Ast_Node * gta_ast_node_return_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_RETURN(self));
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;

  assert(return_node->expression);
  return gta_ast_node_analyze(return_node->expression, program, scope);
}


void gta_ast_node_return_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_RETURN(self));
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;

  callback(self, data, return_value);

  assert(return_node->expression);
  gta_ast_node_walk(return_node->expression, callback, data, return_value);
}


bool gta_ast_node_return_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_RETURN(self));
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  GTA_VectorX * b = context->program->bytecode;
  GTA_VectorX * o = context->bytecode_offsets;

  assert(return_node->expression);

  return true
  // Compile the expression to bytecode.
    && gta_ast_node_compile_to_bytecode(return_node->expression, context)
  // Jump to the return label.
  //   JMP context->return_label
    && GTA_BYTECODE_APPEND(o, b->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMP))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(context, context->return_label, b->count - 1)
  ;
}


bool gta_ast_node_return_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_RETURN(self));
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  return true
  // Compile the expression to binary.
    && gta_ast_node_compile_to_binary__x86_64(return_node->expression, context)
  // Jump to the return label.
  //   jmp context->return_label
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, context->return_label, v->count - 4)
  ;
}
