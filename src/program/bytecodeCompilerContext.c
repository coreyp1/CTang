
#include <cutil/memory.h>
#include <tang/program/bytecodeCompilerContext.h>
#include <tang/program/program.h>
#include <tang/program/variable.h>

GTA_Bytecode_Compiler_Context * gta_bytecode_compiler_context_create(GTA_Program * program) {
  GTA_Bytecode_Compiler_Context * context = gcu_malloc(sizeof(GTA_Bytecode_Compiler_Context));
  if (!context) {
    return 0;
  }

  if (!gta_bytecode_compiler_context_create_in_place(context, program)) {
    gcu_free(context);
    return 0;
  }

  return context;
}


bool gta_bytecode_compiler_context_create_in_place(GTA_Bytecode_Compiler_Context * context, GTA_Program * program) {
  GTA_VectorX * bytecode_offsets = GTA_VECTORX_CREATE(32);
  if (!bytecode_offsets) {
    return false;
  }
  GTA_VectorX * scope_stack = GTA_VECTORX_CREATE(32);
  if (!scope_stack) {
    goto SCOPE_STACK_VECTOR_CREATE_FAILED;
  }
  GTA_HashX * scope = GTA_HASHX_CREATE(32);
  if (!scope) {
    goto SCOPE_HASH_CREATE_FAILED;
  }
  GTA_VECTORX_APPEND(scope_stack, GTA_TYPEX_MAKE_P(scope));
  GTA_HashX * globals = GTA_HASHX_CREATE(32);
  if (!globals) {
    goto GLOBALS_HASH_CREATE_FAILED;
  }
  GTA_VectorX * labels_from = GTA_VECTORX_CREATE(32);
  if (!labels_from) {
    goto LABELS_FROM_VECTOR_CREATE_FAILED;
  }
  GTA_VectorX * labels = GTA_VECTORX_CREATE(32);
  if (!labels) {
    goto LABELS_VECTOR_CREATE_FAILED;
  }

  *context = (GTA_Bytecode_Compiler_Context) {
    .program = program,
    .bytecode_offsets = bytecode_offsets,
    .scope_stack = scope_stack,
    .globals = globals,
    .labels_from = labels_from,
    .labels = labels,
  };
  return true;

  // Failure conditions.
LABELS_VECTOR_CREATE_FAILED:
  GTA_VECTORX_DESTROY(labels_from);
LABELS_FROM_VECTOR_CREATE_FAILED:
  GTA_HASHX_DESTROY(globals);
GLOBALS_HASH_CREATE_FAILED:
  GTA_HASHX_DESTROY(scope);
SCOPE_HASH_CREATE_FAILED:
  GTA_VECTORX_DESTROY(scope_stack);
SCOPE_STACK_VECTOR_CREATE_FAILED:
  GTA_VECTORX_DESTROY(bytecode_offsets);
  return false;
}


void gta_bytecode_compiler_context_destroy(GTA_Bytecode_Compiler_Context * context) {
  gta_bytecode_compiler_context_destroy_in_place(context);
  gcu_free(context);
}


void gta_bytecode_compiler_context_destroy_in_place(GTA_Bytecode_Compiler_Context * context) {
  GTA_VECTORX_DESTROY(context->bytecode_offsets);
  for (size_t i = 0; i < context->scope_stack->count; ++i) {
    GTA_HASHX_DESTROY(GTA_TYPEX_P(context->scope_stack->data[i]));
  }
  GTA_VECTORX_DESTROY(context->scope_stack);
  GTA_HASHX_DESTROY(context->globals);
  for (size_t i = 0; i < context->labels_from->count; ++i) {
    GTA_VECTORX_DESTROY(GTA_TYPEX_P(context->labels_from->data[i]));
  }
  GTA_VECTORX_DESTROY(context->labels_from);
  GTA_VECTORX_DESTROY(context->labels);
}


GTA_NO_DISCARD GTA_Integer gta_bytecode_compiler_context_get_label(GTA_Bytecode_Compiler_Context * context) {
  GTA_VectorX * labels_from_vector = GTA_VECTORX_CREATE(32);
  if (!labels_from_vector) {
    return -1;
  }
  if (!GTA_VECTORX_APPEND(context->labels_from, GTA_TYPEX_MAKE_P(labels_from_vector))) {
    GTA_VECTORX_DESTROY(labels_from_vector);
    return -1;
  }
  if (!GTA_VECTORX_APPEND(context->labels, GTA_TYPEX_MAKE_UI(0))) {
    return -1;
  }
  return context->labels->count - 1;
}


bool gta_bytecode_compiler_context_add_label_jump(GTA_Bytecode_Compiler_Context * context, GTA_Integer label, GTA_Integer from_offset) {
  GTA_VectorX * labels_from_vector = GTA_TYPEX_P(context->labels_from->data[label]);
  if (!GTA_VECTORX_APPEND(labels_from_vector, GTA_TYPEX_MAKE_UI(from_offset))) {
    return false;
  }
  return true;
}


bool gta_bytecode_compiler_context_set_label(GTA_Bytecode_Compiler_Context * context, GTA_Integer label, GTA_Integer offset) {
  if (label < 0 || (GTA_UInteger)label >= context->labels->count) {
    return false;
  }
  context->labels->data[label] = GTA_TYPEX_MAKE_UI(offset);
  return true;
}
