/**
 * @file
 *
 * This file contains the definition of the GTA_Computed_Value_String class.
 */

#ifndef G_TANG_COMPUTED_VALUE_STRING_H
#define G_TANG_COMPUTED_VALUE_STRING_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue.h>
#include <tang/unicodeString.h>

/**
 * The vtable for the GTA_Ast_Node_String class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_string_vtable;

/**
 * The GTA_Computed_Value_String class.
 */
typedef struct GTA_Computed_Value_String {
  /**
   * The base class.
   */
  GTA_Computed_Value base;
  /**
   * The string.
   *
   * Verify that the string is owned by the object by checking the is_owned
   * field.
   */
  GTA_Unicode_String * value;
  /**
   * Whether or not this object owns the string.
   *
   * For performance reasons, some strings should be reused.  For example, any
   * string that is a constant in the source code should be reused.  The empty
   * string is also a singleton.
   */
  bool is_owned;
} GTA_Computed_Value_String;

/**
 * Singleton for an empty Computed_Value_String object.
 */
extern GTA_Computed_Value * gta_computed_value_string_empty;

/**
 * Creates a new GTA_Computed_Value_String object.
 *
 * @param value The GTA_Unicode_String project.
 * @param adopt Whether or not this object should take control of the string
 *   reference.
 * @param context The execution context to create the value in.
 * @return The new object.
 */
GTA_Computed_Value_String * GTA_CALL gta_computed_value_string_create(GTA_Unicode_String * value, bool adopt, GTA_Execution_Context * context);

/**
 * Creates a new GTA_Computed_Value_String object in place.
 *
 * @param self The memory address of the object.
 * @param value The GTA_Unicode_String project.
 * @param adopt Whether or not this object should take control of the string
 *  reference.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_string_create_in_place(GTA_Computed_Value_String * self, GTA_Unicode_String * value, bool adopt, GTA_Execution_Context * context);

/**
 * Destroys a GTA_Computed_Value_String object.
 *
 * This function should not be called directly.  Instead, use the
 * gta_computed_value_destroy() function.
 *
 * @see gta_computed_value_destroy
 *
 * @param object The object.
 */
void GTA_CALL gta_computed_value_string_destroy(GTA_Computed_Value * object);

/**
 * Destroys a GTA_Computed_Value_String object in place.
 *
 * This function should not be called directly.  Instead, use the
 * gta_computed_value_destroy_in_place() function.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The object.
 */
void GTA_CALL gta_computed_value_string_destroy_in_place(GTA_Computed_Value * self);

/**
 * Creates a deep copy of a GTA_Computed_Value_String object.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param self The object.
 * @return The new object or NULL on failure.
 */
GTA_Computed_Value * GTA_CALL gta_computed_value_string_deep_copy(GTA_Computed_Value * self);

/**
 * Get a string representation of a GTA_Computed_Value_String object.
 *
 * The caller is responsible for freeing the returned string with
 * gta_free().
 *
 * @see gta_computed_value_to_string
 *
 * @param self The object.
 * @return The string or NULL on failure.
 */
char * GTA_CALL gta_computed_value_string_to_string(GTA_Computed_Value * self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTED_VALUE_STRING_H