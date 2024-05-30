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

/**
 * A representation for the registers that may be available in the target
 * architecture.
 */
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
  GTA_REG_MMX0,
  GTA_REG_MMX1,
  GTA_REG_MMX2,
  GTA_REG_MMX3,
  GTA_REG_MMX4,
  GTA_REG_MMX5,
  GTA_REG_MMX6,
  GTA_REG_MMX7,
  GTA_REG_XMM0,
  GTA_REG_XMM1,
  GTA_REG_XMM2,
  GTA_REG_XMM3,
  GTA_REG_XMM4,
  GTA_REG_XMM5,
  GTA_REG_XMM6,
  GTA_REG_XMM7,
  GTA_REG_XMM8,
  GTA_REG_XMM9,
  GTA_REG_XMM10,
  GTA_REG_XMM11,
  GTA_REG_XMM12,
  GTA_REG_XMM13,
  GTA_REG_XMM14,
  GTA_REG_XMM15,
  GTA_REG_YMM0,
  GTA_REG_YMM1,
  GTA_REG_YMM2,
  GTA_REG_YMM3,
  GTA_REG_YMM4,
  GTA_REG_YMM5,
  GTA_REG_YMM6,
  GTA_REG_YMM7,
  GTA_REG_YMM8,
  GTA_REG_YMM9,
  GTA_REG_YMM10,
  GTA_REG_YMM11,
  GTA_REG_YMM12,
  GTA_REG_YMM13,
  GTA_REG_YMM14,
  GTA_REG_YMM15,
  GTA_REG_NONE,
} GTA_Register;

/**
 * Verify that the vector has enough space to store the additional bytes.
 *
 * If the vector has enough space, the function returns true. Otherwise, it
 * tries to increase the vector's capacity to store the additional bytes. If
 * the vector's capacity is increased, the function returns true. Otherwise, it
 * returns false.
 *
 * @param vector The vector to be verified.
 * @param additional The number of additional bytes to be stored.
 * @return True on success, false on failure.
 */
bool gta_binary_optimistic_increase(GCU_Vector8 * vector, size_t additional);

/**
 * Get the register code for the given register.
 *
 * Register codes are defined by the target architecture.
 *
 * @param reg The register to get the code for.
 * @return uint8_t The architecture-specific register code.
 */
uint8_t gta_binary_get_register_code__x86_64(GTA_Register reg);

/**
 * x86_64 instruction: AND reg, imm
 *
 * @param vector The vector in which to store the instruction.
 * @param dst The destination register.
 * @param immediate The immediate value.
 * @return True on success, false on failure.
 */
bool gta_and_reg_imm__x86_64(GCU_Vector8 * vector, GTA_Register dst, int32_t immediate);

/**
 * x86_64 instruction: CALL reg
 *
 * @param vector The vector in which to store the instruction.
 * @param reg The register to call.
 * @return True on success, false on failure.
 */
bool gta_call_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg);

/**
 * x86_64 instruction: JNZ offset
 *
 * The instruction jumps either forward or backward by the given offset.  The
 * offset is relative to the next instruction and is limited to a signed 32-bit
 * integer.
 *
 * @param vector The vector in which to store the instruction.
 * @param offset The offset to jump to.
 * @return True on success, false on failure.
 */
bool gta_jnz__x86_64(GCU_Vector8 * vector, int32_t offset);

/**
 * x86_64 instruction: LEA reg, [base + index*scale + offset]
 *
 * TODO: Add support for index and scale.
 *
 * @param vector The vector in which to store the instruction.
 * @param dst The destination register.
 * @param base The base register for the memory address.
 * @param index The index register for the memory address.
 * @param scale The scale for the index register.
 * @param offset The offset to add to the base register.
 * @return True on success, false on failure.
 */
bool gta_lea_reg_ind__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register base, GTA_Register index, uint8_t scale, int32_t offset);

/**
 * x86_64 instruction: LEAVE
 *
 * The instruction is used to restore the stack frame of the calling function.
 *
 * @param vector The vector in which to store the instruction.
 * @return True on success, false on failure.
 */
bool gta_leave__x86_64(GCU_Vector8 * vector);

/**
 * x86_64 instruction: MOV [base + index*scale + offset], reg
 *
 * @param vector The vector in which to store the instruction.
 * @param base The base register for the memory address.
 * @param index The index register for the memory address.
 * @param scale The scale for the index register.
 * @param offset The offset to add to the base register.
 * @param src The source register.
 * @return True on success, false on failure.
 */
bool gta_mov_ind_reg__x86_64(GCU_Vector8 * vector, GTA_Register base, GTA_Register index, uint8_t scale, int32_t offset, GTA_Register src);

/**
 * x86_64 instruction: MOV reg, reg
 *
 * @param vector The vector in which to store the instruction.
 * @param dst The destination register.
 * @param src The source register.
 * @return True on success, false on failure.
 */
bool gta_mov_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src);

/**
 * x86_64 instruction: MOV reg, imm
 *
 * If the immediate value is too large to fit in the specified register, the
 * function returns false.
 *
 * @param vector The vector in which to store the instruction.
 * @param dst The destination register.
 * @param value The immediate value.
 * @return True on success, false on failure.
 */
bool gta_mov_reg_imm__x86_64(GCU_Vector8 * vector, GTA_Register dst, int64_t value);

/**
 * x86_64 instruction: MOV reg, [base + index*scale + offset]
 *
 * @param vector The vector in which to store the instruction.
 * @param dst The destination register.
 * @param base The base register for the memory address.
 * @param index The index register for the memory address.
 * @param scale The scale for the index register.
 * @param offset The offset to add to the base register.
 * @return True on success, false on failure.
 */
bool gta_mov_reg_ind__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register base, GTA_Register index, uint8_t scale, int32_t offset);

/**
 * x86_64 instruction: MOVQ reg, reg
 *
 * @param vector The vector in which to store the instruction.
 * @param dst The destination register.
 * @param src The source register.
 * @return True on success, false on failure.
 */
bool gta_movq_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src);

/**
 * x86_64 instruction: OR reg, reg
 *
 * @param vector The vector in which to store the instruction.
 * @param dst The destination register.
 * @param src The source register.
 * @return True on success, false on failure.
 */
bool gta_or_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src);

/**
 * x86_64 instruction: POP reg
 *
 * @param vector The vector in which to store the instruction.
 * @param reg The register to pop.
 * @return True on success, false on failure.
 */
bool gta_pop_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg);

/**
 * x86_64 instruction: PUSH reg
 *
 * @param vector The vector in which to store the instruction.
 * @param reg The register to push.
 * @return True on success, false on failure.
 */
bool gta_push_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg);

/**
 * x86_64 instruction: RET
 *
 * @param vector The vector in which to store the instruction.
 * @return True on success, false on failure.
 */
bool gta_ret__x86_64(GCU_Vector8 * vector);

/**
 * x86_64 instruction: XOR reg
 *
 * @param vector The vector in which to store the instruction.
 * @param dst The destination register.
 * @param src The source register.
 * @return True on success, false on failure.
 */
bool gta_xor_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_PROGRAM_BINARY_H
