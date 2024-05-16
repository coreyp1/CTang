
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeIndex.h>
#include <tang/ast/astNodePeriod.h>
#include <tang/program/binaryCompilerContext.h>
#include <tang/program/variable.h>

GTA_Ast_Node_VTable gta_ast_node_assign_vtable = {
  .name = "Assign",
  .compile_to_bytecode = gta_ast_node_assign_compile_to_bytecode,
  .compile_to_binary = gta_ast_node_assign_compile_to_binary,
  .destroy = gta_ast_node_assign_destroy,
  .print = gta_ast_node_assign_print,
  .simplify = gta_ast_node_assign_simplify,
  .analyze = gta_ast_node_assign_analyze,
  .walk = gta_ast_node_assign_walk,
};


GTA_Ast_Node_Assign * gta_ast_node_assign_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Assign * self = gcu_malloc(sizeof(GTA_Ast_Node_Assign));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Assign) {
    .base = {
      .vtable = &gta_ast_node_assign_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .lhs = lhs,
    .rhs = rhs,
  };
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


GTA_Ast_Node * gta_ast_node_assign_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;
  GTA_Ast_Node * result = 0;
  result = gta_ast_node_analyze(assign->lhs, program, scope);
  if (!result) {
    result = gta_ast_node_analyze(assign->rhs, program, scope);
  }
  return result;
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

    if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY) {
      GTA_HashX_Value val = GTA_HASHX_GET(context->program->scope->global_positions, identifier->mangled_name_hash);
      if (!val.exists) {
        printf("Error: Identifier %s not found in global positions.\n", identifier->mangled_name);
        return false;
      }
      return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POKE_GLOBAL))
        && GTA_VECTORX_APPEND(context->program->bytecode, val.value);
    }
    else if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL) {
      GTA_HashX_Value val = GTA_HASHX_GET(identifier->scope->local_positions, identifier->mangled_name_hash);
      if (!val.exists) {
        printf("Error: Identifier %s not found in local positions.\n", identifier->mangled_name);
        return false;
      }
      return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POKE_LOCAL))
        && GTA_VECTORX_APPEND(context->program->bytecode, val.value);
    }

    return false;
  }

  return false;
}


static bool __compile_binary_lhs_is_identifier(GTA_Ast_Node * lhs, GTA_Binary_Compiler_Context * context) {
  // RHS is in RAX.
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) lhs;
  bool * is_singleton_offset = &((GTA_Computed_Value *)0)->is_singleton;
  bool * is_temporary_offset = &((GTA_Computed_Value *)0)->is_temporary;
  GCU_Vector8 * v = context->binary_vector;

  // Overview:
  // If the computed value is a singleton or temporary, then set the
  // `is_temporary` value to 1 and store the value in the appropriate location.
  // Otherwise, make a deep copy of the value and store the copy in the
  // appropriate location.

  if (!gcu_vector8_reserve(v, v->count + 41 + 27 + 17 + 8)) {
    return false;
  }

  /////////////////////////////////////////////////////////////////////////////
  // if (is_singleton || is_temporary) jump to done - 41 bytes
  /////////////////////////////////////////////////////////////////////////////
  //   mov rdx, is_singleton_offset  ; Load the byte offset of is_singleton.
  GTA_BINARY_WRITE2(v, 0x48, 0xBA);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(v->data + v->count - 8, &is_singleton_offset, 8);
  //   mov r8, [rax + rdx]           ; Load the is_singleton value.
  GTA_BINARY_WRITE4(v, 0x4C, 0x8B, 0x04, 0x10);
  //   mov rdx, is_temporary_offset  ; Load the byte offset of is_temporary.
  GTA_BINARY_WRITE2(v, 0x48, 0xBA);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(v->data + v->count - 8, &is_temporary_offset, 8);
  //   mov r9, [rax + rdx]           ; Load the is_temporary value.
  GTA_BINARY_WRITE4(v, 0x4C, 0x8B, 0x0C, 0x10);
  //   or r8, r9                     ; Combine the is_singleton and is_temporary values.
  GTA_BINARY_WRITE3(v, 0x4D, 0x09, 0xC8);
  //   jnz done                      ; If singleton, then jump to done.
  GTA_BINARY_WRITE2(v, 0x0F, 0x85);
  GTA_BINARY_WRITE4(v, 0xDE, 0xAD, 0xBE, 0xEF);
  GTA_Integer label_done = gta_binary_compiler_context_get_label(context);
  if (label_done < 0 || !gta_binary_compiler_context_add_label_jump(context, label_done, v->count - 4)) {
    return false;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Call the deep copy function. - 16 bytes
  /////////////////////////////////////////////////////////////////////////////
  // Set up for a function call.
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFFFFFFFFFF0
  GTA_BINARY_WRITE1(v, 0x55);
  GTA_BINARY_WRITE3(v, 0x48, 0x89, 0xE5);
  GTA_BINARY_WRITE4(v, 0x48, 0x83, 0xE4, 0xF0);
  //   mov rdi, rax                  ; Move the value to RDI.
  GTA_BINARY_WRITE3(v, 0x48, 0x89, 0xc7);
  //   mov rax, gta_computed_value_deep_copy ; Make a deep copy of the value.
  GTA_BINARY_WRITE2(v, 0x48, 0xB8);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  GTA_UInteger fp = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_deep_copy);
  memcpy(v->data + v->count - 8, &fp, 8);
  //   call rax
  GTA_BINARY_WRITE2(v, 0xFF, 0xD0);
  // Tear down the function call.
  //   mov rsp, rbp
  //   pop rbp
  GTA_BINARY_WRITE3(v, 0x48, 0x89, 0xEC);
  GTA_BINARY_WRITE1(v, 0x5D);

  /////////////////////////////////////////////////////////////////////////////
  // done: - 16 bytes
  //   is_temporary = 0
  /////////////////////////////////////////////////////////////////////////////
  //   done:                         ; Done.
  if (!gta_binary_compiler_context_set_label(context, label_done, v->count)) {
    return false;
  }
  //   mov rdx, is_temporary_offset  ; Load the byte offset of is_temporary.
  GTA_BINARY_WRITE2(v, 0x48, 0xBA);
  GTA_BINARY_WRITE8(v, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(v->data + v->count - 8, &is_temporary_offset, 8);
  //   xor rcx, rcx                  ; The the value for non-temporary.
  GTA_BINARY_WRITE3(v, 0x48, 0x31, 0xC9);
  //   mov [rax + rdx], al          ; Mark the value as non-temporary.
  GTA_BINARY_WRITE3(v, 0x88, 0x04, 0x10);

  /////////////////////////////////////////////////////////////////////////////
  // Store the value in the appropriate location. - 8 bytes max
  /////////////////////////////////////////////////////////////////////////////
  // RAX contains the final value of the RHS.
  if ((identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY)
    || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL)) {
    // Find the identifier's position in the global positions.
    GTA_HashX_Value val = GTA_HASHX_GET(context->program->scope->global_positions, identifier->mangled_name_hash);
    if (!val.exists) {
      printf("Error: Identifier %s not found in global positions.\n", identifier->mangled_name);
      return false;
    }

    // Reminder: r13 is the global pointer, but it points to the end of the
    // global variables.  We need to move the pointer back to the beginning of
    // the memory address for whichever variable we're trying to access.
    int32_t index = ((int32_t)GTA_TYPEX_UI(val.value) + 1) * -8;

    // Copy the value from the global position (GTA_TYPEX_UI(val.value)) to RAX.
    //   mov [r13 + index], rax
    GTA_BINARY_WRITE3(context->binary_vector, 0x49, 0x89, 0x85);
    GTA_BINARY_WRITE4(context->binary_vector, 0xDE, 0xAD, 0xBE, 0xEF);
    memcpy(&context->binary_vector->data[context->binary_vector->count - 4], &index, 4);

    return true;
  }

  if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL) {
    // Find the identifier's position in the local positions.
    GTA_HashX_Value val = GTA_HASHX_GET(identifier->scope->local_positions, identifier->mangled_name_hash);
    if (!val.exists) {
      printf("Error: Identifier %s not found in local positions.\n", identifier->mangled_name);
      return false;
    }

    // Reminder: r12 is the local pointer, but it points to the end of the
    // local variables.  We need to move the pointer back to the beginning of
    // the memory address for whichever variable we're trying to access.
    int32_t index = ((int32_t)GTA_TYPEX_UI(val.value) + 1) * -8;

    // Copy the value from the local position (GTA_TYPEX_UI(val.value)) to RAX.
    //   mov [r12 + index], rax
    GTA_BINARY_WRITE4(context->binary_vector, 0x49, 0x89, 0x84, 0x24);
    GTA_BINARY_WRITE4(context->binary_vector, 0xDE, 0xAD, 0xBE, 0xEF);
    memcpy(&context->binary_vector->data[context->binary_vector->count - 4], &index, 4);

    return true;
  }
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
