/**
 * @file
 *
 * Header file for the Context class.
 *
 * The Context class is used to manage the state of the execution environment
 * for a Tang program as it is being executed.
 */

#ifndef G_TANG_EXECUTION_CONTEXT_H
#define G_TANG_EXECUTION_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdbool.h>
#include <cutil/vector.h>
#include <tang/macros.h>
#include <tang/unicodeString.h>

typedef struct GTA_Program GTA_Program;
typedef struct GTA_Computed_Value GTA_Computed_Value;

/**
 * The Context class.
 *
 * The Context class is used to manage the state of the execution environment
 * for a Tang program as it is being executed.
*/
typedef struct GTA_Execution_Context {
  /**
   * The program being executed.
  */
  GTA_Program * program;
  /**
   * The output string.
   */
  GTA_Unicode_String * output;
  /**
   * The result of the last operation.
   */
  GTA_Computed_Value * result;
  /**
   * The stack used for execution.
   */
  GTA_VectorX * stack;
} GTA_Execution_Context;

/**
 * Creates a new Context object.
 *
 * Use with gta_execution_context_destroy().
 *
 * @see gta_execution_context_destroy()
 *
 * @param program The program associated with the execution.
 * @return The new Context object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Execution_Context * gta_execution_context_create(GTA_Program * program);

/**
 * Creates a new Context object using the supplied memory location.
 *
 * Use with gta_bytecode_execution_context_destroy_in_place().
 *
 * @see gta_bytecode_execution_context_destroy_in_place()
 *
 * @param context The memory location to use for the new Context object.
 * @param program The program associated with the execution.
 * @return true on success, false on failure.
 */
bool gta_execution_context_create_in_place(GTA_Execution_Context * context, GTA_Program * program);

/**
 * Destroys a Context object.
 *
 * Use with gta_execution_context_create().
 *
 * @see gta_execution_context_create()
 *
 * @param context The Context object to destroy.
 */
void gta_execution_context_destroy(GTA_Execution_Context * context);

/**
 * Destroys a Context object using the supplied memory location.
 *
 * The memory used by the Context object is not freed, but the object is
 * destroyed.
 *
 * Use with gta_execution_context_create_in_place().
 *
 * @see gta_execution_context_create_in_place()
 *
 * @param context The Context object to destroy.
 */
void gta_bytecode_execution_context_destroy_in_place(GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_EXECUTION_CONTEXT_H
