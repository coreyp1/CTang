/**
 * @file
 *
 * Header file for the garbage collector functionality.
 */

#ifndef GHOTI_IO_GTA_PROGRAM_GARBAGECOLLECTOR_H
#define GHOTI_IO_GTA_PROGRAM_GARBAGECOLLECTOR_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <ghoti.io/cutil/vector.h>

/**
 * A list of allocations that have been created but not yet freed.
 *
 * Each entry is a pointer to the computedValue object.
 */
typedef GTA_VectorX GTA_Garbage_Collector_Allocations_List;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_PROGRAM_GARBAGECOLLECTOR_H