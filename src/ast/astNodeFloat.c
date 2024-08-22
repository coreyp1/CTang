
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeFloat.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/program/binary.h>
#include <tang/program/bytecode.h>

GTA_Ast_Node_VTable gta_ast_node_float_vtable = {
  .name = "Float",
  .compile_to_bytecode = gta_ast_node_float_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_float_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_float_destroy,
  .print = gta_ast_node_float_print,
  .simplify = gta_ast_node_float_simplify,
  .analyze = 0,
  .walk = gta_ast_node_float_walk,
};


GTA_Ast_Node_Float * gta_ast_node_float_create(GCU_float64_t value, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Float * self = gcu_malloc(sizeof(GTA_Ast_Node_Float));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Float) {
    .base = {
      .vtable = &gta_ast_node_float_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_FLOAT,
      .is_singleton = false,
    },
    .value = value,
  };
  return self;
}


void gta_ast_node_float_destroy(GTA_Ast_Node * self) {
  assert(self);
  gcu_free(self);
}


void gta_ast_node_float_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_FLOAT(self));
  GTA_Ast_Node_Float * float_node = (GTA_Ast_Node_Float *) self;

  assert(indent);
  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s: %f\n", indent, self->vtable->name, float_node->value);
}


GTA_Ast_Node * gta_ast_node_float_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_float_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  callback(self, data, return_value);
}


bool gta_ast_node_float_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_FLOAT(self));
  GTA_Ast_Node_Float * float_node = (GTA_Ast_Node_Float *) self;

  GTA_Computed_Value * singleton = gta_program_get_singleton(context->program, &gta_computed_value_float_vtable, float_node->value);
  if (!singleton) {
    singleton = (GTA_Computed_Value *)gta_computed_value_float_create(float_node->value, NULL);
    if (!singleton) {
      return false;
    }
    if (!gta_program_set_singleton(context->program, &gta_computed_value_float_vtable, float_node->value, singleton)) {
      gta_computed_value_destroy(singleton);
      return false;
    }
  }

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  return true
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_LOAD))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_P(singleton))
  ;
}


bool gta_ast_node_float_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_FLOAT(self));
  GTA_Ast_Node_Float * float_node = (GTA_Ast_Node_Float *) self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  GTA_Computed_Value * singleton = gta_program_get_singleton(context->program, &gta_computed_value_float_vtable, float_node->value);
  if (!singleton) {
    singleton = (GTA_Computed_Value *)gta_computed_value_float_create(float_node->value, NULL);
    if (!singleton) {
      return false;
    }
    if (!gta_program_set_singleton(context->program, &gta_computed_value_float_vtable, float_node->value, singleton)) {
      gta_computed_value_destroy(singleton);
      return false;
    }
  }

  return true
  // mov rax, singleton
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (int64_t)singleton)
  ;
}
