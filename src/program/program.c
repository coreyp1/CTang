
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
#include <tang/ast/astNodeAll.h>
#include <tang/computedValue/computedValue.h>
#include <tang/program/binaryCompilerContext.h>
#include <tang/program/executionContext.h>
#include <tang/program/program.h>
#include <tang/program/variable.h>
#include <tang/program/virtualMachine.h>
#include <tang/tangLanguage.h>

static void gta_program_compile_bytecode(GTA_Program * program) {
  GTA_VectorX * bytecode = GTA_VECTORX_CREATE(0);
  bool no_memory_error = true;
  if (bytecode) {
    GTA_Bytecode_Compiler_Context context;
    if (!gta_bytecode_compiler_context_create_in_place(&context, program)) {
      GTA_VECTORX_DESTROY(bytecode);
      bytecode = 0;
    }
    else {
      program->bytecode = bytecode;

      // Push the globals onto the stack in the correct order.
      // First, we must determine the order that globals should be pushed onto
      // the stack.
      GTA_VectorX * globals_order = GTA_VECTORX_CREATE(program->scope->global_positions->entries);
      if (!globals_order) {
        GTA_VECTORX_DESTROY(bytecode);
        program->bytecode = 0;
        gta_bytecode_compiler_context_destroy_in_place(&context);
        return;
      }
      globals_order->count = program->scope->global_positions->entries;
      GTA_HashX_Iterator iterator = GTA_HASHX_ITERATOR_GET(program->scope->global_positions);
      while (iterator.exists) {
        globals_order->data[GTA_TYPEX_UI(iterator.value)] = GTA_TYPEX_MAKE_UI(iterator.hash);
        iterator = GTA_HASHX_ITERATOR_NEXT(iterator);
      }
      // Now, loop through the globals in the correct order.  If it is a
      // library, then load the library.  Otherwise, push a NULL onto the
      // stack.
      // TODO: Handle Functions definitions.
      for (size_t i = 0; i < globals_order->count; ++i) {
        GTA_HashX_Value value = GTA_HASHX_GET(program->scope->identified_variables, GTA_TYPEX_UI(globals_order->data[i]));
        if (!value.exists) {
          GTA_VECTORX_DESTROY(bytecode);
          program->bytecode = 0;
          GTA_VECTORX_DESTROY(globals_order);
          gta_bytecode_compiler_context_destroy_in_place(&context);
          return;
        }
        GTA_Ast_Node_Identifier * identifier = value.value.p;
        if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY) {
          // Find the library AST and compile it.
          GTA_HashX_Value use_node = GTA_HASHX_GET(program->scope->library_declarations, identifier->mangled_name_hash);
          if (use_node.exists && GTA_AST_IS_USE(use_node.value.p)) {
            if (!gta_ast_node_compile_to_bytecode((GTA_Ast_Node *)use_node.value.p, &context)) {
              GTA_VECTORX_DESTROY(bytecode);
              program->bytecode = 0;
              GTA_VECTORX_DESTROY(globals_order);
              gta_bytecode_compiler_context_destroy_in_place(&context);
              return;
            }
          }
          else {
            // Library not found, but it should exist.
            GTA_VECTORX_DESTROY(bytecode);
            program->bytecode = 0;
            GTA_VECTORX_DESTROY(globals_order);
            gta_bytecode_compiler_context_destroy_in_place(&context);
            return;
          }
        }
        else {
          // We don't know how to handle this type of global.
          GTA_VECTORX_DESTROY(bytecode);
          program->bytecode = 0;
          GTA_VECTORX_DESTROY(globals_order);
          gta_bytecode_compiler_context_destroy_in_place(&context);
          return;
        }
      }
      GTA_VECTORX_DESTROY(globals_order);

      if (!gta_ast_node_compile_to_bytecode(program->ast, &context)) {
        GTA_VECTORX_DESTROY(bytecode);
        program->bytecode = 0;
      }
      gta_bytecode_compiler_context_destroy_in_place(&context);
    }
    if (program->bytecode) {
      no_memory_error &= GTA_VECTORX_APPEND(program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_RETURN));
      // If there was a memory error, then the bytecode is not valid.
      if (!no_memory_error) {
        GTA_VECTORX_DESTROY(program->bytecode);
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
  if (!gcu_vector8_reserve(context->binary_vector, 1024)) {
    gta_binary_compiler_context_destroy(context);
    return;
  }

  // Create a variable scope and collect the variable names.
  // All identifiers need to be found so that space can be reserved for
  // them on the stack when the program is executed.
  if (!gta_program_create_scope(context->scope_stack, context->globals, program->ast)) {
    gta_binary_compiler_context_destroy(context);
    return;
  }
  size_t local_orders_count = GTA_HASHX_COUNT(program->scope->local_positions);

#ifdef GTA_X86_64
  // 64-bit x86
  // https://defuse.ca/online-x86-assembler.htm
  // Callee-saved registers: rbp, rbx, r12, r13, r14, r15
  // Caller-saved registers: rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11
  // __cdecl argument registers: rdi, rsi, rdx, rcx, r8, r9
  // __cdecl return register: rax
  // __cdecl stack: rbp
  // Function will be called with arguments:
  //   (GTA_Binary_Execution_Context * context)
  // Setup will pre-populate registers with:
  //   r15 = context (rdi)
  //   r14 = &context->result
  //   r13 = global stack pointer
  //   r12 = frame (variable) stack pointer
  // Each execution will put the result in rax.  It is up to
  // the caller to move the result to the correct location.
  // Registers available for use:
  //   rbx (must save before use and restore)
  //   r10, r11 (may be clobbered by function calls)
  //   rdi, rsi, rdx, rcx, r8, r9 (function arguments, may be clobbered by function calls)
  //   rax (return value, will be clobbered by function calls)
  //
  // Procedure for calling a function:
  //   1. Save caller-saved registers (if needed)
  //      rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11
  //   2. Move arguments into correct registers
  //   3. Call function
  //        push rbp
  //        mov rbp, rsp
  //        and rsp, 0xFFFFFFFFFFFFFFF0
  //        call function_pointer
  //        mov rsp, rbp
  //        pop rbp
  //   4. Restore caller-saved registers

  // Reserve space for the binary.
  if (!gcu_vector8_reserve(context->binary_vector, 2048)) {
    gta_binary_compiler_context_destroy(context);
    return;
  }

  // Set up the beginning of the function:
  //   push rbp
  //   mov rbp, rsp
  GTA_BINARY_WRITE1(context->binary_vector, 0x55);
  GTA_BINARY_WRITE3(context->binary_vector, 0x48, 0x89, 0xE5);
  // Push callee-saved registers onto the stack.
  //   push r15
  //   push r14
  //   push r13
  //   push r12
  GTA_BINARY_WRITE2(context->binary_vector, 0x41, 0x57);
  GTA_BINARY_WRITE2(context->binary_vector, 0x41, 0x56);
  GTA_BINARY_WRITE2(context->binary_vector, 0x41, 0x55);
  GTA_BINARY_WRITE2(context->binary_vector, 0x41, 0x54);
  //   mov r15, rdi          ; Store context in r15.
  GTA_BINARY_WRITE3(context->binary_vector, 0x49, 0x89, 0xFF);
  //   lea r14, [r15 + offsetof(GTA_Binary_Execution_Context, result)]
  GTA_BINARY_WRITE3(context->binary_vector, 0x4D, 0x8D, 0x77);
  GTA_BINARY_WRITE1(context->binary_vector, (uint8_t)(size_t)(&((GTA_Execution_Context *)0)->result));

  //   mov r13, rsp          ; Store the global stack pointer in r13.
  GTA_BINARY_WRITE3(context->binary_vector, 0x49, 0x89, 0xE5);

  /////////////////////////////////////////////////////////////////////////////
  // Push the globals onto the stack in the correct order.
  // First, we must determine the order that globals should be pushed onto
  // the stack.
  size_t global_orders_count = program->scope->global_positions->entries;
  GTA_UInteger * globals_order = gcu_calloc(sizeof(GTA_UInteger), global_orders_count);
  if (!globals_order) {
    gta_binary_compiler_context_destroy(context);
    return;
  }
  GTA_HashX_Iterator iterator = GTA_HASHX_ITERATOR_GET(program->scope->global_positions);
  while (iterator.exists) {
    globals_order[GTA_TYPEX_UI(iterator.value)] = iterator.hash;
    iterator = GTA_HASHX_ITERATOR_NEXT(iterator);
  }
  // Now, loop through the globals in the correct order.  If it is a
  // library, then load the library.  Otherwise, push a NULL onto the
  // stack.
  // TODO: Handle Functions definitions.
  for (size_t i = 0; i < global_orders_count; ++i) {
    GTA_HashX_Value value = GTA_HASHX_GET(program->scope->identified_variables, globals_order[i]);
    if (!value.exists) {
      gcu_free(globals_order);
      gta_binary_compiler_context_destroy(context);
      return;
    }
    GTA_Ast_Node_Identifier * identifier = value.value.p;
    if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY) {
      // Find the library AST and compile it.
      GTA_HashX_Value use_node = GTA_HASHX_GET(program->scope->library_declarations, identifier->mangled_name_hash);
      if (use_node.exists && GTA_AST_IS_USE(use_node.value.p)) {
        if (!gta_ast_node_compile_to_binary((GTA_Ast_Node *)use_node.value.p, context)) {
          gcu_free(globals_order);
          gta_binary_compiler_context_destroy(context);
          return;
        }
        // Compile was successful.
        // Result should be in RAX.
        if (!gcu_vector8_reserve(context->binary_vector, context->binary_vector->count + 1)) {
          gcu_free(globals_order);
          gta_binary_compiler_context_destroy(context);
          return;
        }
        // Push the result onto the stack.
        //   push rax
        GTA_BINARY_WRITE1(context->binary_vector, 0x50);
      }
      else {
        // Library not found, but it should exist.
        gcu_free(globals_order);
        gta_binary_compiler_context_destroy(context);
        return;
      }
    }
    else {
      // We don't know how to handle this type of global.
      // Initialize to null.
      if (!gcu_vector8_reserve(context->binary_vector, context->binary_vector->count + 11)) {
        gcu_free(globals_order);
        gta_binary_compiler_context_destroy(context);
        return;
      }
      // Put the memory location of the null value into rdx.
      //   mov rdx, 0xDEADBEEFDEADBEEF
      GTA_BINARY_WRITE2(context->binary_vector, 0x48, 0xBA);
      GTA_BINARY_WRITE8(context->binary_vector, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
      memcpy(&context->binary_vector->data[context->binary_vector->count - sizeof(gta_computed_value_null)], &gta_computed_value_null, sizeof(gta_computed_value_null));
      //   push rdx
      GTA_BINARY_WRITE1(context->binary_vector, 0x52);
    }
  }
  gcu_free(globals_order);
  // Done with globals.
  /////////////////////////////////////////////////////////////////////////////

  if (!gcu_vector8_reserve(context->binary_vector, context->binary_vector->count + 13 + (local_orders_count) + 8)) {
    gta_binary_compiler_context_destroy(context);
    return;
  }

  // Store the frame (local variable) stack pointer.
  //   mov r12, rsp
  GTA_BINARY_WRITE3(context->binary_vector, 0x49, 0x89, 0xE4);

  // Put the memory location of the null value into rdx.
  // (RDX may have been overwritten by a function call in the global
  // initializations.)
  //   mov rdx, 0xDEADBEEFDEADBEEF
  GTA_BINARY_WRITE2(context->binary_vector, 0x48, 0xBA);
  GTA_BINARY_WRITE8(context->binary_vector, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF);
  memcpy(&context->binary_vector->data[context->binary_vector->count - sizeof(gta_computed_value_null)], &gta_computed_value_null, sizeof(gta_computed_value_null));

  // Initialize all locals to null.
  for (size_t i = 0; i < local_orders_count; ++i) {
    //  push rdx
    GTA_BINARY_WRITE1(context->binary_vector, 0x52);
  }

  // Align the stack to 16 bytes.
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFFFFFFFFFF0
  GTA_BINARY_WRITE1(context->binary_vector, 0x55);
  GTA_BINARY_WRITE3(context->binary_vector, 0x48, 0x89, 0xE5);
  GTA_BINARY_WRITE4(context->binary_vector, 0x48, 0x83, 0xE4, 0xF0);


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
  if (!gta_ast_node_compile_to_binary(program->ast, context)) {
    gta_binary_compiler_context_destroy(context);
    return;
  }

#ifdef GTA_X86_64
  // 64-bit x86
  if (!gcu_vector8_reserve(context->binary_vector, context->binary_vector->count + 17)) {
    gta_binary_compiler_context_destroy(context);
    return;
  }

  // Restore to the potentially-unaligned state.
  //   mov rsp, rbp
  //   pop rbp
  GTA_BINARY_WRITE3(context->binary_vector, 0x48, 0x89, 0xEC);
  GTA_BINARY_WRITE1(context->binary_vector, 0x5D);

  // Restore the stack pointer to before we added the global and local
  // variables.  This is faster than popping the locals and globals off the
  // stack, and the garbage collector will clean up the memory.
  //   mov rsp, r13
  GTA_BINARY_WRITE3(context->binary_vector, 0x4C, 0x89, 0xEC);

  // Pop callee-saved registers off the stack.
  //   pop r12
  //   pop r13
  //   pop r14
  //   pop r15
  GTA_BINARY_WRITE2(context->binary_vector, 0x41, 0x5C);
  GTA_BINARY_WRITE2(context->binary_vector, 0x41, 0x5D);
  GTA_BINARY_WRITE2(context->binary_vector, 0x41, 0x5E);
  GTA_BINARY_WRITE2(context->binary_vector, 0x41, 0x5F);

  // Set up the end of the function:
  //   leave
  //   ret
  GTA_BINARY_WRITE1(context->binary_vector, 0xC9);
  GTA_BINARY_WRITE1(context->binary_vector, 0xC3);

  // Lastly, correct the JUMP instructions to point to the correct location in
  // the binary.
  if (context->labels->count != context->labels_from->count) {
    gta_binary_compiler_context_destroy(context);
    return;
  }
  for (size_t i = 0; i < context->labels->count; ++i) {
    GTA_Integer label = GTA_TYPEX_UI(context->labels->data[i]);
    GTA_VectorX * jumps = GTA_TYPEX_P(context->labels_from->data[i]);
    for (size_t j = 0; j < jumps->count; ++j) {
      GTA_Integer jump = GTA_TYPEX_UI(jumps->data[j]);
      if (jump < 0 || (GTA_UInteger)jump >= context->binary_vector->count) {
        gta_binary_compiler_context_destroy(context);
        return;
      }
      // Jump is relative to the next instruction and is 4 bytes long.
      int32_t offset = (int32_t)(label - jump - 4);
      memcpy(&context->binary_vector->data[jump], &offset, sizeof(offset));
    }
  }

#elif defined(GTA_X86)
  // 32-bit x86
  // Set up the end of the function:
  //   pop ebp
  //   ret
  no_memory_error
    && GTA_BINARY_WRITE1(context->binary_vector, 0x5D)
    && GTA_BINARY_WRITE1(context->binary_vector, 0xC3);
#endif

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
      // printf("\nProgram code:\n%s\n", program->code);
      // fwrite(context->binary_vector->data, 1, length, stderr);
    }
  }
#endif

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
    .scope = 0,
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

  // Create the scope structure.
  char * name = gcu_calloc(1, 1);
  if (!name) {
    gta_ast_node_destroy(program->ast);
    return false;
  }
  program->scope = gta_variable_scope_create(name, program->ast, 0);

  // Analyze the AST to collect constants and build scopes.
  GTA_Ast_Node * error = gta_ast_node_analyze(program->ast, program, program->scope);
  if (error) {
    gta_ast_node_destroy(error);
    gta_ast_node_destroy(program->ast);
    gta_variable_scope_destroy(program->scope);
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

  if (!program->bytecode && !program->binary) {
    gta_ast_node_destroy(program->ast);
    gta_variable_scope_destroy(program->scope);
    return false;
  }

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
    GTA_VECTORX_DESTROY(self->bytecode);
  }
  if (self->binary) {
#ifdef _WIN32
    VirtualFree(self->binary, 0, MEM_RELEASE);
#else
    size_t length = gcu_vector8_count(self->binary);
    munmap(self->binary, length);
#endif // _WIN32
  }
  gta_variable_scope_destroy(self->scope);
}

bool gta_program_execute(GTA_Execution_Context * context) {
  if (context->program->binary) {
    return gta_program_execute_binary(context);
  } else if (context->program->bytecode) {
    return gta_program_execute_bytecode(context);
  }
  // TODO: Do some sort of error message.
  return false;
}

bool gta_program_execute_bytecode(GTA_Execution_Context * context) {
  return gta_virtual_machine_execute_bytecode(context);
}

typedef union Function_Converter {
  GTA_Computed_Value * GTA_CALL (*f)(GTA_Execution_Context *);
  void * b;
} Function_Converter;

bool gta_program_execute_binary(GTA_Execution_Context * context) {
  if (context->program->binary) {
    context->result = (Function_Converter){.b = context->program->binary}.f(context);
    return true;
  }
  return false;
}

void gta_program_bytecode_print(GTA_Program * self) {
  gta_bytecode_print(self->bytecode);
}

/**
 * Helper type for collecting identifiers.
 */
typedef struct {
  GTA_VectorX * scope_stack;
  GTA_HashX * globals;
} GTA_Program_Collect_Identifiers_Data;

/**
 * Helper function for collecting identifiers.
 *
 * @param self The AST node being visited.
 * @param data The data for the collection.
 * @param return_value The return value for the action.
 */
static void __gta_program_collect_identifiers(GTA_Ast_Node * self, void * data, void * return_value) {
  GTA_Program_Collect_Identifiers_Data * collect_data = (GTA_Program_Collect_Identifiers_Data *)data;
  GTA_VectorX * scope_stack = collect_data->scope_stack;
  GTA_HashX * globals = collect_data->globals;
  GTA_HashX * scope = GTA_TYPEX_P(scope_stack->data[scope_stack->count - 1]);

  if (GTA_AST_IS_IDENTIFIER(self)) {
    GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *)self;
    if (!GTA_HASHX_CONTAINS(globals, identifier->hash) && !GTA_HASHX_CONTAINS(scope, identifier->hash)) {
      // Add the identifier to the scope.
      if (!GTA_HASHX_SET(scope, identifier->hash, GTA_TYPEX_MAKE_UI(scope->entries))) {
        *(bool *)return_value = false;
        return;
      }
    }
  }
  else if (GTA_AST_IS_GLOBAL(self)) {
    GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *)self;
    if (GTA_HASHX_CONTAINS(scope, ((GTA_Ast_Node_Identifier *)global->identifier)->hash)) {
      // Remove the identifier from the local scope.
      GTA_HASHX_REMOVE(scope, ((GTA_Ast_Node_Identifier *)global->identifier)->hash);
    }
    if (!GTA_HASHX_CONTAINS(globals, ((GTA_Ast_Node_Identifier *)global->identifier)->hash)) {
      // Add the identifier to the globals.
      if (!GTA_HASHX_SET(globals, ((GTA_Ast_Node_Identifier *)global->identifier)->hash, GTA_TYPEX_MAKE_UI(globals->entries))) {
        *(bool *)return_value = false;
        return;
      }
    }
  }
  // TODO: How do we handle function declarations?
}


bool gta_program_create_scope(GTA_VectorX * scope_stack, GTA_HashX * globals, GTA_Ast_Node * ast) {
  GTA_HashX * scope = GTA_HASHX_CREATE(32);
  if (!scope) {
    return false;
  }
  if (!GTA_VECTORX_APPEND(scope_stack, GTA_TYPEX_MAKE_P(scope))) {
    GTA_HASHX_DESTROY(scope);
    return false;
  }
  // TODO: First walk the AST and collect the libraries.
  // Walk the AST and collect all of the identifiers.
  bool success = true;
  GTA_Program_Collect_Identifiers_Data data = {
    .scope_stack = scope_stack,
    .globals = globals,
  };
  gta_ast_node_walk(ast, __gta_program_collect_identifiers, &data, &success);
  return success;
}

void gta_program_destroy_scope(GTA_VectorX * scope_stack) {
  if (scope_stack->count) {
    GTA_HASHX_DESTROY(GTA_TYPEX_P(scope_stack->data[scope_stack->count - 1]));
    --scope_stack->count;
  }
}


// static void __identify_globals(GTA_Ast_Node * self, void * data, void * return_value) {
//   GTA_HashX * globals = (GTA_HashX *)data;
//   if (GTA_AST_IS_GLOBAL(self)) {
//     GTA_Ast_Node_Global * global = (GTA_Ast_Node_Global *)self;
//     if (!GTA_HASHX_CONTAINS(globals, ((GTA_Ast_Node_Identifier *)global->identifier)->hash)) {
//       // Add the identifier to the globals.
//       if (!GTA_HASHX_SET(globals, ((GTA_Ast_Node_Identifier *)global->identifier)->hash, GTA_TYPEX_MAKE_UI(globals->entries))) {
//         *(bool *)return_value = false;
//       }
//     }
//   }
//   else if (GTA_AST_IS_USE(self)) {
//     GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
//     if (!GTA_HASHX_CONTAINS(globals, use->hash)) {
//       // Add the identifier to the globals.
//       if (!GTA_HASHX_SET(globals, use->hash, GTA_TYPEX_MAKE_UI(globals->entries))) {
//         *(bool *)return_value = false;
//       }
//     }
//   }
//   else if (GTA_AST_IS_)
// }
