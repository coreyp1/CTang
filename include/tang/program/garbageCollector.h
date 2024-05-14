/**
 * @file
 *
 * Header file for the garbage collector functionality.
 */

#ifndef G_TANG_GARBAGECOLLECTOR_H
#define G_TANG_GARBAGECOLLECTOR_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <cutil/vector.h>
#include <tang/macros.h>

/**
 * A list of allocations that have been created but not yet freed.
 *
 * Each entry is a pointer to the computedValue object.
 */
typedef GTA_VectorX GTA_Garbage_Collector_Allocations_List;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_GARBAGECOLLECTOR_H