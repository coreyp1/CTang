/**
 * @file
 *
 * Header file for the Program class.
 *
 * @see GTA_Program
 */

#ifndef G_TANG_PROGRAM_H
#define G_TANG_PROGRAM_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "tang/astNode.h"
#include "tang/bytecode.h"
#include "tang/computedValue.h"
#include "tang/context.h"
#include "tang/macros.h"

typedef struct GTA_Context GTA_Context;

typedef struct GTA_Program {
  const char * code;
  GTA_Ast_Node * ast;
  GTA_Bytecode_Vector * bytecode;
  void * binary;
  uint64_t flags;
} GTA_Program;

#define GTA_PROGRAM_FLAG_DEFAULT 0
#define GTA_PROGRAM_FLAG_DEBUG 1
#define GTA_PROGRAM_FLAG_DUPLICATE_CODE 2
#define GTA_PROGRAM_FLAG_FORCE_BYTECODE 4
#define GTA_PROGRAM_FLAG_FORCE_BINARY 8

GTA_Program * gta_program_create(const char * code);

bool gta_program_create_in_place(GTA_Program * program, const char * code);

GTA_Program * gta_program_create_with_flags(const char * code, uint64_t flags);

bool gta_program_create_in_place_with_flags(GTA_Program * program, const char * code, uint64_t flags);

void gta_program_destroy(GTA_Program * program);

void gta_program_destroy_in_place(GTA_Program * program);

bool gta_program_execute(GTA_Program * program, GTA_Context * context);

bool gta_program_execute_bytecode(GTA_Program * program, GTA_Context * context);

bool gta_program_execute_binary(GTA_Program * program, GTA_Context * context);

void gta_program_bytecode_print(GTA_Program * program);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_PROGRAM_H
