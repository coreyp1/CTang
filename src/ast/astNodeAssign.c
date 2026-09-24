/*
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * Copyright (C) 2024-2026 Corey Pennycuff
 *
 * This file is part of Ghoti.io Tang.
 *
 * Ghoti.io Tang is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Ghoti.io Tang is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ghoti.io/cutil/memory.h>
#include <ghoti.io/cutil/string.h>
#include <ghoti.io/tang/macros.h>
#include <ghoti.io/tang/ast/astNodeAssign.h>
#include <ghoti.io/tang/ast/astNodeIdentifier.h>
#include <ghoti.io/tang/ast/astNodeIndex.h>
#include <ghoti.io/tang/ast/astNodeParseError.h>
#include <ghoti.io/tang/ast/astNodePeriod.h>
#include <ghoti.io/tang/program/binary.h>
#include <ghoti.io/tang/program/bytecode.h>
#include <ghoti.io/tang/program/variable.h>
#include <ghoti.io/tang/computedValue/computedValueString.h>
#include <ghoti.io/tang/unicodeString.h>

GTA_Ast_Node_VTable gta_ast_node_assign_vtable = {
  .name = "Assign",
  .compile_to_bytecode = gta_ast_node_assign_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_assign_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_assign_destroy,
  .print = gta_ast_node_assign_print,
  .simplify = gta_ast_node_assign_simplify,
  .analyze = gta_ast_node_assign_analyze,
  .walk = gta_ast_node_assign_walk,
};


GTA_Ast_Node_Assign * gta_ast_node_assign_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_PARSER_LTYPE location) {
  assert(lhs);
  assert(rhs);

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
  assert(self);
  assert(GTA_AST_IS_ASSIGN(self));
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;

  gta_ast_node_destroy(assign->lhs);
  gta_ast_node_destroy(assign->rhs);
  gcu_free(self);
}


void gta_ast_node_assign_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_ASSIGN(self));
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;

  assert(indent);
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }

  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);

  assert(self->vtable->name);
  printf("%s%s\n", indent, self->vtable->name);

  printf("%s  LHS:\n", indent);
  gta_ast_node_print(assign->lhs, new_indent);

  printf("%s  RHS:\n", indent);
  gta_ast_node_print(assign->rhs, new_indent);

  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_assign_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_ASSIGN(self));
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
  assert(self);
  assert(GTA_AST_IS_ASSIGN(self));
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;

  // Only a name, a member and a subscript can be assigned to.  Anything else
  // - a slice, a literal, the result of a call - is rejected here, once,
  // rather than by each compiler returning false from the middle of an
  // emission it has already half done.
  if (!GTA_AST_IS_IDENTIFIER(assign->lhs)
    && !GTA_AST_IS_PERIOD(assign->lhs)
    && !GTA_AST_IS_INDEX(assign->lhs)) {
    return (GTA_Ast_Node *)gta_ast_node_parse_error_create("Cannot assign to this expression.", self->location);
  }

  GTA_Ast_Node * result = 0;
  result = gta_ast_node_analyze(assign->lhs, program, scope);
  if (!result) {
    result = gta_ast_node_analyze(assign->rhs, program, scope);
  }
  return result;
}


void gta_ast_node_assign_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_ASSIGN(self));
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;

  callback(self, data, return_value);

  gta_ast_node_walk(assign->lhs, callback, data, return_value);
  gta_ast_node_walk(assign->rhs, callback, data, return_value);
}


/**
 * Get the interned string value for a period's member name.
 *
 * `a.b = v` means `a["b"] = v`, so the member name has to reach the run time
 * as a string value.  The name lives in the AST for as long as the program
 * does, so its address is a stable interning key, the same way a string
 * literal node uses the address of its own GTA_Unicode_String.
 *
 * @param context The compiler context.
 * @param name The member name.
 * @return The interned string value, or NULL if it could not be created.
 */
static GTA_Computed_Value * period_member_name(GTA_Compiler_Context * context, const char * name) {
  assert(context);
  assert(context->program);
  assert(name);

  GTA_UInteger hash = (GTA_UInteger)name;
  GTA_Computed_Value * singleton = gta_program_get_singleton(context->program, &gta_computed_value_string_vtable, hash);
  if (singleton) {
    return singleton;
  }

  // The member name is written in the source, so it is as trusted as the
  // program text itself.
  GTA_Unicode_String * string = gta_unicode_string_create(name, strlen(name), GTA_UNICODE_STRING_TYPE_TRUSTED);
  if (!string) {
    return NULL;
  }
  singleton = (GTA_Computed_Value *)gta_computed_value_string_create(string, true, NULL);
  if (!singleton) {
    gta_unicode_string_destroy(string);
    return NULL;
  }
  if (!gta_program_set_singleton(context->program, &gta_computed_value_string_vtable, hash, singleton)) {
    gta_computed_value_destroy(singleton);
    return NULL;
  }
  return singleton;
}


bool gta_ast_node_assign_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_ASSIGN(self));
  GTA_Ast_Node_Assign * assign = (GTA_Ast_Node_Assign *) self;

  if (!gta_ast_node_compile_to_bytecode(assign->rhs, context)) {
    return false;
  }

  // An assignment may be in several forms:
  //   a = foo;
  //   a.b = foo;
  //   a[b] = foo;
  assert(assign->lhs);
  if (GTA_AST_IS_IDENTIFIER(assign->lhs)) {
    GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) assign->lhs;

    if ((identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY)
      || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL)) {
      const char *  name= identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY
        ? identifier->mangled_name
        : identifier->identifier;
      GTA_Integer name_hash = identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY
        ? identifier->mangled_name_hash
        : identifier->hash;
      GTA_HashX_Value val = GTA_HASHX_GET(context->program->scope->variable_positions, name_hash);
      if (!val.exists) {
        printf("Error: Identifier %s not found in global positions.\n", name);
        return false;
      }
      // Clear is_temporary before storing. A stored value that is still marked
      // temporary is fair game for the in-place fast path in the arithmetic
      // operators, which mutates an operand rather than allocating a result -
      // so `a = a + 1; b = a + 3;` left a equal to 9 instead of 6.
      //
      // SET_NOT_TEMP rather than ADOPT deliberately: ADOPT also deep-copies a
      // non-temporary, which would give assignment value semantics for arrays
      // and maps. Both engines alias them today, so that is a language design
      // question and not this fix's to answer.
      return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_SET_NOT_TEMP))
        && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POKE_GLOBAL))
        && GTA_VECTORX_APPEND(context->program->bytecode, val.value);
    }
    else if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL) {
      GTA_HashX_Value val = GTA_HASHX_GET(identifier->scope->variable_positions, identifier->mangled_name_hash);
      if (!val.exists) {
        printf("Error: Identifier %s not found in local positions.\n", identifier->mangled_name);
        return false;
      }
      // Clear is_temporary before storing; see the global case above.
      return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_SET_NOT_TEMP))
        && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POKE_LOCAL))
        && GTA_VECTORX_APPEND(context->program->bytecode, val.value);
    }

    return false;
  }

  if (GTA_AST_IS_PERIOD(assign->lhs)) {
    // `a.b = v` is `a["b"] = v`.  Section 4.13 of the language reference says
    // attribute assignment is for map members, and routing it through
    // assign_index is what makes that true of every other type as well: an
    // array answers "invalid index" to a string subscript, and a string, a
    // library and null all answer "not supported", instead of the assignment
    // being accepted on anything at all.
    GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) assign->lhs;
    GTA_Computed_Value * name = period_member_name(context, period->rhs);
    if (!name) {
      return false;
    }

    return true
    // Compile the lhs expression.
      && gta_ast_node_compile_to_bytecode(period->lhs, context)
    // Push the member name as the index.
      && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_LOAD))
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_P(name))
    // Compile the rhs expression.
      && gta_ast_node_compile_to_bytecode(assign->rhs, context)
    // Store the value in the appropriate location.
      && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_ASSIGN_INDEX));
  }

  if (GTA_AST_IS_INDEX(assign->lhs)) {
    GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) assign->lhs;

    return true
    // Compile the lhs expression.
      && gta_ast_node_compile_to_bytecode(index->lhs, context)
    // Compile the lhs index.
      && gta_ast_node_compile_to_bytecode(index->rhs, context)
    // Compile the rhs expression.
      && gta_ast_node_compile_to_bytecode(assign->rhs, context)
    // Store the value in the appropriate location.
      && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_ASSIGN_INDEX));
  }

  return false;
}


static bool __compile_binary_lhs_is_identifier__x86_64(GTA_Ast_Node * lhs, GTA_Compiler_Context * context) {
  // RHS is in RAX.
  assert(lhs);
  assert(GTA_AST_IS_IDENTIFIER(lhs));
  GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *) lhs;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  // Find the identifier's position in the global or local positions.
  GTA_HashX_Value val;
  bool look_in_global = (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY)
    || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL);
  val = look_in_global
    ? GTA_HASHX_GET(context->program->scope->variable_positions, identifier->mangled_name_hash)
    : GTA_HASHX_GET(identifier->scope->variable_positions, identifier->mangled_name_hash);
  if (!val.exists) {
    printf("Error: Identifier %s not found in %s positions.\n", identifier->mangled_name, look_in_global ? "global" : "local");
    return false;
  }
  // Reminder: r12 & r13 are pointers, but they point to the end of the
  // variable stacks.  We will need to move the pointer back to the beginning
  // of the memory address for whichever variable we're trying to access.
  int32_t index = ((int32_t)GTA_TYPEX_UI(val.value) + 1) * -8;

  bool * is_temporary_offset = &((GTA_Computed_Value *)0)->is_temporary;

  return true
  // Clear is_temporary before storing. A stored value that is still marked
  // temporary is fair game for the in-place fast path in the arithmetic
  // operators, which mutates an operand rather than allocating a result - so
  // `a = a + 1; b = a + 3;` left a equal to 9 instead of 6.
  //
  // Clearing the flag is all that happens here: `b = a` gives both names the
  // same array, as section 3 of the language reference says it does for every
  // reference type. The bytecode compiler emits SET_NOT_TEMP for the same
  // reason.
  //
  //   mov byte ptr [rax + is_temporary_offset], 0
    && gta_mov_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)is_temporary_offset, 0)

  // Store the value in the appropriate location.
  // RAX contains the final value of the RHS.
  // Copy the value from the indexed position (GTA_TYPEX_UI(val.value)) to RAX.
  // If global:
  //   mov [r13 + index], rax
  // If local:
  //   mov [r12 + index], rax
    && gta_mov_ind_reg__x86_64(v, (look_in_global ? GTA_REG_R13 : GTA_REG_R12), GTA_REG_NONE, 0, index, GTA_REG_RAX);
}


/**
 * Compile `a.b = v` for x86-64.
 *
 * The same sequence the index case uses, with the member name loaded as an
 * immediate where the index expression would be.  See the note there about
 * why each intermediate takes a pair of stack slots.
 *
 * @param lhs The period node on the left of the assignment.
 * @param rhs The expression on the right of the assignment.
 * @param context The compiler context.
 * @return true on success, false on failure.
 */
static bool __compile_binary_lhs_is_period(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_Compiler_Context * context) {
  assert(lhs);
  assert(GTA_AST_IS_PERIOD(lhs));
  GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) lhs;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  GTA_Computed_Value * name = period_member_name(context, period->rhs);
  if (!name) {
    return false;
  }

  return true
  // Compile the lhs expression.
  //    push rax
  //    push rax   ; alignment padding
    && gta_ast_node_compile_to_binary__x86_64(period->lhs, context)
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
  // Load the member name as the index.
  //    mov rax, name
  //    push rax
  //    push rax   ; alignment padding
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (GTA_UInteger)name)
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
  // Compile the rhs expression.
  //    push rax
  //    push rax   ; alignment padding
    && gta_ast_node_compile_to_binary__x86_64(rhs, context)
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
    && gta_push_reg__x86_64(v, GTA_REG_RAX)
  // gta_computed_value_assign_index(expression, index, value, context)
  //
  // The context is the fourth argument, so it goes in R4.  It used to be
  // loaded into R2 - the second - and the pop below then wrote the index
  // over it, leaving the callee to read whatever happened to be in R4 as a
  // context.  Nothing noticed while the value being assigned was temporary,
  // because the array adopts those without touching the context; assigning a
  // named value took the deep-copy path instead and crashed.
  //    mov GTA_X86_64_R4, r15
  //    pop GTA_X86_64_R3 ; add rsp, 8
  //    pop GTA_X86_64_R2 ; add rsp, 8
  //    pop GTA_X86_64_R1 ; add rsp, 8
    && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R4, GTA_REG_R15)
    && gta_pop_reg__x86_64(v, GTA_X86_64_R3)
    && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 8)
    && gta_pop_reg__x86_64(v, GTA_X86_64_R2)
    && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 8)
    && gta_pop_reg__x86_64(v, GTA_X86_64_R1)
    && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 8)
    && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_assign_index);
}


bool gta_ast_node_assign_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_ASSIGN(self));
  GTA_Ast_Node_Assign * assign_node = (GTA_Ast_Node_Assign *) self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

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
    ? (true
    // Compile the rhs expression.
      && gta_ast_node_compile_to_binary__x86_64(assign_node->rhs, context)
      && __compile_binary_lhs_is_identifier__x86_64(assign_node->lhs, context)
    )
    : GTA_AST_IS_PERIOD(assign_node->lhs)
      ? __compile_binary_lhs_is_period(assign_node->lhs, assign_node->rhs, context)
      : GTA_AST_IS_INDEX(assign_node->lhs)
        ? (true
        // Each intermediate is saved in a PAIR of stack slots, not one.
        // The prologue leaves rsp 16-byte aligned, so alignment holds only
        // while an even number of 8-byte slots are live. Saving these singly
        // left the nested expression compilations below running at odd parity,
        // so any call they emitted violated the System V AMD64 ABI. The
        // duplicate slot is pure padding.
        // Compile the lhs expression.
        //    push rax
        //    push rax   ; alignment padding
          && gta_ast_node_compile_to_binary__x86_64(((GTA_Ast_Node_Index *)assign_node->lhs)->lhs, context)
          && gta_push_reg__x86_64(v, GTA_REG_RAX)
          && gta_push_reg__x86_64(v, GTA_REG_RAX)
        // Compile the lhs index.
        //    push rax
        //    push rax   ; alignment padding
          && gta_ast_node_compile_to_binary__x86_64(((GTA_Ast_Node_Index *)assign_node->lhs)->rhs, context)
          && gta_push_reg__x86_64(v, GTA_REG_RAX)
          && gta_push_reg__x86_64(v, GTA_REG_RAX)
        // Compile the rhs expression.
        //    push rax
        //    push rax   ; alignment padding
          && gta_ast_node_compile_to_binary__x86_64(assign_node->rhs, context)
          && gta_push_reg__x86_64(v, GTA_REG_RAX)
          && gta_push_reg__x86_64(v, GTA_REG_RAX)
        // gta_computed_value_assign_index(expression, index, value, context)
        // Each pop takes the value and then discards its padding slot.
        //
        // The context is the fourth argument, so it goes in R4.  It used to
        // be loaded into R2 - the second - and the pop below then wrote the
        // index over it, leaving the callee to read whatever happened to be
        // in R4 as a context.  Nothing noticed while the value being assigned
        // was temporary, because a container adopts those without touching
        // the context; assigning a named value takes the deep-copy path
        // instead, and `x = [1, 2]; x[0] = x;` crashed.
        //    mov GTA_X86_64_R4, r15
        //    pop GTA_X86_64_R3 ; add rsp, 8
        //    pop GTA_X86_64_R2 ; add rsp, 8
        //    pop GTA_X86_64_R1 ; add rsp, 8
          && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R4, GTA_REG_R15)
          && gta_pop_reg__x86_64(v, GTA_X86_64_R3)
          && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 8)
          && gta_pop_reg__x86_64(v, GTA_X86_64_R2)
          && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 8)
          && gta_pop_reg__x86_64(v, GTA_X86_64_R1)
          && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 8)
          && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_assign_index)
        )
        : false;
}
