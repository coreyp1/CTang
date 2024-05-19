
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/type.h>
#include <tang/ast/astNode.h>
#include <tang/computedValue/computedValue.h>
#include <tang/program/binary.h>
#include <tang/program/bytecode.h>
#include <tang/program/bytecodeCompilerContext.h>
#include <tang/program/program.h>

GTA_Ast_Node_VTable gta_ast_node_null_vtable = {
  .name = "Null",
  .compile_to_bytecode = gta_ast_node_null_compile_to_bytecode,
  .compile_to_binary = gta_ast_node_null_compile_to_binary,
  .destroy = gta_ast_node_null_destroy,
  .print = gta_ast_node_null_print,
  .simplify = gta_ast_node_null_simplify,
  .analyze = 0,
  .walk = gta_ast_node_null_walk,
};


GTA_Ast_Node * GTA_CALL gta_ast_node_create(GTA_PARSER_LTYPE location) {
  GTA_Ast_Node * self = gcu_malloc(sizeof(GTA_Ast_Node));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node) {
    .vtable = &gta_ast_node_null_vtable,
    .location = location,
    .possible_type = GTA_AST_POSSIBLE_TYPE_NULL,
    .is_singleton = false,
  };
  return self;
}


void GTA_CALL gta_ast_node_destroy(GTA_Ast_Node * self) {
  !self->is_singleton && self->vtable->destroy
    ? self->vtable->destroy(self)
    : gta_ast_node_null_destroy(self);
}


bool gta_ast_node_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  return self->vtable->compile_to_binary
    ? self->vtable->compile_to_binary(self, context)
    : true;
}


bool gta_ast_node_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  return self->vtable->compile_to_bytecode
    ? self->vtable->compile_to_bytecode(self, context)
    : true;
}


void gta_ast_node_print(GTA_Ast_Node * self, const char * indent) {
  self->vtable->print
    ? self->vtable->print(self, indent)
    : gta_ast_node_null_print(self, indent);
}


GTA_Ast_Node * gta_ast_node_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  return self->vtable->simplify
    ? self->vtable->simplify(self, variable_map)
    : gta_ast_node_null_simplify(self, variable_map);
}


GTA_Ast_Node * gta_ast_node_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  return self->vtable->analyze
    ? self->vtable->analyze(self, program, scope)
    : NULL;
}


void gta_ast_node_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  self->vtable->walk
    ? self->vtable->walk(self, callback, data, return_value)
    : gta_ast_node_null_walk(self, callback, data, return_value);
}


bool gta_ast_node_null_compile_to_binary(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Binary_Compiler_Context * context)) {
  GCU_Vector8 * v = context->binary_vector;

  // TODO: Replace with:
  //   mov rax, gta_computed_value_null
  return true
    // Set up for a function call.
    //   push rbp
    //   mov rbp, rsp
    //   and rsp, 0xFFFFFFFFFFFFFFF0
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, (int32_t)0xFFFFFFF0)
    // Assembly to call gta_computed_value_create(0):
    //   mov rax, gta_computed_value_create
    //   mov rdi, 0
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_create))
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RDI, 0)
    //   call rax
    && gta_call_reg__x86_64(v, GTA_REG_RAX)
    // Tear down the function call.
    //   mov rsp, rbp
    //   pop rbp
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP);
}


bool gta_ast_node_null_compile_to_bytecode(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_Bytecode_Compiler_Context * context) {
  return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count) && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_NULL));
}


void GTA_CALL gta_ast_node_null_destroy(GTA_Ast_Node * self) {
  gcu_free(self);
}


void gta_ast_node_null_print(GTA_Ast_Node * self, const char * indent) {
  printf("%s%s\n", indent, self->vtable->name);
}


GTA_Ast_Node * gta_ast_node_null_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_null_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}


void gta_ast_simplify_variable_map_invalidate(GTA_Ast_Simplify_Variable_Map * variable_map) {
  // Iterate through the map and remove all of the entries.
  GCU_Hash64_Iterator iterator = gcu_hash64_iterator_get(variable_map);
  while (iterator.exists) {
    gcu_hash64_remove(variable_map, iterator.hash);
    iterator = gcu_hash64_iterator_next(iterator);
  }
}


void gta_ast_simplify_variable_map_synchronize(GTA_Ast_Simplify_Variable_Map * target, GTA_Ast_Simplify_Variable_Map * source) {
  // Iterate through the target map and remove any entries that mismatch the
  // source map.
  GCU_Hash64_Iterator target_iterator = gcu_hash64_iterator_get(target);
  while (target_iterator.exists) {
    GCU_Hash64_Value source_value = gcu_hash64_get(source, target_iterator.hash);
    if (source_value.exists) {
      if (target_iterator.value.p != source_value.value.p) {
        gcu_hash64_remove(target, target_iterator.hash);
      }
    }
    else {
      gcu_hash64_remove(target, target_iterator.hash);
    }
    target_iterator = gcu_hash64_iterator_next(target_iterator);
  }
}
