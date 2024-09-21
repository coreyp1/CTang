
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeIndex.h>
#include <tang/ast/astNodePeriod.h>
#include <tang/program/binary.h>
#include <tang/program/bytecode.h>
#include <tang/program/variable.h>

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
      return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POKE_GLOBAL))
        && GTA_VECTORX_APPEND(context->program->bytecode, val.value);
    }
    else if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL) {
      GTA_HashX_Value val = GTA_HASHX_GET(identifier->scope->variable_positions, identifier->mangled_name_hash);
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

  if (GTA_AST_IS_PERIOD(assign->lhs)) {
    // This is explicitly not supported.
    return false;
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

  return true
  // Adopt the value.
    && gta_binary_adopt__x86_64(context, GTA_REG_RAX, GTA_REG_RDX, GTA_REG_R8, GTA_REG_R9)

  // Store the value in the appropriate location.
  // RAX contains the final value of the RHS.
  // Copy the value from the indexed position (GTA_TYPEX_UI(val.value)) to RAX.
  // If global:
  //   mov [r13 + index], rax
  // If local:
  //   mov [r12 + index], rax
    && gta_mov_ind_reg__x86_64(v, (look_in_global ? GTA_REG_R13 : GTA_REG_R12), GTA_REG_NONE, 0, index, GTA_REG_RAX);
}


static bool __compile_binary_lhs_is_period(GTA_Ast_Node * lhs, GTA_Compiler_Context * context) {
  (void) lhs;
  (void) context;
  return false;
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
      ? __compile_binary_lhs_is_period(assign_node->lhs, context)
      : GTA_AST_IS_INDEX(assign_node->lhs)
        ? (true
        // Compile the lhs expression.
        //    push rax
          && gta_ast_node_compile_to_binary__x86_64(((GTA_Ast_Node_Index *)assign_node->lhs)->lhs, context)
          && gta_push_reg__x86_64(v, GTA_REG_RAX)
        // Compile the lhs index.
        //    push rax
          && gta_ast_node_compile_to_binary__x86_64(((GTA_Ast_Node_Index *)assign_node->lhs)->rhs, context)
          && gta_push_reg__x86_64(v, GTA_REG_RAX)
        // Compile the rhs expression.
        //    push rax
          && gta_ast_node_compile_to_binary__x86_64(assign_node->rhs, context)
          && gta_push_reg__x86_64(v, GTA_REG_RAX)
        // gta_computed_value_assign_index(expression, index, value, context)
        //    mov GTA_X86_64_R2, r15
        //    pop GTA_X86_64_R3
        //    pop GTA_X86_64_R2
        //    pop GTA_X86_64_R1
          && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R2, GTA_REG_R15)
          && gta_pop_reg__x86_64(v, GTA_X86_64_R3)
          && gta_pop_reg__x86_64(v, GTA_X86_64_R2)
          && gta_pop_reg__x86_64(v, GTA_X86_64_R1)
          && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_assign_index)
        )
        : false;
}
