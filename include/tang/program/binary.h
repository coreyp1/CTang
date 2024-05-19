/**
 * @file
 *
 * Contains functions dealing with the writing of binary instructions.
 */

#ifndef TANG_PROGRAM_BINARY_H
#define TANG_PROGRAM_BINARY_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/macros.h>

typedef enum GTA_Register {
  GTA_REG_RAX = 0,
  GTA_REG_RBX,
  GTA_REG_RCX,
  GTA_REG_RDX,
  GTA_REG_RSI,
  GTA_REG_RDI,
  GTA_REG_RBP,
  GTA_REG_RSP,
  GTA_REG_R8,
  GTA_REG_R9,
  GTA_REG_R10,
  GTA_REG_R11,
  GTA_REG_R12,
  GTA_REG_R13,
  GTA_REG_R14,
  GTA_REG_R15,
  GTA_REG_EAX,
  GTA_REG_EBX,
  GTA_REG_ECX,
  GTA_REG_EDX,
  GTA_REG_ESI,
  GTA_REG_EDI,
  GTA_REG_EBP,
  GTA_REG_ESP,
  GTA_REG_AX,
  GTA_REG_BX,
  GTA_REG_CX,
  GTA_REG_DX,
  GTA_REG_SI,
  GTA_REG_DI,
  GTA_REG_BP,
  GTA_REG_SP,
  GTA_REG_AL,
  GTA_REG_BL,
  GTA_REG_CL,
  GTA_REG_DL,
  GTA_REG_AH,
  GTA_REG_BH,
  GTA_REG_CH,
  GTA_REG_DH,
} GTA_Register;

bool gta_binary_optimistic_increase(GCU_Vector8 * vector, size_t additional);
uint8_t gta_binary_get_register_code__x86_64(GTA_Register reg);
bool gta_and_reg_imm__x86_64(GCU_Vector8 * vector, GTA_Register dst, int32_t src);
bool gta_call_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg);
bool gta_lea_reg_mem__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register base, int32_t offset);
bool gta_leave__x86_64(GCU_Vector8 * vector);
bool gta_mov_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src);
bool gta_mov_reg_imm__x86_64(GCU_Vector8 * vector, GTA_Register dst, uint64_t src);
bool gta_pop_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg);
bool gta_push_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg);
bool gta_ret__x86_64(GCU_Vector8 * vector);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_PROGRAM_BINARY_H
