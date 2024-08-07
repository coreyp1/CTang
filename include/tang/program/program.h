/**
 * @file
 *
 * Header file for the Program class.
 *
 * @see GTA_Program
 */

#ifndef G_TANG_PROGRAM_H
#define G_TANG_PROGRAM_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <cutil/hash.h>
#include <tang/ast/astNode.h>
#include <tang/computedValue/computedValue.h>
#include <tang/program/bytecode.h>
#include <tang/program/executionContext.h>

/**
 * The flags for a program.
 *
 * @see GTA_PROGRAM_FLAG_DEFAULT
 * @see GTA_PROGRAM_FLAG_DEBUG
 * @see GTA_PROGRAM_FLAG_DUPLICATE_CODE
 * @see GTA_PROGRAM_FLAG_DISABLE_BYTECODE
 * @see GTA_PROGRAM_FLAG_DISABLE_BINARY
 */
typedef uint32_t GTA_Program_Flags;

/**
 * Use default flags for the program.
 *
 * The default flag set is to have no flags set.
 *
 * @see GTA_Program_Flags
 * @see gta_program_create()
 */
#define GTA_PROGRAM_FLAG_DEFAULT 0

/**
 * Enable debug mode for the program.
 *
 * @see GTA_Program_Flags
 * @see gta_program_create()
 */
#define GTA_PROGRAM_FLAG_DEBUG 1

/**
 * When creating the program, do not adopt the code.  Rather, make a copy of
 * it.
 *
 * @see GTA_Program_Flags
 * @see gta_program_create()
 */
#define GTA_PROGRAM_FLAG_DUPLICATE_CODE 2

/**
 * Disable bytecode generation for the program.
 *
 * @see GTA_Program_Flags
 * @see gta_program_create()
 */
#define GTA_PROGRAM_FLAG_DISABLE_BYTECODE 4

/**
 * Disable binary generation for the program.
 *
 * @see GTA_Program_Flags
 * @see gta_program_create()
 */
#define GTA_PROGRAM_FLAG_DISABLE_BINARY 8

/**
 * Holds the metadata for a program.
 */
typedef struct GTA_Program {
  /**
   * The code that the program was created from.
   */
  const char * code;
  /**
   * The AST for the program, if parsing was successful.
   */
  GTA_Ast_Node * ast;
  /**
   * The bytecode for the program, if it was generated.
   */
  GTA_VectorX * bytecode;
  /**
   * The binary for the program, if it was generated.
   */
  void * binary;
  /**
   * The flags for the program.
   */
  GTA_Program_Flags flags;
  /**
   * The variable scope structure for the program.
   *
   * This structure is built during the analysis phase of the script
   * compilation.  It contains information such as which AST defines
   * a variable name and the scope in which it is defined.
   */
  GTA_Variable_Scope * scope;
  /**
   * A container to hold Computed Value singleton objects.
   *
   * During the analysis or compilation phase, AST nodes may create singleton
   * objects that are used at runtime.  This container holds those objects so
   * that they can be destroyed when the program is destroyed.
   */
  GTA_VectorX * singletons;
} GTA_Program;

/**
 * Create a new program from the given code.
 *
 * Default flags are used based on environment variables.  If no environment
 * variables are set, then a default configuration will be chosen.
 *
 * | Environment Variable | Default Value | Description |
 * | -------------------- | ------------- | ----------- |
 * | TANG_DEBUG |  | If set, then the program will be created with the debug flag enabled. |
 * | TANG_DISABLE_BYTECODE |  | If set, then programs will not be compiled to bytecode. |
 * | TANG_DISABLE_BINARY |  | If set, then programs will not be compiled to binary. |
 *
 * @param code The code to create the program from.
 * @return The new program or null if the program could not be created.
 */
GTA_NO_DISCARD GTA_Program * gta_program_create(const char * code);

/**
 * Create a new program from the given code in the memory location provided.
 *
 * Default flags are used based on environment variables.  For more details,
 * see gta_program_create().
 *
 * A program created in place must only be destroyed with
 * gta_program_destroy_in_place().
 *
 * @see gta_program_destroy_in_place()
 *
 * @param program The memory location to create the program in.
 * @param code The code to create the program from.
 * @return True if the program was created successfully, false otherwise.
 */
bool gta_program_create_in_place(GTA_Program * program, const char * code);

/**
 * Create a new program from the given code with the given flags.
 *
 * @param code The code to create the program from.
 * @param flags The flags to create the program with.
 * @return The new program or null if the program could not be created.
 */
GTA_NO_DISCARD GTA_Program * gta_program_create_with_flags(const char * code, GTA_Program_Flags flags);

/**
 * Create a new program from the given code with the given flags in the memory
 * location provided.
 *
 * A program created in place must only be destroyed with
 * gta_program_destroy_in_place().
 *
 * @see gta_program_destroy_in_place()
 *
 * @param program The memory location to create the program in.
 * @param code The code to create the program from.
 * @param flags The flags to create the program with.
 * @return True if the program was created successfully, false otherwise.
 */
bool gta_program_create_in_place_with_flags(GTA_Program * program, const char * code, GTA_Program_Flags flags);

/**
 * Destroy the given program.
 *
 * @param program The program to destroy.
*/
void gta_program_destroy(GTA_Program * program);

/**
 * Destroy the given program, except for the program container.
 *
 * Use this only if you have allocated the program container yourself.
 *
 * @see gta_program_create_in_place()
 * @see gta_program_create_in_place_with_flags()
 *
 * @param program The program to destroy.
 */
void gta_program_destroy_in_place(GTA_Program * program);

/**
 * Execute the given program with the given context.
 *
 * If the program was compiled to binary, then the binary will be executed.
 * Otherwise, if the program was compiled to bytecode then the bytecode will be
 * executed.  If the program has neither, then the function will return false.
 *
 * @param context The initialized context with which to execute the program.
 * @return True if the program executed successfully, false otherwise.
 */
bool gta_program_execute(GTA_Execution_Context * context);

/**
 * Execute the given program with the given context using the bytecode.
 *
 * @param context The initialized context with which to execute the program.
 * @return True if the program executed successfully, false otherwise.
 */
bool gta_program_execute_bytecode(GTA_Execution_Context * context);

/**
 * Execute the given program with the given context using the binary.
 *
 * @param context The initialized context with which to execute the program.
 * @return True if the program executed successfully, false otherwise.
 */
bool gta_program_execute_binary(GTA_Execution_Context * context);

/**
 * Print the bytecode for the given program.
 *
 * @param program The program to print the bytecode for.
 */
void gta_program_bytecode_print(GTA_Program * program);

/**
 * Attempt to JIT compile the given program to binary for x86_64.
 *
 * If the compilation fails, then no binary will be produced in
 * program->binary.
 *
 * @param program The program to be compiled.
 */
void gta_program_compile_binary__x86_64(GTA_Program * program);

/**
 * Attempt to JIT compile the given program to binary for arm_64.
 *
 * If the compilation fails, then no binary will be produced in
 * program->binary.
 *
 * @param program The program to be compiled.
 */
void gta_program_compile_binary__arm_64(GTA_Program * program);

/**
 * Attempt to JIT compile the given program to binary for x86_32.
 *
 * If the compilation fails, then no binary will be produced in
 * program->binary.
 *
 * @param program The program to be compiled.
 */
void gta_program_compile_binary__x86_32(GTA_Program * program);

/**
 * Attempt to JIT compile the given program to binary for arm_32.
 *
 * If the compilation fails, then no binary will be produced in
 * program->binary.
 *
 * @param program The program to be compiled.
 */
void gta_program_compile_binary__arm_32(GTA_Program * program);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_PROGRAM_H
