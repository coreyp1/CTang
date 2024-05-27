
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeIndex.h>
#include <tang/ast/astNodePeriod.h>
#include <tang/program/binary.h>
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
  GTA_Integer label_done;
  GTA_HashX_Value val;

  // Overview:
  // If the computed value is a singleton or temporary, then set the
  // `is_temporary` value to 1 and store the value in the appropriate location.
  // Otherwise, make a deep copy of the value and store the copy in the
  // appropriate location.

  // Find the identifier's position in the global or local positions.
  bool look_in_global = (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY)
    || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL);
  val = look_in_global
    ? GTA_HASHX_GET(context->program->scope->global_positions, identifier->mangled_name_hash)
    : GTA_HASHX_GET(identifier->scope->local_positions, identifier->mangled_name_hash);
  if (!val.exists) {
    printf("Error: Identifier %s not found in %s positions.\n", identifier->mangled_name, look_in_global ? "global" : "local");
    return false;
  }
  // Reminder: r12 & r13 are pointers, but they point to the end of the
  // variable stacks.  We will need to move the pointer back to the beginning
  // of the memory address for whichever variable we're trying to access.
  int32_t index = ((int32_t)GTA_TYPEX_UI(val.value) + 1) * -8;

  return true
  /////////////////////////////////////////////////////////////////////////////
  // if (is_singleton || is_temporary) jump to done
  /////////////////////////////////////////////////////////////////////////////
  //   mov rdx, is_singleton_offset  ; Load the byte offset of is_singleton.
  //   mov r8, [rax + rdx]           ; Load the is_singleton value.
  //   mov rdx, is_temporary_offset  ; Load the byte offset of is_temporary.
  //   mov r9, [rax + rdx]           ; Load the is_temporary value.
  //   or r8, r9                     ; Combine the is_singleton and is_temporary values.
  //   jnz done                      ; If singleton, then jump to done.
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDX, (int64_t)is_singleton_offset)
    && gta_mov_reg_ind__x86_64(v, GTA_REG_R8, GTA_REG_RAX, GTA_REG_RDX, 1, 0)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDX, (int64_t)is_temporary_offset)
    && gta_mov_reg_ind__x86_64(v, GTA_REG_R9, GTA_REG_RAX, GTA_REG_RDX, 1, 0)
    && gta_or_reg_reg__x86_64(v, GTA_REG_R8, GTA_REG_R9)
    && gta_jnz__x86_64(v, 0xDEADBEEF)
    && ((label_done = gta_binary_compiler_context_get_label(context)) >= 0)
    && gta_binary_compiler_context_add_label_jump(context, label_done, v->count - 4)

  /////////////////////////////////////////////////////////////////////////////
  // Call the deep copy function.
  /////////////////////////////////////////////////////////////////////////////
  // Set up for a function call.
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFFFFFFFFFF0
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, 0xFFFFFFF0)
  //   mov rdi, rax                  ; Move the value to RDI.
  //   mov rax, gta_computed_value_deep_copy ; Make a deep copy of the value.
  //   call rax
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RDI, GTA_REG_RAX)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (int64_t)gta_computed_value_deep_copy)
    && gta_call_reg__x86_64(v, GTA_REG_RAX)
  // Tear down the function call.
  //   mov rsp, rbp
  //   pop rbp
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP)

  /////////////////////////////////////////////////////////////////////////////
  // done:
  //   is_temporary = 0
  /////////////////////////////////////////////////////////////////////////////
  //   done:                         ; Done.
  //   mov rdx, is_temporary_offset  ; Load the byte offset of is_temporary.
  //   xor rcx, rcx                  ; The the value for non-temporary.
  //   mov [rax + rdx], cl           ; Mark the value as non-temporary.
    && gta_binary_compiler_context_set_label(context, label_done, v->count)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDX, (int64_t)is_temporary_offset)
    && gta_xor_reg_reg__x86_64(v, GTA_REG_RCX, GTA_REG_RCX)
    && gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RDX, 1, 0, GTA_REG_CL)

  /////////////////////////////////////////////////////////////////////////////
  // Store the value in the appropriate location.
  /////////////////////////////////////////////////////////////////////////////
  // RAX contains the final value of the RHS.
  // Copy the value from the indexed position (GTA_TYPEX_UI(val.value)) to RAX.
  // If global:
  //   mov [r13 + index], rax
  // If local:
  //   mov [r12 + index], rax
    && gta_mov_ind_reg__x86_64(v, (look_in_global ? GTA_REG_R13 : GTA_REG_R12), GTA_REG_NONE, 0, index, GTA_REG_RAX);
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
