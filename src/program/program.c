
// Include the correct header file for the platform.
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeAll.h>
#include <tang/computedValue/computedValueAll.h>
#include <tang/program/compilerContext.h>
#include <tang/program/executionContext.h>
#include <tang/program/binary.h>
#include <tang/program/program.h>
#include <tang/program/variable.h>
#include <tang/program/virtualMachine.h>
#include <tang/tangLanguage.h>


/**
 * Helper function to clean up the type/attribute -> callback hash.
 *
 * @param hash The hash to clean up.
 */
static void computed_value_attribute_hash_cleanup(GTA_HashX * hash);


/**
 * Helper function to clean up the type/value_hash -> singleton hash.
 *
 * This is for the first dimension of the hash.
 *
 * @param hash The hash to clean up.
 */
static void computed_value_singleton_hash_cleanup_1(GTA_HashX * hash);


/**
 * Helper function to clean up the type/value_hash -> singleton hash.
 *
 * This is for the second dimension of the hash.
 *
 * @param hash The hash to clean up.
 */
static void computed_value_singleton_hash_cleanup_2(GTA_HashX * hash);


static void gta_program_compile_bytecode(GTA_Program * program) {
  assert(program);

  GTA_VectorX * bytecode = GTA_VECTORX_CREATE(0);
  if (!bytecode) {
    goto BYTECODE_SET_NULL;
  }

  bool error_free = true;
  GTA_Compiler_Context context;
  if (!gta_compiler_context_create_in_place(&context, program)) {
    goto BYTECODE_VECTOR_CLEANUP;
  }
  program->bytecode = bytecode;

  // Push the variables onto the stack in the correct order.
  GTA_VectorX * variables_order = GTA_VECTORX_CREATE(program->scope->variable_positions->entries);
  if (!variables_order) {
    goto BYTECODE_COMPILER_CONTEXT_CLEANUP;
  }
  variables_order->count = program->scope->variable_positions->entries;
  GTA_HashX_Iterator iterator = GTA_HASHX_ITERATOR_GET(program->scope->variable_positions);
  while (iterator.exists) {
    variables_order->data[GTA_TYPEX_UI(iterator.value)] = GTA_TYPEX_MAKE_UI(iterator.hash);
    iterator = GTA_HASHX_ITERATOR_NEXT(iterator);
  }
  // Now, loop through the globals in the correct order.  If it is a
  // library, then load the library.  Otherwise, push a NULL onto the
  // stack.
  for (size_t i = 0; i < variables_order->count; ++i) {
    GTA_HashX_Value value = GTA_HASHX_GET(program->scope->identified_variables, GTA_TYPEX_UI(variables_order->data[i]));
    if (!value.exists) {
      goto BYTECODE_DESTROY_GLOBALS_ORDER;
    }
    if (GTA_AST_IS_IDENTIFIER(value.value.p)) {
      GTA_Ast_Node_Identifier * identifier = value.value.p;
      if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY) {
        // Find the library AST and compile it.
        GTA_HashX_Value use_node = GTA_HASHX_GET(program->scope->library_declarations, identifier->mangled_name_hash);
        if (use_node.exists && GTA_AST_IS_USE(use_node.value.p)) {
          if (!gta_ast_node_compile_to_bytecode((GTA_Ast_Node *)use_node.value.p, &context)) {
            goto BYTECODE_DESTROY_GLOBALS_ORDER;
          }
        }
        else {
          // Library not found, but it should exist.
          goto BYTECODE_DESTROY_GLOBALS_ORDER;
        }
      }
      else if ((identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL) || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL)) {
        // Put the memory location of the null value onto the stack.
        error_free &= GTA_VECTORX_APPEND(bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_NULL));
      }
      else {
        // We don't know how to handle this type of global.
        goto BYTECODE_DESTROY_GLOBALS_ORDER;
      }
    }
    else if (GTA_AST_IS_FUNCTION(value.value.p)) {
      GTA_Ast_Node_Function * function = value.value.p;
      error_free &= true
        && GTA_VECTORX_APPEND(bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_LOAD))
        && GTA_VECTORX_APPEND(bytecode, GTA_TYPEX_MAKE_P(function->runtime_function));
    }
    else {
      // We don't know how to handle this type of global.
      fprintf(stderr, "Unknown global type.\n");
      goto BYTECODE_DESTROY_GLOBALS_ORDER;
    }
    error_free &= GTA_VECTORX_APPEND(bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_SET_NOT_TEMP));
  }

  // Actually compile the AST to bytecode.
  error_free &= true
    && gta_ast_node_compile_to_bytecode(program->ast, &context)

  // At this point, a return value will be on top of the stack from either the
  // code block being executed, the 'break' statement, or the 'continue'
  // statement.  'Break' jumps here directly (with the appropriate value on
  // top of the stack).  'Continue' jumps here after putting the null computed
  // value on top of the stack.
    && gta_compiler_context_set_label(&context, context.break_label, bytecode->count)
    && gta_compiler_context_set_label(&context, context.return_label, bytecode->count)

  // Add the return instruction.
    && GTA_BYTECODE_APPEND(context.bytecode_offsets, bytecode->count)
    && GTA_VECTORX_APPEND(bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_RETURN))

  // Continue: It is possible that a continue statement was not within a loop.
  // We will catch this situation here and make sure that a null value is on
  // top of the stack and jump to the break label.
  //   NULL
  //   JMP break
    && gta_compiler_context_set_label(&context, context.continue_label, bytecode->count)
    && GTA_BYTECODE_APPEND(context.bytecode_offsets, bytecode->count)
    && GTA_VECTORX_APPEND(bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_NULL))
    && GTA_BYTECODE_APPEND(context.bytecode_offsets, bytecode->count)
    && GTA_VECTORX_APPEND(bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_JMP))
    && GTA_VECTORX_APPEND(bytecode, GTA_TYPEX_MAKE_UI(0))
    && gta_compiler_context_add_label_jump(&context, context.break_label, bytecode->count - 1)
  ;

  // Correct the JUMP instructions to point to the correct location in the
  // bytecode.  The jump targets are relative to the instruction after the
  // jump (because the PC will have already been incremented).
  for (size_t i = 0; error_free && (i < context.labels->count); ++i) {
    GTA_Integer label = GTA_TYPEX_UI(context.labels->data[i]);
    GTA_VectorX * jumps = GTA_TYPEX_P(context.labels_from->data[i]);
    for (size_t j = 0; j < jumps->count; ++j) {
      GTA_Integer jump = GTA_TYPEX_UI(jumps->data[j]);
      if (jump < 0 || (GTA_UInteger)jump >= bytecode->count) {
        goto BYTECODE_DESTROY_GLOBALS_ORDER;
      }
      // Jump is relative to the next instruction.
      int32_t offset = (int32_t)(label - jump - 1);
      bytecode->data[jump] = GTA_TYPEX_MAKE_UI(offset);
    }
  }

  // Cleanup and exit.
  GTA_VECTORX_DESTROY(variables_order);
  gta_compiler_context_destroy_in_place(&context);
  return;

  // Failure conditions.  Cleanup and exit.
BYTECODE_DESTROY_GLOBALS_ORDER:
  GTA_VECTORX_DESTROY(variables_order);
BYTECODE_COMPILER_CONTEXT_CLEANUP:
  gta_compiler_context_destroy_in_place(&context);
BYTECODE_VECTOR_CLEANUP:
  GTA_VECTORX_DESTROY(bytecode);
BYTECODE_SET_NULL:
  program->bytecode = 0;
  return;
}


GTA_Program * gta_program_create(GTA_Language * language, const char * code) {
  assert(code);

  GTA_Program * program = gcu_malloc(sizeof(GTA_Program));
  if (!program) {
    return 0;
  }

  if (!gta_program_create_in_place(program, language, code)) {
    gcu_free(program);
    return 0;
  }
  return program;
}


bool gta_program_create_in_place(GTA_Program * program, GTA_Language * language, const char * code) {
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
  return gta_program_create_in_place_with_flags(program, language, code, flags);
}


GTA_Program * gta_program_create_with_flags(GTA_Language * language, const char * code, GTA_Program_Flags flags) {
  GTA_Program * program = gcu_malloc(sizeof(GTA_Program));
  if (!program) {
    return 0;
  }

  if (!gta_program_create_in_place_with_flags(program, language, code, flags)) {
    gcu_free(program);
    return 0;
  }
  return program;
}


bool gta_program_create_in_place_with_flags(GTA_Program * program, GTA_Language * language, const char * code, GTA_Program_Flags flags) {
  assert(program);

  // Initialize the program data structure.
  *program = (GTA_Program) {
    .language = language,
    .library = 0,
    .code = code,
    .ast = 0,
    .bytecode = 0,
    .binary = 0,
    .flags = flags,
    .scope = 0,
    .singletons = 0,
    .attributes = 0,
  };

  // Create the library.
  program->library = gta_library_create();
  if (!program->library) {
    goto LIBRARY_CREATE_FAILURE;
  }

  // Create the attributes hash.
  program->attributes = GTA_HASHX_CREATE(32);
  if (!program->attributes) {
    goto ATTRIBUTE_HASH_CREATE_FAILURE;
  }
  program->attributes->cleanup = computed_value_attribute_hash_cleanup;

  GTA_Computed_Value_VTable * vtable[] = {
    &gta_computed_value_array_vtable,
    &gta_computed_value_boolean_vtable,
    &gta_computed_value_error_vtable,
    &gta_computed_value_float_vtable,
    &gta_computed_value_function_vtable,
    &gta_computed_value_function_native_vtable,
    &gta_computed_value_integer_vtable,
    &gta_computed_value_iterator_vtable,
    &gta_computed_value_map_vtable,
    &gta_computed_value_null_vtable,
    &gta_computed_value_rng_vtable,
    &gta_computed_value_string_vtable,
  };
  size_t vtable_count = sizeof(vtable) / sizeof(vtable[0]);
  for (size_t i = 0; i < vtable_count; ++i) {
    GTA_Computed_Value_Attribute_Pair * attributes = vtable[i]->attributes;
    size_t attributes_count = vtable[i]->attributes_count;
    for (size_t j = 0; j < attributes_count; ++j) {
      if (!gta_program_set_type_attribute(program, vtable[i], GTA_STRING_HASH(attributes[j].name, strlen(attributes[j].name)), attributes[j].callback)) {
        goto ATTRIBUTE_HASH_POPULATE_FAILURE;
      }
    }
  }

  // Allocate the singleton vector.
  program->singletons = GTA_HASHX_CREATE(32);
  if (!program->singletons) {
    goto SINGLETON_HASH_CREATE_FAILURE;
  }
  program->singletons->cleanup = computed_value_singleton_hash_cleanup_1;

  // Either parse the code into an AST or create a null AST.
  program->ast = program->flags & GTA_PROGRAM_FLAG_IS_TEMPLATE
    ? gta_tang_parse_template(code)
    : gta_tang_parse_script(code);
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
    goto PARSE_FAILURE;
  }
  if (!program->ast) {
    // If there is no AST, then there is no program.
    goto COMPLETE_FAILURE;
  }

  // Create the scope structure.
  char * name = gcu_calloc(1, 1);
  if (!name) {
    goto SCOPE_NAME_CREATION_FAILURE;
  }
  if (!(program->scope = gta_variable_scope_create(name, program->ast, 0))) {
    gcu_free(name);
    goto SCOPE_CREATION_FAILURE;
  }

  // Analyze the AST to collect constants and build scopes.
  GTA_Ast_Node * error;
  if ((error = gta_ast_node_analyze(program->ast, program, program->scope))) {
    gta_ast_node_destroy(error);
    goto ANALYZE_FAILURE;
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
    goto COMPILE_FAILURE;
  }

  return true;

  // Cleanup on failure.
COMPILE_FAILURE:
ANALYZE_FAILURE:
  gta_variable_scope_destroy(program->scope);
  program->scope = 0;
SCOPE_CREATION_FAILURE:
SCOPE_NAME_CREATION_FAILURE:
PARSE_FAILURE:
  gta_ast_node_destroy(program->ast);
  program->ast = 0;
COMPLETE_FAILURE:
  GTA_HASHX_DESTROY(program->singletons);
  program->singletons = 0;
SINGLETON_HASH_CREATE_FAILURE:
ATTRIBUTE_HASH_POPULATE_FAILURE:
  GTA_HASHX_DESTROY(program->attributes);
ATTRIBUTE_HASH_CREATE_FAILURE:
  gta_library_destroy(program->library);
  program->library = 0;
LIBRARY_CREATE_FAILURE:
  return false;
}


void gta_program_destroy(GTA_Program * self) {
  assert(self);
  gta_program_destroy_in_place(self);
  gcu_free(self);
}


void gta_program_destroy_in_place(GTA_Program * self) {
  assert(self);

  // Destroy the library.
  if (self->library) {
    gta_library_destroy(self->library);
  }

  // Destroy the AST.
  if (self->ast) {
    gta_ast_node_destroy(self->ast);
  }
  self->ast = 0;

  // Destroy the scope.
  gta_variable_scope_destroy(self->scope);
  self->scope = 0;

  // Destroy the attributes.
  assert(self->attributes);
  GTA_HASHX_DESTROY(self->attributes);

  // Destroy the singletons.
  assert(self->singletons);
  GTA_HASHX_DESTROY(self->singletons);
  self->singletons = 0;

  // Destroy the bytecode.
  if (self->bytecode) {
    GTA_VECTORX_DESTROY(self->bytecode);
  }
  self->bytecode = 0;

  // Destroy the binary.
  if (self->binary) {
#ifdef _WIN32
    VirtualFree(self->binary, 0, MEM_RELEASE);
#else
    size_t length = gcu_vector8_count(self->binary);
    munmap(self->binary, length);
#endif // _WIN32
  }
  self->binary = 0;
}


static void computed_value_attribute_hash_cleanup(GTA_HashX * hash) {
  assert(hash);
  GTA_HashX_Iterator iterator = GTA_HASHX_ITERATOR_GET(hash);
  while (iterator.exists) {
    GTA_HASHX_DESTROY(GTA_TYPEX_P(iterator.value));
    iterator = GTA_HASHX_ITERATOR_NEXT(iterator);
  }
}


GTA_Computed_Value_Attribute_Callback gta_program_get_type_attribute(GTA_Program * program, GTA_Computed_Value_VTable * type_vtable, GTA_UInteger identifier_hash) {
  assert(program);
  assert(program->attributes);
  GTA_HashX_Value value = GTA_HASHX_GET(program->attributes, (GTA_UInteger)type_vtable);
  if (value.exists) {
    GTA_HashX * type_hash = GTA_TYPEX_P(value.value);
    value = GTA_HASHX_GET(type_hash, identifier_hash);
    if (value.exists) {
      return (GTA_Computed_Value_Attribute_Callback)(GTA_TYPEX_UI(value.value));
    }
  }
  return NULL;
}


bool gta_program_set_type_attribute(GTA_Program * program, GTA_Computed_Value_VTable * type_vtable, GTA_UInteger identifier_hash, GTA_Computed_Value_Attribute_Callback callback) {
  assert(program);
  assert(program->attributes);
  GTA_HashX_Value type_value = GTA_HASHX_GET(program->attributes, (GTA_UInteger)type_vtable);
  GTA_HashX * attribute_hash;
  if (!type_value.exists) {
    attribute_hash = GTA_HASHX_CREATE(32);
    if (!attribute_hash) {
      return false;
    }
    if (!GTA_HASHX_SET(program->attributes, (GTA_UInteger)type_vtable, GTA_TYPEX_MAKE_P(attribute_hash))) {
      GTA_HASHX_DESTROY(attribute_hash);
      return false;
    }
  }
  else {
    attribute_hash = GTA_TYPEX_P(type_value.value);
  }
  return GTA_HASHX_SET(attribute_hash, identifier_hash, GTA_TYPEX_MAKE_UI(GTA_JIT_FUNCTION_CONVERTER(callback)));
}


static void computed_value_singleton_hash_cleanup_1(GTA_HashX * hash) {
  assert(hash);
  GTA_HashX_Iterator iterator = GTA_HASHX_ITERATOR_GET(hash);
  while (iterator.exists) {
    GTA_HASHX_DESTROY(GTA_TYPEX_P(iterator.value));
    iterator = GTA_HASHX_ITERATOR_NEXT(iterator);
  }
}


static void computed_value_singleton_hash_cleanup_2(GTA_HashX * hash) {
  assert(hash);
  GTA_HashX_Iterator iterator = GTA_HASHX_ITERATOR_GET(hash);
  while (iterator.exists) {
    gta_computed_value_destroy((GTA_Computed_Value *)GTA_TYPEX_P(iterator.value));
    iterator = GTA_HASHX_ITERATOR_NEXT(iterator);
  }
}


GTA_Computed_Value * gta_program_get_singleton(GTA_Program * program, GTA_Computed_Value_VTable * type_vtable, GTA_UInteger value_hash) {
  assert(program);
  assert(program->singletons);
  GTA_HashX_Value value = GTA_HASHX_GET(program->singletons, (GTA_UInteger)type_vtable);
  if (value.exists) {
    GTA_HashX * type_hash = GTA_TYPEX_P(value.value);
    value = GTA_HASHX_GET(type_hash, value_hash);
    if (value.exists) {
      return (GTA_Computed_Value *)GTA_TYPEX_P(value.value);
    }
  }
  return NULL;
}


bool gta_program_set_singleton(GTA_Program * program, GTA_Computed_Value_VTable * type_vtable, GTA_UInteger value_hash, GTA_Computed_Value * singleton) {
  assert(program);
  assert(program->singletons);
  GTA_HashX_Value type_value = GTA_HASHX_GET(program->singletons, (GTA_UInteger)type_vtable);
  GTA_HashX * values;
  if (!type_value.exists) {
    values = GTA_HASHX_CREATE(32);
    if (!values) {
      return false;
    }
    values->cleanup = computed_value_singleton_hash_cleanup_2;
    if (!GTA_HASHX_SET(program->singletons, (GTA_UInteger)type_vtable, GTA_TYPEX_MAKE_P(values))) {
      GTA_HASHX_DESTROY(values);
      return false;
    }
  }
  else {
    values = GTA_TYPEX_P(type_value.value);
  }
  singleton->is_singleton = true;
  singleton->is_temporary = false;
  return GTA_HASHX_SET(values, value_hash, GTA_TYPEX_MAKE_P(singleton));
}


bool gta_program_execute(GTA_Execution_Context * context) {
  assert(context);
  assert(context->program);

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
  assert(context);
  assert(context->program);
  if (context->program->binary) {
    context->result = (Function_Converter){.b = context->program->binary}.f(context);
    return true;
  }
  return false;
}


void gta_program_bytecode_print(GTA_Program * self) {
  assert(self);
  gta_bytecode_print(self->bytecode);
}


/**
 * Helper function used to update function pointers in the AST to point to the
 * actual address in the binary.  This function is called after the binary has
 * been compiled and the function pointers are still byte offsets into the
 * binary.
 *
 * @param self The AST node to update.
 * @param data In this case, the pointer to the binary block.
 * @param error Unused.
 */
static void update_function_pointers(GTA_Ast_Node * self, void * data, GTA_MAYBE_UNUSED(void * error)) {
  assert(self);
  if (GTA_AST_IS_FUNCTION(self)) {
    GTA_Ast_Node_Function * function = (GTA_Ast_Node_Function *)self;
    assert(function->runtime_function);
    function->runtime_function->pointer += (size_t)data;
  }
}


void gta_program_compile_binary__x86_64(GTA_Program * program) {
  assert(program);

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
  //        and rsp, 0xFFFFFFF0
  //        call function_pointer
  //        mov rsp, rbp
  //        pop rbp
  //   4. Restore caller-saved registers

  GTA_Compiler_Context * context = gta_compiler_context_create(program);
  if (!context) {
    return;
  }
  // Reserve space for the binary.
  if (!gcu_vector8_reserve(context->binary_vector, 2048)) {
    goto CONTEXT_CLEANUP;
  }

  GCU_Vector8 * v = context->binary_vector;

  assert(program->scope);
  size_t variable_positions_count = GTA_HASHX_COUNT(program->scope->variable_positions);
  bool error_free = true;

  error_free
  // Set up the beginning of the function:
  //   push rbp
  //   mov rbp, rsp
    &= gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)

  // Push callee-saved registers onto the stack.
  //   push r15
  //   push r14
  //   push r13
  //   push r12
  //   push rbx
    && gta_push_reg__x86_64(v, GTA_REG_R15)
    && gta_push_reg__x86_64(v, GTA_REG_R14)
    && gta_push_reg__x86_64(v, GTA_REG_R13)
    && gta_push_reg__x86_64(v, GTA_REG_R12)
    && gta_push_reg__x86_64(v, GTA_REG_RBX)

  //   mov r15, rdi          ; Store context in r15.
    && gta_mov_reg_reg__x86_64(v, GTA_REG_R15, GTA_REG_RDI)

  //   lea r14, [r15 + offsetof(GTA_Binary_Execution_Context, result)]
    && gta_lea_reg_ind__x86_64(v, GTA_REG_R14, GTA_REG_R15, GTA_REG_NONE, 0, (int32_t)(size_t)(&((GTA_Execution_Context *)0)->result))

  //   mov r13, rsp          ; Store the global stack pointer in r13.
  //   mov r12, rsp          ; Store the frame (local variable) stack pointer in r12.
    && gta_mov_reg_reg__x86_64(v, GTA_REG_R13, GTA_REG_RSP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_R12, GTA_REG_RSP)
  ;

  /////////////////////////////////////////////////////////////////////////////
  // Push the globals onto the stack in the correct order.
  // First, we must determine the order that globals should be pushed onto
  // the stack.
  GTA_UInteger * variables_order = gcu_calloc(sizeof(GTA_UInteger), variable_positions_count);
  if (!variables_order) {
    goto CONTEXT_CLEANUP;
  }
  GTA_HashX_Iterator iterator = GTA_HASHX_ITERATOR_GET(program->scope->variable_positions);
  while (iterator.exists) {
    variables_order[GTA_TYPEX_UI(iterator.value)] = iterator.hash;
    iterator = GTA_HASHX_ITERATOR_NEXT(iterator);
  }
  // Now, loop through the globals in the correct order.  If it is a
  // library, then load the library.  Otherwise, push a NULL onto the
  // stack.
  for (size_t i = 0; error_free && (i < variable_positions_count); ++i) {
    GTA_HashX_Value value = GTA_HASHX_GET(program->scope->identified_variables, variables_order[i]);
    if (!value.exists) {
      goto GLOBALS_ORDER_CLEANUP;
    }

    if (GTA_AST_IS_IDENTIFIER(value.value.p)) {
      GTA_Ast_Node_Identifier * identifier = value.value.p;
      assert(identifier);

      if (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY) {
        // Find the library AST and compile it.
        GTA_HashX_Value use_node = GTA_HASHX_GET(program->scope->library_declarations, identifier->mangled_name_hash);
        error_free
          &= use_node.exists
          && GTA_AST_IS_USE(use_node.value.p)
          // Compile the expression.  The result will be in RAX.
          && gta_ast_node_compile_to_binary__x86_64((GTA_Ast_Node *)use_node.value.p, context)
          // Push the result onto the stack.
          //   push rax
          && gta_push_reg__x86_64(v, GTA_REG_RAX);
      }
      else if ((identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL) || (identifier->type == GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL)) {
        // Put the memory location of the null value onto the stack.
        //   mov rax, gta_computed_value_null
        //   push rax
        error_free
          &= gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (uint64_t)gta_computed_value_null)
          && gta_push_reg__x86_64(v, GTA_REG_RAX);
      }
      else {
        // We don't know how to handle this type of global.
        // Put the memory location of the null value into rdx.
        //   mov rdx, gta_computed_value_null
        //   push rdx
        error_free
          &= gta_mov_reg_imm__x86_64(v, GTA_REG_RDX, (uint64_t)gta_computed_value_null)
          && gta_push_reg__x86_64(v, GTA_REG_RDX);
      }
    }
    else if (GTA_AST_IS_FUNCTION(value.value.p)) {
      GTA_Ast_Node_Function * function = value.value.p;
      assert(function);

      error_free &= true
      // Push the function onto the stack.
      //   mov rax, function->runtime_function
      //   push rax
        && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (uint64_t)function->runtime_function)
        && gta_push_reg__x86_64(v, GTA_REG_RAX);
    }
    else {
      // We don't know how to handle this type of global.
      fprintf(stderr, "Unknown global type.\n");
      error_free = false;
    }
  }
  gcu_free(variables_order);
  // Done with globals.
  /////////////////////////////////////////////////////////////////////////////


  // Actually compile the AST to binary.
  error_free &= true
    && gta_ast_node_compile_to_binary__x86_64(program->ast, context)

  // At this point, a return value will be in RAX from either the code block
  // being executed, the 'break' statement, or the 'continue' statement.
  // 'Break' jumps here directly (with the appropriate value in RAX).
  // 'Continue' jumps here after setting RAX to the null computed value.
  // 'Return' jumps here after setting RAX to the return value.
    && gta_compiler_context_set_label(context, context->break_label, v->count)
    && gta_compiler_context_set_label(context, context->return_label, v->count)

  // Restore the stack pointer to before we added the global and local
  // variables.  This is faster than popping the locals and globals off the
  // stack, and the garbage collector will clean up the memory.
  //   mov rsp, r13
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_R13)

  // Pop callee-saved registers off the stack.
  //   pop rbx
  //   pop r12
  //   pop r13
  //   pop r14
  //   pop r15
    && gta_pop_reg__x86_64(v, GTA_REG_RBX)
    && gta_pop_reg__x86_64(v, GTA_REG_R12)
    && gta_pop_reg__x86_64(v, GTA_REG_R13)
    && gta_pop_reg__x86_64(v, GTA_REG_R14)
    && gta_pop_reg__x86_64(v, GTA_REG_R15)

  // Set up the end of the function:
  //   leave
  //   ret
    && gta_leave__x86_64(v)
    && gta_ret__x86_64(v)

  // Continue: It is possible that a continue statement was not within a loop.
  // We will catch this situation here and make sure that a null value is in
  // RAX and jump to the break label.
  //   mov rax, gta_computed_value_null
  //   jmp break
    && gta_compiler_context_set_label(context, context->continue_label, v->count)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (uint64_t)gta_computed_value_null)
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, context->break_label, v->count - 4)
  ;

  // The compilation is finished (aside from writing the jump targets).  If
  // there were any errors, then the binary is not valid.
  if (!error_free) {
    goto CONTEXT_CLEANUP;
  }

  // Correct the JUMP instructions to point to the correct location in the
  // binary.  The jump targets are relative to the instruction after the jump.
  // It is 4 bytes long.
  if (context->labels->count != context->labels_from->count) {
    goto CONTEXT_CLEANUP;
  }
  for (size_t i = 0; i < context->labels->count; ++i) {
    GTA_Integer label = GTA_TYPEX_UI(context->labels->data[i]);
    GTA_VectorX * jumps = GTA_TYPEX_P(context->labels_from->data[i]);
    for (size_t j = 0; j < jumps->count; ++j) {
      GTA_Integer jump = GTA_TYPEX_UI(jumps->data[j]);
      if (jump < 0 || (GTA_UInteger)jump >= v->count) {
        goto CONTEXT_CLEANUP;
      }
      // Jump is relative to the next instruction and is 4 bytes long.
      int32_t offset = (int32_t)(label - jump - 4);
      memcpy(&v->data[jump], &offset, 4);
    }
  }

  // Lastly, copy the binary into executable memory.
  size_t length = gcu_vector8_count(v);
#ifdef _WIN32
  program->binary = VirtualAlloc(0, length, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (program->binary) {
    memcpy(program->binary, v->data, length);
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
    memcpy(program->binary, v->data, length);
    if (mprotect(program->binary, length, PROT_EXEC | PROT_READ) != 0) {
      munmap(program->binary, length);
      program->binary = 0;
    }
    else {
      // dump the binary to stderr
      // printf("\nProgram code:\n%s\n", program->code);
      // fwrite(v->data, 1, length, stderr);
    }
  }
#endif

  // At this point, we must update any function pointers to point to the acutal
  // address in the binary.  They currently represent the byte offset into the
  // code block.  We will do this by iterating through the AST and updating the
  // function pointers.
  gta_ast_node_walk(program->ast, update_function_pointers, program->binary, 0);

  goto CONTEXT_CLEANUP;

GLOBALS_ORDER_CLEANUP:
  gcu_free(variables_order);

CONTEXT_CLEANUP:
  gta_compiler_context_destroy(context);
}


void gta_program_compile_binary__arm_64(GTA_MAYBE_UNUSED(GTA_Program * program)) {}


void gta_program_compile_binary__x86_32(GTA_MAYBE_UNUSED(GTA_Program * program)) {}


void gta_program_compile_binary__arm_32(GTA_MAYBE_UNUSED(GTA_Program * program)) {}
