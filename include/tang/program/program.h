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
#include <tang/library/library.h>
#include <tang/program/bytecode.h>
#include <tang/program/executionContext.h>
#include <tang/program/language.h>

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
 * Set the program to be interpreted as a template.
 *
 * The default mode is to interpret the program as a script.
 *
 * @see GTA_Program_Flags
 * @see gta_program_create()
 */
#define GTA_PROGRAM_FLAG_IS_TEMPLATE 16

/**
 * Print the output directly to stdout.
 *
 * The output buffer will not be populated.
 *
 * @see GTA_Program_Flags
 * @see gta_program_create()
 */
#define GTA_PROGRAM_FLAG_PRINT_TO_STDOUT 32

/**
 * Ignore environment variables when setting flags.
 *
 * @see GTA_Program_Flags
 * @see gta_program_create()
 */
#define GTA_PROGRAM_FLAG_IGNORE_ENVIRONMENT 64

/**
 * Holds the metadata for a program.
 */
struct GTA_Program {
  /**
   * The language with which the program should be executed.
   */
  GTA_Language * language;
  /**
   * The library entries associated with this program.
   */
  GTA_Library * library;
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
   *
   * This is a 2-dimensional hash, in which the first dimension key is the
   * memory address of the VTable and the second dimension is a hash of the
   * computed value's value.  The value is the singleton object.
   */
  GTA_HashX * singletons;
  /**
   * A hash table that maps object types to the attributes that they have.
   *
   * This is a 2-dimensional hash, in which the first dimension key is the
   * memory address of the VTable and the second dimension is a hash of the
   * attribute name.  The value is the function that will be called to fulfill
   * the attribute value request.
   */
  GTA_HashX * attributes;
};

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
 * @param language The language with which the program should be executed.
 * @param code The code to create the program from.
 * @return The new program or null if the program could not be created.
 */
GTA_NO_DISCARD GTA_Program * gta_program_create(GTA_Language * language, const char * code);

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
 * @param language The language with which the program should be executed.
 * @param code The code to create the program from.
 * @return True if the program was created successfully, false otherwise.
 */
bool gta_program_create_in_place(GTA_Program * program, GTA_Language * language, const char * code);

/**
 * Create a new program from the given code with the given flags.
 *
 * @param language The language with which the program should be executed.
 * @param code The code to create the program from.
 * @param flags The flags to create the program with.
 * @return The new program or null if the program could not be created.
 */
GTA_NO_DISCARD GTA_Program * gta_program_create_with_flags(GTA_Language * language, const char * code, GTA_Program_Flags flags);

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
 * @param language The language with which the program should be executed.
 * @param code The code to create the program from.
 * @param flags The flags to create the program with.
 * @return True if the program was created successfully, false otherwise.
 */
bool gta_program_create_in_place_with_flags(GTA_Program * program, GTA_Language * language, const char * code, GTA_Program_Flags flags);

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

/**
 * The JIT compilation is not supported for the given architecture.
 *
 * @param program The program to be compiled.
 */
void gta_program_compile_binary__unsupported(GTA_Program * program);

/**
 * Get the type attribute function for the given type and identifier.
 *
 * The return value is a function that will be called to fulfill the attribute
 * value request.
 *
 * @param program The program to get the attribute function from.
 * @param type_vtable The vtable of the type.
 * @param identifier_hash The hash of the attribute name.
 * @return The function that will be called to fulfill the attribute value
 *   request.
 */
GTA_Computed_Value_Attribute_Callback gta_program_get_type_attribute(GTA_Program * program, GTA_Computed_Value_VTable * type_vtable, GTA_UInteger identifier_hash);

/**
 * Set an attribute function for a given type and identifier.
 *
 * @param program The program to set the attribute function for.
 * @param type_vtable The vtable of the type.
 * @param identifier_hash The hash of the attribute name.
 * @param callback The function that will be called to fulfill the attribute
 *   value request.
 * @return True if the attribute function was set successfully, false
 *   otherwise.
 */
bool gta_program_set_type_attribute(GTA_Program * program, GTA_Computed_Value_VTable * type_vtable, GTA_UInteger identifier_hash, GTA_Computed_Value_Attribute_Callback callback);

/**
 * Get a singleton for a given type and value hash, if it exists.
 *
 * @param program The program to get the singleton from.
 * @param type_vtable The vtable of the type.
 * @param value_hash The hash of the value.
 * @return The singleton object or null if it does not exist.
 */
GTA_Computed_Value * gta_program_get_singleton(GTA_Program * program, GTA_Computed_Value_VTable * type_vtable, GTA_UInteger value_hash);

/**
 * Set a singleton for a given type and value hash.
 *
 * @param program The program to set the singleton for.
 * @param type_vtable The vtable of the type.
 * @param value_hash The hash of the value.
 * @param singleton The singleton object to set.
 * @return True if the singleton was set successfully, false otherwise.
 */
bool gta_program_set_singleton(GTA_Program * program, GTA_Computed_Value_VTable * type_vtable, GTA_UInteger value_hash, GTA_Computed_Value * singleton);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_PROGRAM_H
