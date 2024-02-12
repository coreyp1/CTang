
// Include the correct header file for the platform.
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <cutil/memory.h>
#include "tang/program.h"
#include "tang/tangLanguage.h"
#include "tang/astNodeBlock.h"
#include "tang/astNodeParseError.h"
#include "tang/virtualMachine.h"
#include "tang/binaryCompilerContext.h"

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
  GTA_Binary_Compiler_Context * context = gta_binary_compiler_context_create(program);
  if (!context) {
    return;
  }

  bool no_memory_error = true;
#ifdef GTA_X86_64
  // 64-bit x86
  // https://defuse.ca/online-x86-assembler.htm
  // Set up the beginning of the function:
  //   push rbp
  //   mov rbp, rsp
  no_memory_error
    &= GTA_BINARY_WRITE1(context->binary_vector, 0x55)
    && GTA_BINARY_WRITE3(context->binary_vector, 0x48, 0x89, 0xE5);

#elif defined(GTA_X86)
  // 32-bit x86
  // Set up the beginning of the function:
  //   push ebp
  //   mov ebp, esp
  no_memory_error
    &= GTA_BINARY_WRITE1(context->binary_vector, 0x55)
    && GTA_BINARY_WRITE2(context->binary_vector, 0x89, 0xE5);

#else
  // Not supported.
  gta_binary_compiler_context_destroy(context);
  return;
#endif

  // Actually compile the AST to binary.
  no_memory_error &= gta_ast_node_compile_to_binary(program->ast, context);

#ifdef GTA_X86_64
  // 64-bit x86
  // Set up the end of the function:
  //   add rsp, 8
  //   ret
  no_memory_error
    &= GTA_BINARY_WRITE4(context->binary_vector, 0x48, 0x83, 0xC4, 0x08)
    && GTA_BINARY_WRITE1(context->binary_vector, 0xC3);
#elif defined(GTA_X86)
  // 32-bit x86
  // Set up the end of the function:
  //   pop ebp
  //   ret
  no_memory_error
    && GTA_BINARY_WRITE1(context->binary_vector, 0x5D)
    && GTA_BINARY_WRITE1(context->binary_vector, 0xC3);
#endif

  if (!no_memory_error) {
    gta_binary_compiler_context_destroy(context);
    return;
  }

  // Copy the binary to executable memory.
  size_t length = gcu_vector8_count(context->binary_vector);
#ifdef _WIN32
  program->binary = VirtualAlloc(0, length, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (program->binary) {
    memcpy(program->binary, context->binary_vector->data, length);
  }
#else
#if !defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
  // neither MAP_ANONYMOUS nor MAP_ANON are defined
  // so we must use a file descriptor
  int fd = open("/dev/zero", O_RDWR);
  program->binary = mmap(0, length, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  close(fd);
#else
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
  program->binary = mmap(0, length, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif // !defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
  if (program->binary != MAP_FAILED) {
    memcpy(program->binary, context->binary_vector->data, length);
    if (mprotect(program->binary, length, PROT_EXEC | PROT_READ) != 0) {
      munmap(program->binary, length);
      program->binary = 0;
    }
    else {
      // dump the binary to stderr
      fprintf(stderr, "Binary: %p\n", program->binary);
      fwrite(context->binary_vector->data, 1, length, stderr);
      fprintf(stderr, "\n");
    }
  }
#endif
  // Correct JUMP instructions to point to the correct location in the binary.
  // TODO: implement this.


  gta_binary_compiler_context_destroy(context);
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
    gcu_vector64_destroy(self->bytecode);
  }
  if (self->binary) {
#ifdef _WIN32
    VirtualFree(self->binary, 0, MEM_RELEASE);
#else
    size_t length = gcu_vector8_count(self->binary);
    munmap(self->binary, length);
#endif // _WIN32
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

typedef union Function_Converter {
  GTA_Computed_Value * GTA_CALL (*f)(void);
  void * b;
} Function_Converter;

bool gta_program_execute_binary(GTA_MAYBE_UNUSED(GTA_Program * program), GTA_MAYBE_UNUSED(GTA_Context * context)) {
  if (program->binary) {
    context->result = (Function_Converter){.b = program->binary}.f();
    return true;
  }
  return false;
}

void gta_program_bytecode_print(GTA_Program * self) {
  gta_bytecode_print(self->bytecode);
}
