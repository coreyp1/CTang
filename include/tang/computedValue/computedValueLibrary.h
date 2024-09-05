/**
 * @file
 */

#ifndef G_TANG_COMPUTED_VALUE_LIBRARY_H
#define G_TANG_COMPUTED_VALUE_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue/computedValue.h>
#include <tang/library/library.h>

/**
 * The vtable for the GTA_Computed_Value_Library class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_library_vtable;

/**
 * This struct is used to associate a library attribute name with a function
 * that will be called to get the library's attribute value.
 *
 * The function is called with only the context as an argument.
 */
typedef struct GTA_Computed_Value_Library_Attribute_Pair {
  /**
   * The attribute name.
   */
  const char * name;
  /**
   * The function to be called to get the library's attribute value.
   */
  GTA_Library_Callback callback;
} GTA_Computed_Value_Library_Attribute_Pair;

/**
 * The GTA_Computed_Value_Library class.
 */
typedef struct GTA_Computed_Value_Library {
  /**
   * The base class.
   */
  GTA_Computed_Value base;
  /**
   * The name of the library.
   */
  const char * name;
  /**
   * The library attribute pairs.
   */
  GTA_Computed_Value_Library_Attribute_Pair * attributes;
  /**
   * The number of library attribute pairs.
   */
  GTA_UInteger attribute_count;
} GTA_Computed_Value_Library;

/**
 * Initializes a GTA_Computed_Value_Library object.
 *
 * The name will be copied into the object.
 * The attributes array will be copied into the object.
 *
 * @param name The name of the library.
 * @param attributes The library attribute pairs.
 * @param attribute_count The number of library attribute pairs.
 * @param context The execution context in which to create the value.
 * @return The initialized object or NULL if an error occurred.
 */
GTA_Computed_Value_Library * GTA_CALL gta_computed_value_library_create(const char * name, GTA_Computed_Value_Library_Attribute_Pair * attributes, GTA_UInteger attribute_count, GTA_Execution_Context * context);

/**
 * Create a computed value for a library in place.
 *
 * The name will be copied into the object.
 * The attributes array will be copied into the object.
 *
 * @param self The memory address of the computed value.
 * @param name The name of the library.
 * @param attributes The library attribute pairs.
 * @param attribute_count The number of library attribute pairs.
 * @param context The execution context in which to create the value.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_library_create_in_place(GTA_Computed_Value_Library * self, const char * name, GTA_Computed_Value_Library_Attribute_Pair * attributes, GTA_UInteger attribute_count, GTA_Execution_Context * context);

/**
 * Destroys a GTA_Computed_Value_Library object.
 *
 * This function should not be called directly. Instead, use the
 * gta_computed_value_destroy() function.
 *
 * @see gta_computed_value_destroy
 *
 * @param self The object to destroy.
 */
void GTA_CALL gta_computed_value_library_destroy(GTA_Computed_Value * self);

/**
 * Destroy a computed value for a library in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The memory address of the computed value.
 */
void GTA_CALL gta_computed_value_library_destroy_in_place(GTA_Computed_Value * self);

/**
 * Deep copy a computed value for a library.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param value The computed value to be copied.
 * @param context The execution context of the program.
 * @return The deep copy of the GTA_Computed_Value_Library or NULL if an error occurred.
 */
GTA_Computed_Value * GTA_CALL gta_computed_value_library_deep_copy(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Gets a string representation of a computed value for a library.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The object to convert.
 * @return A string representation of the object or NULL if the operation failed.
 */
char * GTA_CALL gta_computed_value_library_to_string(GTA_Computed_Value * self);

/**
 * Gets the value of a library attribute.
 *
 * @param self The object.
 * @param attribute_name The name of the attribute.
 * @param context The execution context of the program.
 * @return The value of the attribute or NULL if the operation failed.
 */
GTA_Computed_Value * gta_computed_value_library_period(GTA_Computed_Value * self, GTA_UInteger identifier_hash, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTED_VALUE_LIBRARY_H
