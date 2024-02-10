
#include <stdlib.h>
#include <cutil/memory.h>
#include "tang/program.h"
#include "tang/tangLanguage.h"
#include "tang/astNodeBlock.h"
#include "tang/astNodeParseError.h"
#include "tang/virtualMachine.h"

static void gta_program_compile_bytecode(GTA_Program * program) {
  GCU_Vector64 * bytecode = gcu_vector64_create(0);
  bool no_memory_error = true;
  if (bytecode) {
    GTA_Bytecode_Compiler_Context context;
    if (!gta_bytecode_compiler_context_create_in_place(&context, program)) {
      gcu_vector64_destroy(bytecode);
      bytecode = 0;
    }
    else {
      program->bytecode = bytecode;
      if (!gta_ast_node_compile_to_bytecode(program->ast, &context)) {
        gcu_vector64_destroy(bytecode);
        program->bytecode = 0;
      }
      gta_bytecode_compiler_context_destroy_in_place(&context);
    }
    if (program->bytecode) {
      size_t length = gcu_vector64_count(program->bytecode);

      // Make sure that the bytecode terminates with a RETURN instruction.
      // Replace an ending POP instruction with a RETURN instruction.
      if (program->bytecode->data[length - 1].ui64 == GTA_BYTECODE_POP) {
        program->bytecode->data[length - 1].ui64 = GTA_BYTECODE_RETURN;
      }

      // Add a NULL and RETURN instruction, in case there are any jumps to the
      // end of the bytecode.
      // If the AST is simply an expression, then the NULL is not necessary.
      if (GTA_AST_IS_BLOCK(program->ast)) {
        no_memory_error &= gcu_vector64_append(program->bytecode, GCU_TYPE64_UI64(GTA_BYTECODE_NULL));
      }
      no_memory_error &= gcu_vector64_append(program->bytecode, GCU_TYPE64_UI64(GTA_BYTECODE_RETURN));
      // If there was a memory error, then the bytecode is not valid.
      if (!no_memory_error) {
        gcu_vector64_destroy(program->bytecode);
        program->bytecode = 0;
      }
    }
  }
}

static void gta_program_compile_binary(GTA_Program * program) {
  (void) program;
}

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
  // Initialize flags based on environment variables.
  GTA_Program_Flags flags = GTA_PROGRAM_FLAG_DEFAULT;
  if (getenv("TANG_DEBUG")) {
    flags |= GTA_PROGRAM_FLAG_DEBUG;
  }
  if (getenv("TANG_DISABLE_BYTECODE")) {
    flags |= GTA_PROGRAM_FLAG_DISABLE_BYTECODE;
  }
  if (getenv("TANG_DISABLE_BINARY")) {
    flags |= GTA_PROGRAM_FLAG_DISABLE_BINARY;
  }
  return gta_program_create_in_place_with_flags(program, code, flags);
}

GTA_Program * gta_program_create_with_flags(const char * code, GTA_Program_Flags flags) {
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

bool gta_program_create_in_place_with_flags(GTA_Program * program, const char * code, GTA_Program_Flags flags) {
  // Initialize the program data structure.
  *program = (GTA_Program) {
    .code = code,
    .ast = 0,
    .bytecode = 0,
    .binary = 0,
    .flags = flags,
  };

  // Either parse the code into an AST or create a null AST.
  program->ast = gta_tang_parse(code);
  if (!program->ast) {
    program->ast = gta_ast_node_create((GTA_PARSER_LTYPE) {
      .first_line = 0,
      .first_column = 0,
      .last_line = 0,
      .last_column = 0,
    });
  }
  else if (GTA_AST_IS_PARSE_ERROR(program->ast)) {
    // If the AST is a parse error, then the program is not valid.
    gta_ast_node_destroy(program->ast);
    program->ast = 0;
    return false;
  }
  if (!program->ast) {
    // If there is no AST, then there is no program.
    return false;
  }

  // If the program can compile to binary, then there is no need to compile to
  // bytecode.
  if (!(flags & GTA_PROGRAM_FLAG_DISABLE_BINARY)) {
    gta_program_compile_binary(program);
  }
  if (!program->binary && !(flags & GTA_PROGRAM_FLAG_DISABLE_BYTECODE)) {
    gta_program_compile_bytecode(program);
  }

  return program->bytecode || program->binary;
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
  if (program->binary) {
    return gta_program_execute_binary(program, context);
  } else if (program->bytecode) {
    return gta_program_execute_bytecode(program, context);
  }
  // TODO: Do some sort of error message.
  return false;
}

bool gta_program_execute_bytecode(GTA_MAYBE_UNUSED(GTA_Program * program), GTA_MAYBE_UNUSED(GTA_Context * context)) {
  return gta_virtual_machine_execute_bytecode(context, program);
}

bool gta_program_execute_binary(GTA_MAYBE_UNUSED(GTA_Program * program), GTA_MAYBE_UNUSED(GTA_Context * context)) {
  return false;
}

void gta_program_bytecode_print(GTA_Program * self) {
  gta_bytecode_print(self->bytecode);
}
