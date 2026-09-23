/*
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * Copyright (C) 2024-2026 Corey Pennycuff
 *
 * This file is part of Ghoti.io Tang.
 *
 * Ghoti.io Tang is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Ghoti.io Tang is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * This file contains the definition of the GTA_Computed_Value_String class.
 */

#ifndef GHOTI_IO_GTA_COMPUTEDVALUE_COMPUTEDVALUESTRING_H
#define GHOTI_IO_GTA_COMPUTEDVALUE_COMPUTEDVALUESTRING_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <ghoti.io/tang/computedValue/computedValue.h>
#include <ghoti.io/tang/unicodeString.h>

/**
 * The vtable for the GTA_Computed_Value_String class.
 */
GTA_API extern GTA_Computed_Value_VTable gta_computed_value_string_vtable;

/**
 * The GTA_Computed_Value_String class.
 */
struct GTA_Computed_Value_String {
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
};

/**
 * Singleton for an empty Computed_Value_String object.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_string_empty;

/**
 * Creates a new GTA_Computed_Value_String object.
 *
 * @param value The GTA_Unicode_String project.
 * @param adopt Whether or not this object should take control of the string
 *   reference.
 * @param context The execution context to create the value in.
 * @return The new object.
 */
GTA_API GTA_Computed_Value_String * GTA_CALL gta_computed_value_string_create(GTA_Unicode_String * value, bool adopt, GTA_Execution_Context * context);

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
GTA_API bool GTA_CALL gta_computed_value_string_create_in_place(GTA_Computed_Value_String * self, GTA_Unicode_String * value, bool adopt, GTA_Execution_Context * context);

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
GTA_API void GTA_CALL gta_computed_value_string_destroy(GTA_Computed_Value * object);

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
GTA_API void GTA_CALL gta_computed_value_string_destroy_in_place(GTA_Computed_Value * self);

/**
 * Creates a deep copy of a GTA_Computed_Value_String object.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param self The object.
 * @param context The execution context of the program.
 * @return The new object or NULL on failure.
 */
GTA_API GTA_Computed_Value * GTA_CALL gta_computed_value_string_deep_copy(GTA_Computed_Value * self, GTA_Execution_Context * context);

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
GTA_API char * GTA_CALL gta_computed_value_string_to_string(GTA_Computed_Value * self);

/**
 * Prints a computed value.
 *
 * Calls the `print` method of the virtual table.
 *
 * @param self The object to print.
 * @param context The execution context of the program.
 * @return A string representation of the object or NULL if the operation
 *  failed.
 */
GTA_API GTA_NO_DISCARD GTA_Unicode_String * GTA_CALL gta_computed_value_string_print(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Concatenates a string with another value.
 *
 * Calls the `add` method of the virtual table.
 *
 * The other value is converted to text exactly as printing it would, so
 * `"a" + b` and printing `"a"` followed by `b` produce the same bytes.  A
 * value that cannot be printed cannot be concatenated, and an error operand
 * is returned as the result rather than being rendered into the text.
 *
 * @param self The string.
 * @param other The other operand.
 * @param self_is_lhs Whether `self` is the left-hand side of the operation.
 * @param is_assignment Whether the operation is an assignment.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_string_add(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Casts a computed value to a different type.
 *
 * Calls the `cast` method of the virtual table.
 *
 * @param self The object to cast.
 * @param type The type to cast to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_string_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);

/**
 * Gets a value from the object using an index.
 *
 * @param self The object to get the value from.
 * @param index The index to get.
 * @param context The execution context of the program.
 * @return The value of the index or NULL if the operation failed.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_string_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Execution_Context * context);

/**
 * Gets a slice of the object.
 *
 * @param self The object to get the slice from.
 * @param start The start of the slice.
 * @param end The end of the slice.
 * @param step The step of the slice.
 * @param context The execution context of the program.
 * @return The slice of the object or NULL if the operation failed.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_string_slice(GTA_Computed_Value * self, GTA_Computed_Value * start, GTA_Computed_Value * end, GTA_Computed_Value * step, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_COMPUTEDVALUE_COMPUTEDVALUESTRING_H
