#include <string.h>
#include <tang/program/binary.h>

#define VECTOR_GROWTH_FACTOR ((double)1.5)

#define REG_IS_256BIT(reg) ((reg) >= GTA_REG_YMM0 && (reg) <= GTA_REG_YMM15)
#define REG_IS_128BIT(reg) ((reg) >= GTA_REG_XMM0 && (reg) <= GTA_REG_XMM15)
#define REG_IS_64BIT(reg) ((reg) <= GTA_REG_R15 || ((reg) >= GTA_REG_MMX0 && (reg) <= GTA_REG_MMX7))
#define REG_IS_32BIT(reg) ((reg) <= GTA_REG_ESP && (reg) >= GTA_REG_EAX)
#define REG_IS_16BIT(reg) ((reg) <= GTA_REG_SP && (reg) >= GTA_REG_AX)
#define REG_IS_8BIT(reg) ((reg) <= GTA_REG_DH && (reg) >= GTA_REG_AL)
#define REG_IS_INTEGER(reg) ((reg) <= GTA_REG_DH)
#define REG_IS_FLOAT(reg) ((reg) >= GTA_REG_XMM0)
#define REG_IS_XMM(reg) ((reg) >= GTA_REG_XMM0 && (reg) <= GTA_REG_XMM15)


bool gta_binary_optimistic_increase(GCU_Vector8 * vector, size_t additional) {
  return gcu_vector8_reserve(vector, vector->count + additional > vector->capacity
    ? vector->capacity * VECTOR_GROWTH_FACTOR > vector->count + additional
      ? vector->capacity * VECTOR_GROWTH_FACTOR
      : vector->count + additional
    : vector->capacity);
}


uint8_t gta_binary_get_register_code__x86_64(GTA_Register reg) {
  const uint8_t reg_codes[] = {
  // RAX,  RBX,  RCX,  RDX,  RSI,  RDI,  RBP,  RSP,
    0x00, 0x03, 0x01, 0x02, 0x06, 0x07, 0x05, 0x04,
  //  R8,   R9,  R10,  R11,  R12,  R13,  R14,  R15,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  // EAX,  EBX,  ECX,  EDX,  ESI,  EDI,  EBP,  ESP,
    0x00, 0x03, 0x01, 0x02, 0x06, 0x07, 0x05, 0x04,
  //  AX,   BX,   CX,   DX,   SI,   DI,   BP,   SP,
    0x00, 0x03, 0x01, 0x02, 0x06, 0x07, 0x05, 0x04,
  //  AL,   BL,   CL,   DL,   AH,   BH,   CH,   DH,
    0x00, 0x03, 0x01, 0x02, 0x04, 0x07, 0x05, 0x06,
  //MMX0, MMX1, MMX2, MMX3, MMX4, MMX5, MMX6, MMX7,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  //XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  //XMM8, XMM9, XMM10,XMM11,XMM12,XMM13,XMM14,XMM15,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  //YMM0, YMM1, YMM2, YMM3, YMM4, YMM5, YMM6, YMM7,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  //YMM8, YMM9, YMM10,YMM11,YMM12,YMM13,YMM14,YMM15,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  };
  return reg < sizeof(reg_codes) ? reg_codes[reg] : 0;
}


bool gta_and_reg_imm__x86_64(GCU_Vector8 * vector, GTA_Register dst, int32_t src) {
  // https://www.felixcloutier.com/x86/and
  if (!REG_IS_INTEGER(dst) || !gta_binary_optimistic_increase(vector, 7)) {
    return false;
  }
  uint8_t dst_code = gta_binary_get_register_code__x86_64(dst);
  if (dst == GTA_REG_AL && src <= 0x7F && src >= -0x80) {
    // AL is a special case.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x24);
    vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)src);
    return true;
  }
  if (dst == GTA_REG_AX && src <= 0x7FFF && src >= -0x8000) {
    // AX is a special case.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x25);
    uint16_t downsized_src = (uint16_t)src;
    memcpy(&vector->data[vector->count], &downsized_src, 2);
    vector->count += 2;
    return true;
  }
  if (dst == GTA_REG_EAX) {
    // EAX is a special case.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x25);
    uint32_t downsized_src = (uint32_t)src;
    memcpy(&vector->data[vector->count], &downsized_src, 4);
    vector->count += 4;
    return true;
  }
  if (dst == GTA_REG_RAX) {
    // RAX is a special case.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x25);
    memcpy(&vector->data[vector->count], &src, 4);
    vector->count += 4;
    return true;
  }
  if (src <= 0x7F && src >= -0x80) {
    // 8-bit immediate.
    if (REG_IS_8BIT(dst)) {
      if (dst == GTA_REG_AH || dst == GTA_REG_BH || dst == GTA_REG_CH || dst == GTA_REG_DH) {
        // AH, BH, CH, DH are not valid for AND in 64-bit mode.
        return false;
      }
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x80);
      vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
      vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)src);
      return true;
    }
    if (REG_IS_16BIT(dst)) {
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x83);
      vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
      vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)src);
      return true;
    }
    if (REG_IS_32BIT(dst)) {
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x83);
      vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
      vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)src);
      return true;
    }
    // 64-bit register.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((dst_code & 0x08) >> 3));
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x83);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
    vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)src);
    return true;
  }
  if (REG_IS_16BIT(dst) && src <= 0x7FFF && src >= -0x8000) {
    // 16-bit register, 16-bit immediate.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x81);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
    uint16_t downsized_src = (uint16_t)src;
    memcpy(&vector->data[vector->count], &downsized_src, 2);
    vector->count += 2;
    return true;
  }
  if (REG_IS_8BIT(dst) || REG_IS_16BIT(dst)){
    // No valid encodings left for 8-bit or 16-bit registers.
    return false;
  }
  if (REG_IS_32BIT(dst)) {
    // 32-bit register, 32-bit immediate.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x81);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
    memcpy(&vector->data[vector->count], &src, 4);
    vector->count += 4;
    return true;
  }
  // 64-bit register, 32-bit immediate.
  vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((dst_code & 0x08) >> 3));
  vector->data[vector->count++] = GCU_TYPE8_UI8(0x81);
  vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
  memcpy(&vector->data[vector->count], &src, 4);
  vector->count += 4;
  return true;
}


bool gta_call_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg) {
  // https://www.felixcloutier.com/x86/call
  if (!REG_IS_INTEGER(reg) || !gta_binary_optimistic_increase(vector, 3)) {
    return false;
  }
  if (!REG_IS_64BIT(reg)) {
    // Not a valid register for CALL in 64-bit mode.
    return false;
  }
  uint8_t code = gta_binary_get_register_code__x86_64(reg);
  if (code & 0x08) {
    // REX prefix
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x41);
  }
  vector->data[vector->count++] = GCU_TYPE8_UI8(0xFF);
  vector->data[vector->count++] = GCU_TYPE8_UI8(0xD0 + (code & 0x07));
  return true;
}


bool gta_lea_reg_mem__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register base, int32_t offset) {
  // https://www.felixcloutier.com/x86/lea
  if (!REG_IS_INTEGER(dst) || !gta_binary_optimistic_increase(vector, 8)) {
    return false;
  }
  uint8_t dst_code = gta_binary_get_register_code__x86_64(dst);
  uint8_t base_code = gta_binary_get_register_code__x86_64(base);
  // These are not valid for LEA.
  if (REG_IS_8BIT(dst) || REG_IS_8BIT(base) || REG_IS_16BIT(base)) {
    return false;
  }
  // Prefixes.
  if (REG_IS_32BIT(base)) {
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x67);
  }
  if (REG_IS_16BIT(dst)) {
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
  }
  if (REG_IS_64BIT(dst) || REG_IS_64BIT(base)) {
    // REX prefix.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((dst_code & 0x08) >> 1) | ((base_code & 0x08) >> 3));
  }
  // LEA opcode.
  vector->data[vector->count++] = GCU_TYPE8_UI8(0x8D);
  // ModR/M byte.
  uint8_t modrm = ((dst_code & 0x7) << 3) + (base_code & 0x7);
  bool sib = false;
  // Determine the mode for the modrm byte.
  if (offset == 0) {
    if (base == GTA_REG_RBP || base == GTA_REG_ESP || base == GTA_REG_SP || base == GTA_REG_R12) {
      // See https://wiki.osdev.org/X86-64_Instruction_Encoding
      sib = true;
    }
  }
  if (offset > 0xFFFF || offset < -0xFFFF || dst == GTA_REG_RBP || dst == GTA_REG_ESP) {
    // 32-bit displacement.
    modrm |= 0x80;
    vector->data[vector->count++] = GCU_TYPE8_UI8(modrm);
    if (sib) {
      // SIB byte.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x20 | (base_code & 0x07));
    }
    memcpy(&vector->data[vector->count], &offset, 4);
    vector->count += 4;
  }
  else if (offset || sib || dst == GTA_REG_R13) {
    // 8-bit displacement.
    modrm |= 0x40;
    vector->data[vector->count++] = GCU_TYPE8_UI8(modrm);
    if (sib) {
      // SIB byte.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x20 | (base_code & 0x07));
    }
    vector->data[vector->count++] = GCU_TYPE8_UI8(offset);
  }
  else {
    // No displacement.
    vector->data[vector->count++] = GCU_TYPE8_UI8(modrm);
  }

  return true;
}


bool gta_leave__x86_64(GCU_Vector8 * vector) {
  // https://www.felixcloutier.com/x86/leave
  if (!gta_binary_optimistic_increase(vector, 1)) {
    return false;
  }
  vector->data[vector->count++] = GCU_TYPE8_UI8(0xC9);
  return true;
}


bool gta_mov_reg_imm__x86_64(GCU_Vector8 * vector, GTA_Register dst, int64_t value) {
  // https://www.felixcloutier.com/x86/mov
  if (!REG_IS_INTEGER(dst) || !gta_binary_optimistic_increase(vector, 10)) {
    return false;
  }
  uint8_t dst_code = gta_binary_get_register_code__x86_64(dst);
  if (REG_IS_64BIT(dst)) {
    // REX prefix
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((dst_code & 0x08) >> 3));
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xB8 + (dst_code & 0x7));
    memcpy(&vector->data[vector->count], &value, 8);
    vector->count += 8;
    return true;
  }
  if (REG_IS_32BIT(dst) && value <= 0x7FFFFFFF && value >= -0x80000000) {
    // 32-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xB8 + (dst_code & 0x7));
    int32_t downsized_src = (int32_t)value;
    memcpy(&vector->data[vector->count], &downsized_src, 4);
    vector->count += 4;
    return true;
  }
  if (REG_IS_16BIT(dst) && value <= 0x7FFF && value >= -0x8000) {
    // 16-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xB8 + (dst_code & 0x7));
    int16_t downsized_src = (int16_t)value;
    memcpy(&vector->data[vector->count], &downsized_src, 2);
    vector->count += 2;
    return true;
  }
  if (REG_IS_8BIT(dst) && value <= 0x7F && value >= -0x80) {
    // 8-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xB0 + (dst_code & 0x7));
    vector->data[vector->count++] = GCU_TYPE8_I8((int8_t)value);
    return true;
  }
  return false;
}


bool gta_mov_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src) {
  // https://www.felixcloutier.com/x86/mov
  if (!REG_IS_INTEGER(dst) || !REG_IS_INTEGER(src) || !gta_binary_optimistic_increase(vector, 3)) {
    return false;
  }
  if (REG_IS_8BIT(dst) || REG_IS_8BIT(src)) {
    return false;
  }
  uint8_t src_code = gta_binary_get_register_code__x86_64(src);
  uint8_t dst_code = gta_binary_get_register_code__x86_64(dst);
  if (REG_IS_64BIT(src) && REG_IS_64BIT(dst)) {
    // REX prefix
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((src_code & 0x08) >> 1) | ((dst_code & 0x08) >> 3));
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x89);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x7) << 3) + (dst_code & 0x7));
    return true;
  }
  if (REG_IS_32BIT(src) && REG_IS_32BIT(dst)) {
    // 32-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x41);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x89);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x7) << 3) + (dst_code & 0x7));
    return true;
  }
  if (REG_IS_16BIT(src) && REG_IS_16BIT(dst)) {
    // 16-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x89);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x7) << 3) + (dst_code & 0x7));
    return true;
  }
  if (REG_IS_8BIT(src) && REG_IS_8BIT(dst)) {
    // 8-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x88);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x7) << 3) + (dst_code & 0x7));
    return true;
  }

  return false;
}


bool gta_movq_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src) {
  // https://www.felixcloutier.com/x86/movq
  if (!gta_binary_optimistic_increase(vector, 5)) {
    return false;
  }
  if (REG_IS_64BIT(src) && REG_IS_INTEGER(src) && REG_IS_XMM(dst)) {
    // ex: movq xmm0, rax
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((src & 0x08) >> 1) | ((dst & 0x08) >> 3));
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x0F);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x6E);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src & 0x07) << 3) + (dst & 0x07));
    return true;
  }
  else if (REG_IS_XMM(src) && REG_IS_64BIT(dst) && REG_IS_INTEGER(dst)) {
    // ex: movq rax, xmm0
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((src & 0x08) >> 1) | ((dst & 0x08) >> 3));
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x0F);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x7E);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((dst & 0x07) << 3) + (src & 0x07));
    return true;
  }
  return false;
}


bool gta_pop_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg) {
  // https://www.felixcloutier.com/x86/pop
  if (!REG_IS_INTEGER(reg) || !gta_binary_optimistic_increase(vector, 2)
    || !REG_IS_64BIT(reg)) {
    return false;
  }
  uint8_t code = gta_binary_get_register_code__x86_64(reg);
  if (code & 0x08) {
    // REX prefix
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x41);
  }
  vector->data[vector->count++] = GCU_TYPE8_UI8(0x58 + (gta_binary_get_register_code__x86_64(reg) & 0x07));
  return true;
}


bool gta_push_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg) {
  // https://www.felixcloutier.com/x86/push
  if (!REG_IS_INTEGER(reg) || !REG_IS_64BIT(reg) || !gta_binary_optimistic_increase(vector, 2)) {
    return false;
  }
  uint8_t code = gta_binary_get_register_code__x86_64(reg);
  if (code & 0x08) {
    // REX prefix
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x41);
  }
  vector->data[vector->count++] = GCU_TYPE8_UI8(0x50 + (gta_binary_get_register_code__x86_64(reg) & 0x07));
  return true;
}


bool gta_ret__x86_64(GCU_Vector8 * vector) {
  // https://www.felixcloutier.com/x86/ret
  if (!gta_binary_optimistic_increase(vector, 1)) {
    return false;
  }
  vector->data[vector->count++] = GCU_TYPE8_UI8(0xC3);
  return true;
}
