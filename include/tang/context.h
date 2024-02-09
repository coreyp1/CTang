/**
 * @file
 *
 * Header file for the Context class.
 *
 * The Context class is used to manage the state of the execution environment
 * for a Tang program as it is being executed.
 */

#ifndef G_TANG_CONTEXT_H
#define G_TANG_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdbool.h>
#include <cutil/vector.h>
#include "tang/unicodeString.h"

typedef struct GTA_Program GTA_Program;
typedef struct GTA_Computed_Value GTA_Computed_Value;

typedef struct GTA_Context {
  GTA_Program * program;
  GTA_Unicode_String * output;
  GTA_Computed_Value * result;
  GCU_Vector64 * stack;
} GTA_Context;

GTA_Context * gta_context_create(GTA_Program * program);
bool gta_context_create_in_place(GTA_Context * context, GTA_Program * program);
void gta_context_destroy(GTA_Context * context);
void gta_context_destroy_in_place(GTA_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_CONTEXT_H
