/**
 * @file
 *
 * The various random number generators available in the Random library.
 */

#ifndef TANG_COMPUTEDVALUE_COMPUTEDVALUERNG_H
#define TANG_COMPUTEDVALUE_COMPUTEDVALUERNG_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * A singleton representing the global random number generator.
 */
extern GTA_Computed_Value * gta_computed_value_random_global;

/**
 * The VTable for the ComputedValueRNG class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_rng_vtable;

/**
 * The computed value for a random number generator.
 */
typedef struct GTA_Computed_Value_RNG {
  /**
   * The base class for the computed value.
   */
  GTA_Computed_Value base;
  /**
   * The seed for the random number generator.
   */
  GTA_UInteger seed;
  /**
   * The random number generator state.
   */
  void * state;
} GTA_Computed_Value_RNG;

/**
 * Create a new computed value for a random number generator with the provided
 * seed.
 *
 * @param seed The seed for the random number generator.
 * @param context The execution context to create the value in.
 * @return The new computed value for the random number generator.
 */
GTA_NO_DISCARD GTA_Computed_Value_RNG * GTA_CALL gta_computed_value_rng_create_seeded(GTA_UInteger seed, GTA_Execution_Context * context);

/**
 * Create a new computed value for a random number generator with a default
 * seed.
 *
 * The seed is generated from the current time.
 *
 * @param context The execution context to create the value in.
 * @return The new computed value for the random number generator.
 */
GTA_NO_DISCARD GTA_Computed_Value_RNG * GTA_CALL gta_computed_value_rng_create(GTA_Execution_Context * context);

/**
 * Create a new computed value for a random number generator in place with the
 * provided seed.
 *
 * @param self The memory address of the computed value.
 * @param seed The seed for the random number generator.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_rng_create_seeded_in_place(GTA_Computed_Value_RNG * self, GTA_UInteger seed, GTA_Execution_Context * context);

/**
 * Destroy a computed value for a random number generator.
 *
 * @see gta_computed_value_destroy
 *
 * @param self The computed value to be destroyed.
 */
void GTA_CALL gta_computed_value_rng_destroy(GTA_Computed_Value * self);

/**
 * Destroy a computed value for a random number generator in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The computed value to be destroyed.
 */
void GTA_CALL gta_computed_value_rng_destroy_in_place(GTA_Computed_Value * self);

/**
 * Deep copy a computed value for a random number generator.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param self The computed value for the random number generator.
 * @param context The execution context to create the value in.
 * @return The deep copy of the computed value.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_rng_deep_copy(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Get a string representation of the computed value for a random number
 * generator.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The computed value for the random number generator.
 * @return The string representation of the computed value for the random number
 *   generator.
 */
GTA_NO_DISCARD char * GTA_CALL gta_computed_value_rng_to_string(GTA_Computed_Value * self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_COMPUTEDVALUE_COMPUTEDVALUERNG_H
