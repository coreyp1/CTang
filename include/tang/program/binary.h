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
 * A representation for the condition codes that may be available in the target
 * architecture.
 */
typedef enum GTA_Condition_Code {
  GTA_CC_A,
  GTA_CC_AE,
  GTA_CC_B,
  GTA_CC_BE,
  GTA_CC_C,
  GTA_CC_E,
  GTA_CC_G,
  GTA_CC_GE,
  GTA_CC_L,
  GTA_CC_LE,
  GTA_CC_O,
  GTA_CC_P,
  GTA_CC_PE,
  GTA_CC_PO,
  GTA_CC_S,
  GTA_CC_Z,
  GTA_CC_NA,
  GTA_CC_NAE,
  GTA_CC_NB,
  GTA_CC_NBE,
  GTA_CC_NC,
  GTA_CC_NE,
  GTA_CC_NG,
  GTA_CC_NGE,
  GTA_CC_NL,
  GTA_CC_NLE,
  GTA_CC_NO,
  GTA_CC_NP,
  GTA_CC_NS,
  GTA_CC_NZ,
} GTA_Condition_Code;

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
 * Helper function to call a function in the binary.
 *
 * This will add the commands to preserve the stack and call the function.
 * Afterward, the stack will be restored.
 *
 * @param vector The vector in which to store the instructions.
 * @param function The function to call.
 * @return True on success, false on failure.
 */
bool gta_binary_call__x86_64(GCU_Vector8 * vector, uint64_t function);

/**
 * Helper function to call a function in the binary.  This variation requires
 * that the function address has already been loaded into the specified
 * register.
 *
 * This will add the commands to preserve the stack and call the function.
 * Afterward, the stack will be restored.
 *
 * @param vector The vector in which to store the instructions.
 * @param reg The register holding the function address.
 * @return True on success, false on failure.
 */
bool gta_binary_call_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg);

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
 * x86_64 instruction: CMOVcc reg, reg
 *
 * The two registers must be the same size.
 *
 * @param vector The vector in which to store the instruction.
 * @param condition The condition code.
 * @param dst The destination register.
 * @param src The source register.
 * @return True on success, false on failure.
 */
bool gta_cmovcc_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Condition_Code condition, GTA_Register dst, GTA_Register src);

/**
 * x86_64 instruction: CMP byte ind, imm8
 *
 * @param vector The vector in which to store the instruction.
 * @param base The base register for the memory address.
 * @param index The index register for the memory address.
 * @param scale The scale for the index register.
 * @param offset The offset to add to the base register.
 * @param immediate The immediate value (8-bit).
 * @return True on success, false on failure.
 */
bool gta_cmp_ind8_imm8__x86_64(GCU_Vector8 * vector, GTA_Register base, GTA_Register index, uint8_t scale, int32_t offset, int8_t immediate);

/**
 * x86_64 instruction: CMP reg, reg
 *
 * @param vector The vector in which to store the instruction.
 * @param op1 The first operand register.
 * @param op2 The second operand register.
 * @return True on success, false on failure.
 */
bool gta_cmp_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register op1, GTA_Register op2);

/**
 * x86_64 instruction: Jcc offset
 *
 * The instruction jumps either forward or backward by the given offset.  The
 * offset is relative to the next instruction and is limited to a signed 32-bit
 * integer.
 *
 * @param vector The vector in which to store the instruction.
 * @param condition The condition code.
 * @param offset The offset to jump to.
 * @return True on success, false on failure.
 */
bool gta_jcc__x86_64(GCU_Vector8 * vector, GTA_Condition_Code condition, int32_t offset);

/**
 * x86_64 instruction: JMP offset
 *
 * The instruction jumps either forward or backward by the given offset.  The
 * offset is relative to the next instruction and is limited to a signed 32-bit
 * integer.
 *
 * @param vector The vector in which to store the instruction.
 * @param offset The offset to jump to.
 * @return True on success, false on failure.
 */
bool gta_jmp__x86_64(GCU_Vector8 * vector, int32_t offset);

/**
 * x86_64 instruction: LEA reg, [base + index*scale + offset]
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
 * x86_64 instruction: MOV byte ptr [base + index*scale + offset], imm8
 *
 * @param vector The vector in which to store the instruction.
 * @param base The base register for the memory address.
 * @param index The index register for the memory address.
 * @param scale The scale for the index register.
 * @param offset The offset to add to the base register.
 * @param immediate The immediate value (8-bit).
 * @return True on success, false on failure.
 */
bool gta_mov_ind8_imm8__x86_64(GCU_Vector8 * vector, GTA_Register base, GTA_Register index, uint8_t scale, int32_t offset, int8_t immediate);

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
 * x86_64 instruction: TEST reg, reg
 *
 * @param vector The vector in which to store the instruction.
 * @param op1 The first operand register.
 * @param op2 The second operand register.
 * @return True on success, false on failure.
 */
bool gta_test_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register op1, GTA_Register op2);

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
