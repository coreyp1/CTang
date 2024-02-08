
#include <cutil/memory.h>
#include "tang/program.h"
#include "tang/tangLanguage.h"

GTA_Program * gta_program_create(const char * code) {
  GTA_Program * program = gcu_malloc(sizeof(GTA_Program));
  if (!program) {
    return 0;
  }

  if (!gta_program_create_in_place(program, code)) {
    gcu_free(program);
    return 0;
  }
  return program;
}

bool gta_program_create_in_place(GTA_Program * program, const char * code) {
  return gta_program_create_in_place_with_flags(program, code, GTA_PROGRAM_FLAG_DEFAULT);
}

GTA_Program * gta_program_create_with_flags(const char * code, uint64_t flags) {
  GTA_Program * program = gcu_malloc(sizeof(GTA_Program));
  if (!program) {
    return 0;
  }

  if (!gta_program_create_in_place_with_flags(program, code, flags)) {
    gcu_free(program);
    return 0;
  }
  return program;
}

bool gta_program_create_in_place_with_flags(GTA_Program * program, const char * code, uint64_t flags) {
  GTA_Ast_Node * ast = gta_tang_parse(code);
  *program = (GTA_Program) {
    .code = code,
    .ast = ast,
    .bytecode = 0,
    .binary = 0,
    .flags = flags,
  };
  return true;
}

void gta_program_destroy(GTA_Program * self) {
  gta_program_destroy_in_place(self);
  gcu_free(self);
}

void gta_program_destroy_in_place(GTA_Program * self) {
  if (self->ast) {
    gta_ast_node_destroy(self->ast);
  }
  if (self->bytecode) {
    gcu_free(self->bytecode);
  }
  if (self->binary) {
    gcu_free(self->binary);
  }
}

bool gta_program_execute(GTA_Program * program, GTA_Context * context) {
  if (!gta_context_create_in_place(context, program)) {
    return false;
  }
  if (program->flags & GTA_PROGRAM_FLAG_FORCE_BINARY) {
    if (!program->binary) {
      return false;
    }
    return gta_program_execute_binary(program, context);
  } else if (program->flags & GTA_PROGRAM_FLAG_FORCE_BYTECODE) {
    if (!program->bytecode) {
      return false;
    }
    return gta_program_execute_bytecode(program, context);
  } else if (program->binary) {
    return gta_program_execute_binary(program, context);
  } else if (program->bytecode) {
    return gta_program_execute_bytecode(program, context);
  }
  // TODO: Do some sort of error message.
  return false;
}

bool gta_program_execute_bytecode(GTA_MAYBE_UNUSED(GTA_Program * program), GTA_MAYBE_UNUSED(GTA_Context * context)) {
  return false;
}

bool gta_program_execute_binary(GTA_MAYBE_UNUSED(GTA_Program * program), GTA_MAYBE_UNUSED(GTA_Context * context)) {
  return false;
}

