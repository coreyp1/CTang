
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeInteger.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_integer_vtable = {
  .name = "Integer",
  .compile_to_bytecode = gta_ast_node_integer_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_integer_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_integer_destroy,
  .print = gta_ast_node_integer_print,
  .simplify = gta_ast_node_integer_simplify,
  .analyze = 0,
  .walk = gta_ast_node_integer_walk,
};


GTA_Ast_Node_Integer * gta_ast_node_integer_create(int64_t integer, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Integer * self = gcu_malloc(sizeof(GTA_Ast_Node_Integer));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Integer) {
    .base = {
      .vtable = &gta_ast_node_integer_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_INTEGER,
      .is_singleton = false,
    },
    .value = integer,
  };
  return self;
}


void gta_ast_node_integer_destroy(GTA_Ast_Node * self) {
  assert(self);
  gcu_free(self);
}


void gta_ast_node_integer_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_INTEGER(self));
  GTA_Ast_Node_Integer * integer = (GTA_Ast_Node_Integer *) self;

  assert(indent);
  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s: %ld\n", indent, self->vtable->name, integer->value);
}


GTA_Ast_Node * gta_ast_node_integer_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_integer_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  callback(self, data, return_value);
}


bool gta_ast_node_integer_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_INTEGER(self));
  GTA_Ast_Node_Integer * integer = (GTA_Ast_Node_Integer *) self;

  GTA_Computed_Value * singleton = gta_program_get_singleton(context->program, &gta_computed_value_integer_vtable, integer->value);
  if (!singleton) {
    singleton = (GTA_Computed_Value *)gta_computed_value_integer_create(integer->value, NULL);
    if (!singleton) {
      return false;
    }
    if (!gta_program_set_singleton(context->program, &gta_computed_value_integer_vtable, integer->value, singleton)) {
      gta_computed_value_destroy(singleton);
      return false;
    }
  }

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_LOAD))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_P(singleton));
}


bool gta_ast_node_integer_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_INTEGER(self));
  GTA_Ast_Node_Integer * integer = (GTA_Ast_Node_Integer *) self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  GTA_Computed_Value * singleton = gta_program_get_singleton(context->program, &gta_computed_value_integer_vtable, integer->value);
  if (!singleton) {
    singleton = (GTA_Computed_Value *)gta_computed_value_integer_create(integer->value, NULL);
    if (!singleton) {
      return false;
    }
    if (!gta_program_set_singleton(context->program, &gta_computed_value_integer_vtable, integer->value, singleton)) {
      gta_computed_value_destroy(singleton);
      return false;
    }
  }

  return true
  // mov rax, singleton
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (int64_t)singleton)
  ;
}
