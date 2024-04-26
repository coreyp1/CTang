
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include "tang/astNodeAssign.h"
#include "tang/astNodeIdentifier.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_assign_vtable = {
  .name = "Assign",
  .compile_to_bytecode = gta_ast_node_assign_compile_to_bytecode,
  .compile_to_binary = gta_ast_node_assign_compile_to_binary,
  .destroy = gta_ast_node_assign_destroy,
  .print = gta_ast_node_assign_print,
  .simplify = gta_ast_node_assign_simplify,
  .walk = gta_ast_node_assign_walk,
};

GTA_Ast_Node_Assign * gta_ast_node_assign_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Assign * self = gcu_malloc(sizeof(GTA_Ast_Node_Assign));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_assign_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->lhs = lhs;
  self->rhs = rhs;
  return self;
}

void gta_ast_node_assign_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;
  gta_ast_node_destroy(assign->lhs);
  gta_ast_node_destroy(assign->rhs);
  gcu_free(self);
}

void gta_ast_node_assign_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  printf("%s%s\n", indent, self->vtable->name);
  printf("%s  LHS:\n", indent);
  gta_ast_node_print(assign->lhs, new_indent);
  printf("%s  RHS:\n", indent);
  gta_ast_node_print(assign->rhs, new_indent);
  gcu_free(new_indent);
}

GTA_Ast_Node * gta_ast_node_assign_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;

  // TODO: Simplify the LHS.
  // The problem is that, if the LHS is an identifier, then simply calling
  // "gta_ast_node_simplify" will cause the identifier to be replaced with
  // the value that it represents, which is not what we want.  Yet, we *do*
  // want to simplify any other possible variables (e.g., y[x + 1], the
  // "x + 1" part should be simplified if possible).
  /*GTA_Ast_Node * simplified_lhs = gta_ast_node_simplify(assign->lhs, variable_map);
  if (simplified_lhs) {
    gta_ast_node_destroy(assign->lhs);
    assign->lhs = simplified_lhs;
  }*/
  GTA_Ast_Node * simplified_rhs = gta_ast_node_simplify(assign->rhs, variable_map);
  if (simplified_rhs) {
    gta_ast_node_destroy(assign->rhs);
    assign->rhs = simplified_rhs;
  }
  // If the LHS is an identifier, then we need to add it to the variable map.
  if (GTA_AST_IS_IDENTIFIER(assign->lhs)) {
    GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) assign->lhs;
    // Walk down the right hand side until we find a non-assign node.
    GTA_Ast_Node * rhs = assign->rhs;
    while (GTA_AST_IS_ASSIGN(rhs)) {
      rhs = ((GTA_Ast_Node_Assign *) rhs)->rhs;
    }
    gcu_hash64_set(variable_map, gcu_string_hash_64(identifier->identifier, strlen(identifier->identifier)), GCU_TYPE64_P(rhs));
  }
  return 0;
}

void gta_ast_node_assign_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;
  gta_ast_node_walk(assign->lhs, callback, data, return_value);
  gta_ast_node_walk(assign->rhs, callback, data, return_value);
}

bool gta_ast_node_assign_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;

  if (!gta_ast_node_compile_to_bytecode(assign->rhs, context)) {
    return false;
  }

  // An assignment may be in several forms:
  //   a = foo;
  //   a.b = foo;
  //   a[b] = foo;
  if (GTA_AST_IS_IDENTIFIER(assign->lhs)) {
    GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) assign->lhs;

    // If the identifier is a global variable, then the stack index is absolute
    // from the beginning of the stack.
    GTA_HashX_Value position = GTA_HASHX_GET(context->globals, identifier->hash);
    if (position.exists) {
      return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_ASSIGN_TO_BASE))
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_TYPEX_UI(position.value)));
    }

    // If the identifier is a local variable, then the stack index is relative
    // to the base pointer.
    position = GTA_HASHX_GET(GTA_TYPEX_P(context->scope_stack->data[context->scope_stack->count - 1]), identifier->hash);
    if (position.exists) {
      return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_ASSIGN))
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_TYPEX_UI(position.value)));
    }

    return false;
  }

  return false;
}

bool gta_ast_node_assign_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  (void) self;
  (void) context;
  GTA_Ast_Node_Assign * assign_node = (GTA_Ast_Node_Assign *) self;
  GCU_Vector8 * v = context->binary_vector;
  if (!gcu_vector8_reserve(v, v->count + 30)) {
    return false;
  }
#if defined(GTA_X86_64)
  // 64-bit x86
  (void) assign_node;
#endif
  return false;
}
