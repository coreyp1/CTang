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
 */

#ifndef GHOTI_IO_GTA_COMPUTEDVALUE_COMPUTEDVALUEMAP_H
#define GHOTI_IO_GTA_COMPUTEDVALUE_COMPUTEDVALUEMAP_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <ghoti.io/tang/computedValue/computedValue.h>

/**
 * The VTable for the ComputedValueMap.
 */
GTA_API extern GTA_Computed_Value_VTable gta_computed_value_map_vtable;

/**
 * A Computed Value Error for when a requested key is not found in the map.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_map_key_not_found;

/**
 * A Computed Value Error for when a supplied key is not a string.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_map_key_not_string;

/**
 * An object that maps identifiers to computed values.
 *
 * Internally, the map's key/value pairs are stored in a pair of hash tables, a
 * key hash and a value hash.  The two hash tables are kept in sync with each
 * other.  They use the same hash value (from the key-string) to refer to both
 * the key and the value.  The reason for this is so that the map can be
 * iterated over and that the actual key string value is recoverable.
 */
struct GTA_Computed_Value_Map {
  /**
   * The base class for the computed value.
   */
  GTA_Computed_Value base;
  /**
   * The hash table for the keys.
   */
  GTA_HashX * key_hash;
  /**
   * The hash table for the values.
   */
  GTA_HashX * value_hash;
};

/**
 * Create a new computed value for a map container.
 *
 * @param size The expected size of the map.
 * @param context The execution context to create the value in.
 * @return The new computed value for the map.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_map_create(size_t size, GTA_Execution_Context * context);

/**
 * Create a new computed value for a map in place.
 *
 * @param self The memory address of the computed value.
 * @param size The expected size of the map.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
GTA_API bool GTA_CALL gta_computed_value_map_create_in_place(GTA_Computed_Value_Map * self, size_t size, GTA_Execution_Context * context);

/**
 * Destroy a computed value for a map.
 *
 * @see gta_computed_value_destroy
 *
 * @param self The computed value for the map.
 */
GTA_API void GTA_CALL gta_computed_value_map_destroy(GTA_Computed_Value * self);

/**
 * Destroy a computed value for a map in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The computed value for the map.
 */
GTA_API void GTA_CALL gta_computed_value_map_destroy_in_place(GTA_Computed_Value * self);

/**
 * Deep copy a computed value for a map.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param value The computed value to be copied.
 * @param context The execution context of the program.
 * @return The deep copy of the ComputedValueMap or NULL if an error occurred.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_map_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context);

/**
 * Get a string representation of the computed value for a map.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The computed value for the map.
 * @return The string representation of the computed value for the map.
 */
GTA_API GTA_NO_DISCARD char * GTA_CALL gta_computed_value_map_to_string(GTA_Computed_Value * self);

/**
 * Assigns a value to an index of the object.
 *
 * @param self The object to modify.
 * @param index The index to assign to.
 * @param other The value to assign.
 * @param context The execution context of the program.
 * @return The modified object or NULL if the operation failed.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_map_assign_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other, GTA_Execution_Context * context);

/**
 * Gets a value from the object using an index.
 *
 * @param self The object to get the value from.
 * @param index The index to get.
 * @param context The execution context of the program.
 * @return The value of the index or NULL if the operation failed.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_map_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Execution_Context * context);

/**
 * Get the computed value for a key in the map.
 * 
 * @param self The map on which to operate
 * @param key A string key to look up in the map
 * @return The computed value for the key or NULL if the key is not found.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_map_get_from_cstring(GTA_Computed_Value_Map * self, const char * key);

/**
 * Reads a member of the map.
 *
 * `m.name` is the same lookup as `m["name"]`.  A built-in attribute of the
 * map type, if one ever exists, is checked first, so that data in a map
 * cannot shadow one.
 *
 * @param self The map.
 * @param identifier_hash The hash of the member name.
 * @param context The execution context of the program.
 * @return The member's value, or an error if there is no such member.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_map_period(GTA_Computed_Value * self, GTA_UInteger identifier_hash, GTA_Execution_Context * context);


/**
 * Add a computed value to the map using the provided key.
 *
 * This function will be a helper function for the Map's index assign function.
 * An index assign leaves the rhs value on the top of the stack after the
 * insertion and assignment, so ultimately the value of the key/value pair is
 * what should be returned.
 *
 * The map will take ownership of the key and value.
 *
 * @param self The map on which to operate.
 * @param key The key to use for the insertion.
 * @param value The value to insert into the map.
 * @return The value of the key/value pair or NULL if an error occurred.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_map_set_key_val(GTA_Computed_Value_Map * self, GTA_Computed_Value * key, GTA_Computed_Value * value);


/**
 * Casts the map to another type.
 *
 * A map casts to a boolean - true when it holds something - and to a
 * string, which is its rendering.  There is no number it could sensibly be.
 *
 * @param self The map.
 * @param type The type to cast to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_map_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_COMPUTEDVALUE_COMPUTEDVALUEMAP_H
