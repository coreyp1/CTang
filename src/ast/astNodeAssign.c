
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeIndex.h>
#include <tang/ast/astNodePeriod.h>
#include <tang/program/binaryCompilerContext.h>

GTA_Ast_Node_VTable gta_ast_node_assign_vtable = {
  .name = "Assign",
  .compile_to_bytecode = gta_ast_node_assign_compile_to_bytecode,
  .compile_to_binary = gta_ast_node_assign_compile_to_binary,
  .destroy = gta_ast_node_assign_destroy,
  .print = gta_ast_node_assign_print,
  .simplify = gta_ast_node_assign_simplify,
  .analyze = 0,
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


static bool __compile_binary_lhs_is_identifier(GTA_Ast_Node * lhs, GTA_Binary_Compiler_Context * context) {
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) lhs;
  bool is_global;
  uint64_t index;
  bool * is_temporary_offset = &((GTA_Computed_Value *)0)->is_temporary;
  bool * is_singleton_offset = &((GTA_Computed_Value *)0)->is_singleton;

  // If the identifier is a global variable, then the stack index is absolute
  // from the beginning of the stack.
  GTA_HashX_Value position = GTA_HASHX_GET(context->globals, identifier->hash);
  if (position.exists) {
    is_global = true;
    index = GTA_TYPEX_UI(position.value);
  }
  else {
    position = GTA_HASHX_GET(GTA_TYPEX_P(context->scope_stack->data[context->scope_stack->count - 1]), identifier->hash);
    // If the identifier is a local variable, then the stack index is relative
    // to the base pointer.
    if (position.exists) {
      is_global = false;
      index = GTA_TYPEX_UI(position.value);
    }
    else {
      return false;
    }
  }

  GCU_Vector8 * v = context->binary_vector;

#if defined(GTA_X86_64)
  // 64-bit x86
  // Determine whether or not the computed value in RAX is a temporary value.
  if (!gcu_vector8_reserve(v, v->count + 100)) {
    return false;
  }
  // If the value is a temporary value, then we can simply mark it as non-temporary
  // and store it in the appropriate location.
  // The computed value is in RAX.
  //  mov r8, 0xDEADBEEFDEADBEEF    ; The index of the value.
  GTA_BINARY_WRITE2(v, 0x49, 0xB8);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(v->data + v->count - 8, &index, 8);
  //  mov rdx, is_singleton_offset  ; Load the byte offset of is_singleton.
  GTA_BINARY_WRITE2(v, 0x48, 0xBA);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(v->data + v->count - 8, &is_singleton_offset, 8);
  //  lea rcx, [rax + rdx]          ; Load the is_singleton value.
  GTA_BINARY_WRITE4(v, 0x48, 0x8D, 0x0C, 0x10);
  //  cmp rcx, 1                    ; Compare the is_singleton value to 1.
  GTA_BINARY_WRITE4(v, 0x48, 0x83, 0xF9, 0x01);
  //  je done                       ; If singleton, then jump to done.
  GTA_BINARY_WRITE2(v, 0x0F, 0x84);
  GTA_BINARY_WRITE4(v, 0xDE, 0xAD, 0xBE, 0xEF);
  GTA_Integer label_done = gta_binary_compiler_context_get_label(context);
  if (label_done < 0) {
    return false;
  }
  if (!gta_binary_compiler_context_add_label_jump(context, label_done, v->count - 4)) {
    return false;
  }

  // not_singleton:
  //  mov rdx, is_temporary_offset  ; Load the byte offset of is_temporary.
  GTA_BINARY_WRITE2(v, 0x48, 0xBA);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(v->data + v->count - 8, &is_temporary_offset, 8);
  //  lea rcx, [rax + rdx]          ; Load the is_temporary value.
  GTA_BINARY_WRITE4(v, 0x48, 0x8D, 0x0C, 0x10);
  //  cmp rcx, 1                    ; Compare the is_temporary value to 1.
  GTA_BINARY_WRITE4(v, 0x48, 0x83, 0xF9, 0x01);
  //  jne not_temporary             ; If permanent, then jump to not_temporary.
  GTA_BINARY_WRITE2(v, 0x0F, 0x85);
  GTA_BINARY_WRITE4(v, 0xDE, 0xAD, 0xBE, 0xEF);
  GTA_Integer label_not_temporary = gta_binary_compiler_context_get_label(context);
  if (label_not_temporary < 0) {
    return false;
  }
  if (!gta_binary_compiler_context_add_label_jump(context, label_not_temporary, v->count - 4)) {
    return false;
  }
  //  mov rcx, 0                    ; The the value for non-temporary.
  GTA_BINARY_WRITE3(v, 0x48, 0xC7, 0xC1);
  GTA_BINARY_WRITE4(v, 0x00, 0x00, 0x00, 0x00);
  //  mov [rax + rdx], rcx          ; Mark the value as non-temporary.
  GTA_BINARY_WRITE4(v, 0x48, 0x89, 0x0C, 0x10);
  //  jmp done                      ; Jump to done.
  GTA_BINARY_WRITE1(v, 0xE9);
  GTA_BINARY_WRITE4(v, 0xDE, 0xAD, 0xBE, 0xEF);
  if (!gta_binary_compiler_context_add_label_jump(context, label_done, v->count - 4)) {
    return false;
  }

  // not_temporary:                 ; The value is not temporary.
  if (!gta_binary_compiler_context_set_label(context, label_not_temporary, v->count)) {
    return false;
  }
  //  mov rdi, rax                  ; Move the value to RDI.
  GTA_BINARY_WRITE3(v, 0x48, 0x89, 0xc7);
  //  mov rax, gta_computed_value_deep_copy ; Make a deep copy of the value.
  GTA_BINARY_WRITE2(v, 0x48, 0xB8);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  GTA_UInteger fp = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_deep_copy);
  memcpy(v->data + v->count - 8, &fp, 8);
  //  call rax
  GTA_BINARY_WRITE2(v, 0xFF, 0xD0);

  // done:                          ; Done.
  if (!gta_binary_compiler_context_set_label(context, label_done, v->count)) {
    return false;
  }
  // RCX and RDX are not needed, they can be used for other purposes.
  //   mov rcx, (is_global ? r13 : r12)
  GTA_BINARY_WRITE3(v, 0x4C, 0x89, is_global ? 0xE9 : 0xE1);
  // sub rcx, r8
  GTA_BINARY_WRITE3(v, 0x4C, 0x29, 0xC1);
  // mov [rcx], rax        ; Store the value in the appropriate location.
  GTA_BINARY_WRITE3(v, 0x48, 0x89, 0x01);

  return true;
#endif
  return false;
}


static bool __compile_binary_lhs_is_period(GTA_Ast_Node * lhs, GTA_Binary_Compiler_Context * context) {
  (void) lhs;
  (void) context;
  return false;
}


static bool __compile_binary_lhs_is_index(GTA_Ast_Node * lhs, GTA_Binary_Compiler_Context * context) {
  (void) lhs;
  (void) context;
  return false;
}


bool gta_ast_node_assign_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  GTA_Ast_Node_Assign * assign_node = (GTA_Ast_Node_Assign *) self;

  if (!gta_ast_node_compile_to_binary(assign_node->rhs, context)) {
    return false;
  }

  // TODO: Optimization: If the RHS is a constant singleton (bool, null), then
  // we can simplify the binary to immediately assign the value to the
  // variable.  If the RHS is a literal type (int, float, string), then we can
  // also simplify the binary to immediately change the `is_temporary` value
  // and adopt the value directly.  Array and object literals may be similarly
  // optimized.

  // An assignment may be in several forms:
  //   a = foo;
  //   a.b = foo;
  //   a[b] = foo;
  return GTA_AST_IS_IDENTIFIER(assign_node->lhs)
    ? __compile_binary_lhs_is_identifier(assign_node->lhs, context)
    : GTA_AST_IS_PERIOD(assign_node->lhs)
      ? __compile_binary_lhs_is_period(assign_node->lhs, context)
      : GTA_AST_IS_INDEX(assign_node->lhs)
        ? __compile_binary_lhs_is_index(assign_node->lhs, context)
        : false;
}
