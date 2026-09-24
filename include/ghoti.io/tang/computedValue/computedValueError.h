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
 * Header file for the computed value error class.
 *
 * Also defines some general-purpose error values.
 */

#ifndef GHOTI_IO_GTA_COMPUTEDVALUE_COMPUTEDVALUEERROR_H
#define GHOTI_IO_GTA_COMPUTEDVALUE_COMPUTEDVALUEERROR_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <ghoti.io/tang/computedValue/computedValue.h>

/**
 * The vtable for the GTA_Computed_Value_Error class.
 */
GTA_API extern GTA_Computed_Value_VTable gta_computed_value_error_vtable;

/**
 * Generic "Not implemented" singleton error value.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_not_implemented;

/**
 * Generic "Out of memory" singleton error value.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_out_of_memory;

/**
 * Generic "invalid bytecode" singleton error value.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_invalid_bytecode;

/**
 * Generic "Divide by zero" singleton error value.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_divide_by_zero;

/**
 * Generic "Modulo by zero" singleton error value.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_modulo_by_zero;

/**
 * Generic "not supported" singleton error value.
 *
 * In contrast to the "not implemented" error, this error is used when the
 * operation is known but not supported by the current implementation.
 * For example, in the code example `"hello" / 3`, the division operation is
 * known but you can't divide a string by a number.
 *
 * If this error is encountered when evaluating a binary operation, then the
 * operation should be retried with the operands swapped and the reversed flag
 * set.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_not_supported;

/**
 * Indicates that the supplied index is not valid.  Only integer indexes are
 * supported.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_invalid_index;

/**
 * Indicates that the attempted function call could not be executed, because
 * the object was not a function.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_invalid_function_call;

/**
 * A singleton for a call that nested deeper than the execution context
 * allows.
 *
 * The x86-64 engine calls compiled functions with real `call` instructions,
 * so unbounded recursion runs the process's own stack out and takes the
 * host down with it.  See GTA_EXECUTION_CONTEXT_DEFAULT_MAX_CALL_DEPTH.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_recursion_limit;

/**
 * Indicates that the attempted function call could not be executed, because
 * the number of arguments supplied did not match the number of parameters
 * expected by the function.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_argument_count_mismatch;

/**
 * Error resulting from trying to set the seed of the global random number
 * generator.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_global_rng_seed_not_changeable;

/**
 * The vtable for error values that render as their own marker.
 *
 * It is the ordinary error vtable in every respect but one: printing it
 * produces the message itself rather than nothing. Ordinary errors print as
 * nothing, which is the right answer when the failure is the operation - there
 * is no value to show. It is the wrong answer when the failure IS the value,
 * because the alternative is not silence but a number that is simply untrue.
 *
 * Casting an out-of-range float to an integer used to give -9223372036854775808
 * for a huge positive value as readily as a huge negative one, because that is
 * what the x86-64 conversion instruction produces when the answer does not fit.
 * A reader cannot tell that from a real result. These values say so instead.
 */
GTA_API extern GTA_Computed_Value_VTable gta_computed_value_error_marker_vtable;

/**
 * Singleton for a value too large to represent in the target type.
 *
 * Prints as `[INTEGER TOO LARGE]`.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_integer_too_large;

/**
 * Singleton for a value too small (too negative) to represent in the target
 * type.
 *
 * Prints as `[INTEGER TOO SMALL]`.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_integer_too_small;

/**
 * Singleton for a value that is not a number at all, and so is neither an
 * overflow nor an underflow.
 *
 * Reachable: `inf - inf` is a NaN, and `inf` is reachable from a float literal
 * too large to represent. Prints as `[NOT A NUMBER]`.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_error_not_a_number;

/**
 * Represents an error value.
 */
struct GTA_Computed_Value_Error {
  /**
   * The base class.
   */
  GTA_Computed_Value base;
  /**
   * The error message.
   */
  char * message;
};

/**
 * Creates a new GTA_Computed_Value_Error object.
 *
 * @param message The error message.
 * @return The new GTA_Computed_Value_Error object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_error_create(const char * message);

/**
 * Produce a string representation of the error.  The caller is responsible for
 * freeing the returned string.
 *
 * @param self The GTA_Computed_Value_Error object.
 * @return The string representation of the error.
 */
GTA_API GTA_NO_DISCARD char * GTA_CALL gta_computed_value_error_to_string(GTA_Computed_Value * self);

/**
 * Produce a string representation of a marker error: the message on its own,
 * with no "Error: " prefix, because the message is already the whole answer.
 * The caller is responsible for freeing the returned string.
 *
 * @param self The GTA_Computed_Value_Error object.
 * @return The string representation of the error.
 */
GTA_API GTA_NO_DISCARD char * GTA_CALL gta_computed_value_error_marker_to_string(GTA_Computed_Value * self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_COMPUTEDVALUE_COMPUTEDVALUEERROR_H
