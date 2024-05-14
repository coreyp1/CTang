
#include <cutil/memory.h>
#include <tang/program/binaryCompilerContext.h>

GTA_Binary_Compiler_Context * gta_binary_compiler_context_create(GTA_Program * program) {
  GTA_Binary_Compiler_Context * context = gcu_malloc(sizeof(GTA_Binary_Compiler_Context));
  if (!context) {
    return 0;
  }
  if (!gta_binary_compiler_context_create_in_place(context, program)) {
    gcu_free(context);
    return 0;
  }
  return context;
}


bool gta_binary_compiler_context_create_in_place(GTA_Binary_Compiler_Context * context, GTA_Program * program) {
  GCU_Vector8 * binary_vector = gcu_vector8_create(1024);
  if (!binary_vector) {
    return false;
  }
  GTA_VectorX * scope_stack = GTA_VECTORX_CREATE(32);
  if (!scope_stack) {
    return false;
  }
  GTA_HashX * scope = GTA_HASHX_CREATE(32);
  if (!scope) {
    GTA_VECTORX_DESTROY(scope_stack);
    return false;
  }
  GTA_VECTORX_APPEND(scope_stack, GTA_TYPEX_MAKE_P(scope));
  GTA_HashX * globals = GTA_HASHX_CREATE(32);
  if (!globals) {
    GTA_HASHX_DESTROY(scope);
    GTA_VECTORX_DESTROY(scope_stack);
    return false;
  }

  GTA_VectorX * labels_from = GTA_VECTORX_CREATE(32);
  if (!labels_from) {
    GTA_HASHX_DESTROY(globals);
    GTA_HASHX_DESTROY(scope);
    GTA_VECTORX_DESTROY(scope_stack);
    return false;
  }

  GTA_VectorX * labels = GTA_VECTORX_CREATE(32);
  if (!labels) {
    GTA_VECTORX_DESTROY(labels_from);
    GTA_HASHX_DESTROY(globals);
    GTA_HASHX_DESTROY(scope);
    GTA_VECTORX_DESTROY(scope_stack);
    return false;
  }

  *context = (GTA_Binary_Compiler_Context) {
    .program = program,
    .binary_vector = binary_vector,
    .binary = 0,
    .stack_depth = 0,
    .scope_stack = scope_stack,
    .globals = globals,
    .labels_from = labels_from,
    .labels = labels,
  };
  return true;
}


void gta_binary_compiler_context_destroy(GTA_Binary_Compiler_Context * context) {
  gta_binary_compiler_context_destroy_in_place(context);
  gcu_free(context);
}


void gta_binary_compiler_context_destroy_in_place(GTA_Binary_Compiler_Context * context) {
  gcu_vector8_destroy(context->binary_vector);
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


GTA_NO_DISCARD GTA_Integer gta_binary_compiler_context_get_label(GTA_Binary_Compiler_Context * context) {
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


bool gta_binary_compiler_context_add_label_jump(GTA_Binary_Compiler_Context * context, GTA_Integer label, GTA_Integer byte_offset) {
  GTA_VectorX * labels_from_vector = GTA_TYPEX_P(context->labels_from->data[label]);
  if (!GTA_VECTORX_APPEND(labels_from_vector, GTA_TYPEX_MAKE_UI(byte_offset))) {
    return false;
  }
  return true;
}


bool gta_binary_compiler_context_set_label(GTA_Binary_Compiler_Context * context, GTA_Integer label, GTA_Integer byte_offset) {
  if (label < 0 || (GTA_UInteger)label >= context->labels->count) {
    return false;
  }
  context->labels->data[label] = GTA_TYPEX_MAKE_UI(byte_offset);
  return true;
}
