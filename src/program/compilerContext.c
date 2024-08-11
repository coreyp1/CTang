
#include <assert.h>
#include <cutil/memory.h>
#include <tang/program/compilerContext.h>

GTA_Compiler_Context * gta_compiler_context_create(GTA_Program * program) {
  GTA_Compiler_Context * context = gcu_malloc(sizeof(GTA_Compiler_Context));
  if (!context) {
    return 0;
  }
  if (!gta_compiler_context_create_in_place(context, program)) {
    gcu_free(context);
    return 0;
  }
  return context;
}


bool gta_compiler_context_create_in_place(GTA_Compiler_Context * context, GTA_Program * program) {
  GTA_VectorX * bytecode_offsets = GTA_VECTORX_CREATE(32);
  if (!bytecode_offsets) {
    return false;
  }
  GCU_Vector8 * binary_vector = gcu_vector8_create(1024);
  if (!binary_vector) {
    goto BINARY_VECTOR_CREATE_FAILED;
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
  assert(globals->entries == 0);
  if (!globals) {
    goto GLOBALS_HASH_CREATE_FAILED;
  }
  GTA_VectorX * labels_from = GTA_VECTORX_CREATE(32);
  if (!labels_from) {
    goto LABELS_FROM_VECTOR_CREATE_FAILED;
  }
  assert(labels_from->count == 0);
  assert(labels_from->capacity == 32);
  GTA_VectorX * labels = GTA_VECTORX_CREATE(32);
  if (!labels) {
    goto LABELS_VECTOR_CREATE_FAILED;
  }
  assert(labels->count == 0);
  assert(labels->capacity == 32);

  assert(context);
  *context = (GTA_Compiler_Context) {
    .program = program,
    .binary_vector = binary_vector,
    .binary = 0,
    .bytecode_offsets = bytecode_offsets,
    .stack_depth = 0,
    .scope_stack = scope_stack,
    .globals = globals,
    .labels_from = labels_from,
    .labels = labels,
    .break_label = 0,
    .continue_label = 0,
    .return_label = 0,
  };

  // Label creation will not fail because the label vector was created with
  // a capacity of 32.
  // NOTE: These cannot be put into the declaration above because the context
  // is not fully initialized until this point.
  context->break_label = gta_compiler_context_get_label(context);
  context->continue_label = gta_compiler_context_get_label(context);
  context->return_label = gta_compiler_context_get_label(context);

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
  gcu_vector8_destroy(binary_vector);
BINARY_VECTOR_CREATE_FAILED:
  GTA_VECTORX_DESTROY(bytecode_offsets);
  return false;
}


void gta_compiler_context_destroy(GTA_Compiler_Context * context) {
  assert(context);
  gta_compiler_context_destroy_in_place(context);
  gcu_free(context);
}


void gta_compiler_context_destroy_in_place(GTA_Compiler_Context * context) {
  assert(context);
  GTA_VECTORX_DESTROY(context->bytecode_offsets);
  gcu_vector8_destroy(context->binary_vector);

  assert(context->scope_stack);
  for (size_t i = 0; i < context->scope_stack->count; ++i) {
    GTA_HASHX_DESTROY(GTA_TYPEX_P(context->scope_stack->data[i]));
  }
  GTA_VECTORX_DESTROY(context->scope_stack);
  GTA_HASHX_DESTROY(context->globals);

  assert(context->labels_from);
  for (size_t i = 0; i < context->labels_from->count; ++i) {
    GTA_VECTORX_DESTROY(GTA_TYPEX_P(context->labels_from->data[i]));
  }
  GTA_VECTORX_DESTROY(context->labels_from);
  GTA_VECTORX_DESTROY(context->labels);
}


GTA_NO_DISCARD GTA_Integer gta_compiler_context_get_label(GTA_Compiler_Context * context) {
  assert(context);
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


bool gta_compiler_context_add_label_jump(GTA_Compiler_Context * context, GTA_Integer label, GTA_Integer byte_offset) {
  assert(context);
  assert(context->labels_from);
  assert((label >= 0) && ((GTA_UInteger)label < context->labels_from->count));
  assert(context->labels_from->data);
  GTA_VectorX * labels_from_vector = GTA_TYPEX_P(context->labels_from->data[label]);
  if (!GTA_VECTORX_APPEND(labels_from_vector, GTA_TYPEX_MAKE_UI(byte_offset))) {
    return false;
  }
  return true;
}


bool gta_compiler_context_set_label(GTA_Compiler_Context * context, GTA_Integer label, GTA_Integer byte_offset) {
  assert(context);
  assert(context->labels);
  if (label < 0 || (GTA_UInteger)label >= context->labels->count) {
    return false;
  }
  assert(context->labels->data);
  context->labels->data[label] = GTA_TYPEX_MAKE_UI(byte_offset);
  return true;
}
