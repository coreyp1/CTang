#include <string.h>
#include <stdio.h>
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
#define REG_IS_YMM(reg) ((reg) >= GTA_REG_YMM0 && (reg) <= GTA_REG_YMM15)
#define REG_IS_NONE(reg) ((reg) == GTA_REG_NONE)


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
  // NONE
    0xFF
  };
  return reg < sizeof(reg_codes) ? reg_codes[reg] : 0;
}


bool gta_and_reg_imm__x86_64(GCU_Vector8 * vector, GTA_Register dst, int32_t immediate) {
  // https://www.felixcloutier.com/x86/and
  if (!REG_IS_INTEGER(dst) || !gta_binary_optimistic_increase(vector, 7)) {
    return false;
  }
  uint8_t dst_code = gta_binary_get_register_code__x86_64(dst);
  if (dst == GTA_REG_AL && immediate <= 0x7F && immediate >= -0x80) {
    // AL is a special case.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x24);
    vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)immediate);
    return true;
  }
  if (dst == GTA_REG_AX && immediate <= 0x7FFF && immediate >= -0x8000) {
    // AX is a special case.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x25);
    uint16_t downsized_src = (uint16_t)immediate;
    memcpy(&vector->data[vector->count], &downsized_src, 2);
    vector->count += 2;
    return true;
  }
  if (dst == GTA_REG_EAX) {
    // EAX is a special case.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x25);
    uint32_t downsized_src = (uint32_t)immediate;
    memcpy(&vector->data[vector->count], &downsized_src, 4);
    vector->count += 4;
    return true;
  }
  if (dst == GTA_REG_RAX) {
    // RAX is a special case.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x25);
    memcpy(&vector->data[vector->count], &immediate, 4);
    vector->count += 4;
    return true;
  }
  if (immediate <= 0x7F && immediate >= -0x80) {
    // 8-bit immediate.
    if (REG_IS_8BIT(dst)) {
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x80);
      vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
      vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)immediate);
      return true;
    }
    if (REG_IS_16BIT(dst)) {
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x83);
      vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
      vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)immediate);
      return true;
    }
    if (REG_IS_32BIT(dst)) {
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x83);
      vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
      vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)immediate);
      return true;
    }
    // 64-bit register.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((dst_code & 0x08) >> 3));
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x83);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
    vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)immediate);
    return true;
  }
  if (REG_IS_16BIT(dst) && immediate <= 0x7FFF && immediate >= -0x8000) {
    // 16-bit register, 16-bit immediate.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x81);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
    uint16_t downsized_src = (uint16_t)immediate;
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
    memcpy(&vector->data[vector->count], &immediate, 4);
    vector->count += 4;
    return true;
  }
  // 64-bit register, 32-bit immediate.
  vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((dst_code & 0x08) >> 3));
  vector->data[vector->count++] = GCU_TYPE8_UI8(0x81);
  vector->data[vector->count++] = GCU_TYPE8_UI8(0xE0 + (dst_code & 0x07));
  memcpy(&vector->data[vector->count], &immediate, 4);
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


bool gta_jnz__x86_64(GCU_Vector8 * vector, int32_t offset) {
  // https://www.felixcloutier.com/x86/jnz
  if (!gta_binary_optimistic_increase(vector, 6)) {
    return false;
  }
  vector->data[vector->count++] = GCU_TYPE8_UI8(0x0F);
  vector->data[vector->count++] = GCU_TYPE8_UI8(0x85);
  memcpy(&vector->data[vector->count], &offset, 4);
  vector->count += 4;
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


bool gta_mov_ind_reg__x86_64(GCU_Vector8 * vector, GTA_Register base, GTA_Register index, uint8_t scale, int32_t offset, GTA_Register src) {
  // https://www.felixcloutier.com/x86/mov
  if (!(REG_IS_INTEGER(src)
      && (REG_IS_INTEGER(base) || REG_IS_NONE(base))
      && ((REG_IS_INTEGER(index) && (scale == 1 || scale == 2 || scale == 4 || scale == 8))
        || (REG_IS_NONE(index) && (scale == 0))))
    || !gta_binary_optimistic_increase(vector, 8)) {
    return false;
  }
  uint8_t src_code = gta_binary_get_register_code__x86_64(src);
  uint8_t base_code = REG_IS_INTEGER(base) ? gta_binary_get_register_code__x86_64(base) : 0;
  uint8_t index_code = REG_IS_INTEGER(index) ? gta_binary_get_register_code__x86_64(index) : 0;
  bool offset_32 = offset > 0xFFFF || offset < -0xFFFF;
  bool rex_required = (REG_IS_64BIT(src) && (src_code & 0x08)) || (REG_IS_64BIT(base) && (base_code & 0x08)) || (REG_IS_64BIT(index) && (index_code & 0x08) && !REG_IS_NONE(index));

  // RIP-relative addressing.
  // e.g., mov [RIP + 0xDEADBEEF], rax
  if (REG_IS_NONE(base)) {
    if (REG_IS_8BIT(src)) {
      // 8-bit register has its own opcode.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x88);
    }
    else {
      if (REG_IS_64BIT(src)) {
        // REX prefix
        vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((src_code & 0x08) >> 1));
      }
      if (REG_IS_16BIT(src)) {
        // 16-bit prefix
        vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
      }
      // 16-bit, 32-bit, and 64-bit registers share the same opcode.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x89);
    }
    // Rip-relative *always* uses a 32-bit displacement.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x05 | ((src_code & 0x07) << 3));
    memcpy(&vector->data[vector->count], &offset, 4);
    vector->count += 4;
    return true;
  }

  // Base or Base + offset.  No Index.
  // e.g., mov [rbx + 0xDEADBEEF], rax
  // also  mov [rbx], rax
  // Note: RSP and R12 require a SIB byte.
  if (REG_IS_NONE(index) && (base != GTA_REG_RSP) && (base != GTA_REG_R12)) {
    if (REG_IS_16BIT(src)) {
      // 16-bit prefix
      // This must come before the REX byte (if any).
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    }
    if (rex_required) {
      // REX.WRXB prefix
      // Note: The REX byte must be here because either the src or the base
      // register is 64-bit.  W is only set if src is 64-bit.
      vector->data[vector->count++] = GCU_TYPE8_UI8((REG_IS_64BIT(src) ? 0x48 : 0x40) | ((src_code & 0x08) >> 1) | ((base_code & 0x08) >> 3));
    }
    if (REG_IS_8BIT(src)) {
      if (rex_required && (src == GTA_REG_AH || src == GTA_REG_BH || src == GTA_REG_CH || src == GTA_REG_DH)) {
        // AH, BH, CH, DH are not valid for 64-bit mode.
        return false;
      }
      // 8-bit register has its own opcode.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x88);
    }
    else {
      // 16-bit, 32-bit, and 64-bit registers share the same opcode.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x89);
    }
    // Note: RBP and R13 are used for RIP-relative addressing, so if they are
    // being referenced, then we must force an offset encoding, even if the
    // offset is zero.
    bool force_offset = offset || ((base_code & 0x07) == 0x05);
    vector->data[vector->count++] = GCU_TYPE8_UI8((force_offset ? offset_32 ? 0x80 : 0x40 : 0x00) | ((src_code & 0x07) << 3) | (base_code & 0x07));
    if (offset_32) {
      memcpy(&vector->data[vector->count], &offset, 4);
      vector->count += 4;
    }
    else if (force_offset) {
      vector->data[vector->count++] = GCU_TYPE8_UI8(offset);
    }
    // Else, no offset.  No need to write anything else.
    return true;
  }

  // All cases in which the index is a form of SP result in a weird encoding
  // that we don't support.
  if (index == GTA_REG_RSP || index == GTA_REG_ESP || index == GTA_REG_SP) {
    return false;
  }

  // Base + (Index * Scale) + Offset.
  // e.g., mov [rbx + rsi * 4 + 0xDEADBEEF], rax
  // also  mov [rbx + rsi * 4], rax
  // also  mov [rbx + rcx], rax
  if (REG_IS_16BIT(src)) {
    // 16-bit prefix
    // This must come before the REX byte (if any).
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
  }
  if (rex_required) {
    // REX.WRXB prefix
    // Note: The REX byte must be here because either the src, base, or index
    // register is 64-bit.  W is only set if src is 64-bit.
    vector->data[vector->count++] = GCU_TYPE8_UI8((REG_IS_64BIT(src) ? 0x48 : 0x40)
      | ((src_code & 0x08) >> 1)
      | ((base_code & 0x08) >> 3)
      | ((index_code & 0x08) >> 2));
  }
  if (REG_IS_8BIT(src)) {
    if (rex_required && (src == GTA_REG_AH || src == GTA_REG_BH || src == GTA_REG_CH || src == GTA_REG_DH)) {
      // AH, BH, CH, DH are not valid for 64-bit mode.
      return false;
    }
    // 8-bit register has its own opcode.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x88);
  }
  else {
    // 16-bit, 32-bit, and 64-bit registers share the same opcode.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x89);
  }
  // Note: RBP and R13 are used for a specific encoding that we do not make use
  // of, so if they are being referenced, then we must force an offset
  // encoding, even if the offset is zero.
  bool force_offset = offset || ((base_code & 0x07) == 0x05);
  vector->data[vector->count++] = GCU_TYPE8_UI8((force_offset ? offset_32 ? 0x84 : 0x44 : 0x04) | ((src_code & 0x07) << 3));
  vector->data[vector->count++] = GCU_TYPE8_UI8((
    scale < 2
      ? 0x00
      : scale == 2
        ? 0x40
        : scale == 4
          ? 0x80
          : 0xC0) | ((REG_IS_NONE(index) ? 0x04 : (index_code & 0x07)) << 3) | (base_code & 0x07));
  if (offset_32) {
    memcpy(&vector->data[vector->count], &offset, 4);
    vector->count += 4;
  }
  else if (force_offset) {
    vector->data[vector->count++] = GCU_TYPE8_UI8(offset);
  }
  // Else, no offset.  No need to write anything else.
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


bool gta_mov_reg_ind__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register base, GTA_Register index, uint8_t scale, int32_t offset) {
  // https://www.felixcloutier.com/x86/mov
  if (!(REG_IS_INTEGER(dst)
      && (REG_IS_INTEGER(base) || REG_IS_NONE(base))
      && ((REG_IS_INTEGER(index) && (scale == 1 || scale == 2 || scale == 4 || scale == 8))
        || (REG_IS_NONE(index) && (scale == 0))))
    || !gta_binary_optimistic_increase(vector, 8)) {
    return false;
  }
  uint8_t dst_code = gta_binary_get_register_code__x86_64(dst);
  uint8_t base_code = gta_binary_get_register_code__x86_64(base);
  uint8_t index_code = REG_IS_INTEGER(index) ? gta_binary_get_register_code__x86_64(index) : 0;
  bool offset_32 = offset > 0xFFFF || offset < -0xFFFF;

  // RIP-relative addressing.
  // e.g., mov al, [RIP + 0xDEADBEEF]
  if (REG_IS_NONE(base)) {
    if (REG_IS_8BIT(dst)) {
      // 8-bit register has its own opcode.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x8A);
    }
    else {
      if (REG_IS_64BIT(dst)) {
        // REX prefix
        vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((dst_code & 0x08) >> 1));
      }
      if (REG_IS_16BIT(dst)) {
        // 16-bit prefix
        vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
      }
      // 16-bit, 32-bit, and 64-bit registers share the same opcode.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x8B);
    }
    // Rip-relative *always* uses a 32-bit displacement.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x05 | ((dst_code & 0x07) << 3));
    memcpy(&vector->data[vector->count], &offset, 4);
    vector->count += 4;
    return true;
  }

  // Base or Base + offset.  No Index.
  // e.g., mov rax, [rbx + 0xDEADBEEF]
  // also  mov rax, [rbx]
  // Note: RSP and R12 require a SIB byte.
  if (REG_IS_NONE(index) && (base != GTA_REG_RSP) && (base != GTA_REG_R12)) {
    if (REG_IS_16BIT(dst)) {
      // 16-bit prefix
      // This must come before the REX byte (if any).
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    }
    if (REG_IS_64BIT(dst) || REG_IS_64BIT(base)) {
      // REX.WRXB prefix
      // Note: The REX byte must be here because either the dst or the base
      // register is 64-bit.  W is only set if dst is 64-bit.
      vector->data[vector->count++] = GCU_TYPE8_UI8((REG_IS_64BIT(dst) ? 0x48 : 0x40) | ((dst_code & 0x08) >> 1) | ((base_code & 0x08) >> 3));
    }
    if (REG_IS_8BIT(dst)) {
      if (dst == GTA_REG_AH || dst == GTA_REG_BH || dst == GTA_REG_CH || dst == GTA_REG_DH) {
        // AH, BH, CH, DH are not valid for 64-bit mode.
        return false;
      }
      // 8-bit register has its own opcode.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x8A);
    }
    else {
      // 16-bit, 32-bit, and 64-bit registers share the same opcode.
      vector->data[vector->count++] = GCU_TYPE8_UI8(0x8B);
    }
    // Note: RBP and R13 are used for RIP-relative addressing, so if they are
    // being referenced, then we must force an offset encoding, even if the
    // offset is zero.
    bool force_offset = offset || ((base_code & 0x07) == 0x05);
    vector->data[vector->count++] = GCU_TYPE8_UI8((force_offset ? offset_32 ? 0x80 : 0x40 : 0x00) | ((dst_code & 0x07) << 3) | (base_code & 0x07));
    if (offset_32) {
      memcpy(&vector->data[vector->count], &offset, 4);
      vector->count += 4;
    }
    else if (force_offset) {
      vector->data[vector->count++] = GCU_TYPE8_UI8(offset);
    }
    // Else, no offset.  No need to write anything else.
    return true;
  }

  // All cases in which the index is a form of SP result in a weird encoding
  // that we don't support.
  if (index == GTA_REG_RSP || index == GTA_REG_ESP || index == GTA_REG_SP) {
    return false;
  }

  // SIB required.
  if (REG_IS_16BIT(dst)) {
    // 16-bit prefix
    // This must come before the REX byte (if any).
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
  }
  if (REG_IS_64BIT(dst) || REG_IS_64BIT(base) || (REG_IS_64BIT(index) && !REG_IS_NONE(index))) {
    // REX.WRXB prefix
    // Note: The REX byte must be here because either the dst or the base
    // register is 64-bit.  W is only set if dst is 64-bit.
    vector->data[vector->count++] = GCU_TYPE8_UI8((REG_IS_64BIT(dst) ? 0x48 : 0x40)
      | ((dst_code & 0x08) >> 1)
      | (REG_IS_NONE(index) ? 0 : ((index_code & 0x08) >> 2))
      | ((base_code & 0x08) >> 3));
  }
  if (REG_IS_8BIT(dst)) {
    if (dst == GTA_REG_AH || dst == GTA_REG_BH || dst == GTA_REG_CH || dst == GTA_REG_DH) {
      // AH, BH, CH, DH are not valid for 64-bit mode.
      return false;
    }
    // 8-bit register has its own opcode.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x8A);
  }
  else {
    // 16-bit, 32-bit, and 64-bit registers share the same opcode.
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x8B);
  }
  // Note: RBP and R13 are used for a specific encoding that we do not make use
  // of, so if they are being referenced, then we must force an offset
  // encoding, even if the offset is zero.
  bool force_offset = offset || ((base_code & 0x07) == 0x05);
  vector->data[vector->count++] = GCU_TYPE8_UI8((force_offset ? offset_32 ? 0x84 : 0x44 : 0x04) | ((dst_code & 0x07) << 3));
  vector->data[vector->count++] = GCU_TYPE8_UI8((
    scale < 2
      ? 0x00
      : scale == 2
        ? 0x40
        : scale == 4
          ? 0x80
          : 0xC0) | ((REG_IS_NONE(index) ? 0x04 : (index_code & 0x07)) << 3) | (base_code & 0x07));
  if (offset_32) {
    memcpy(&vector->data[vector->count], &offset, 4);
    vector->count += 4;
  }
  else if (force_offset) {
    vector->data[vector->count++] = GCU_TYPE8_UI8(offset);
  }
  // Else, no offset.  No need to write anything else.
  return true;
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


bool gta_or_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src) {
  // https://www.felixcloutier.com/x86/or
  if (!REG_IS_INTEGER(dst) || !REG_IS_INTEGER(src) || !gta_binary_optimistic_increase(vector, 3)) {
    return false;
  }
  uint8_t src_code = gta_binary_get_register_code__x86_64(src);
  uint8_t dst_code = gta_binary_get_register_code__x86_64(dst);
  if (REG_IS_64BIT(src) && REG_IS_64BIT(dst)) {
    // REX prefix
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((src_code & 0x08) >> 1) | ((dst_code & 0x08) >> 3));
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x09);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x07) << 3) + (dst_code & 0x07));
    return true;
  }
  if (REG_IS_32BIT(src) && REG_IS_32BIT(dst)) {
    // 32-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x41);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x09);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x07) << 3) + (dst_code & 0x07));
    return true;
  }
  if (REG_IS_16BIT(src) && REG_IS_16BIT(dst)) {
    // 16-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x09);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x07) << 3) + (dst_code & 0x07));
    return true;
  }
  if (REG_IS_8BIT(src) && REG_IS_8BIT(dst)) {
    // 8-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x08);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x07) << 3) + (dst_code & 0x07));
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


bool gta_xor_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src) {
  // https://www.felixcloutier.com/x86/xor
  if (!REG_IS_INTEGER(dst) || !REG_IS_INTEGER(src) || !gta_binary_optimistic_increase(vector, 3)) {
    return false;
  }
  uint8_t src_code = gta_binary_get_register_code__x86_64(src);
  uint8_t dst_code = gta_binary_get_register_code__x86_64(dst);
  if (REG_IS_64BIT(src) && REG_IS_64BIT(dst)) {
    // REX prefix
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((src_code & 0x08) >> 1) | ((dst_code & 0x08) >> 3));
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x31);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x07) << 3) + (dst_code & 0x07));
    return true;
  }
  if (REG_IS_32BIT(src) && REG_IS_32BIT(dst)) {
    // 32-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x41);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x31);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x07) << 3) + (dst_code & 0x07));
    return true;
  }
  if (REG_IS_16BIT(src) && REG_IS_16BIT(dst)) {
    // 16-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x31);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x07) << 3) + (dst_code & 0x07));
    return true;
  }
  if (REG_IS_8BIT(src) && REG_IS_8BIT(dst)) {
    // 8-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x30);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xC0 + ((src_code & 0x07) << 3) + (dst_code & 0x07));
    return true;
  }
  return false;
}
