#include <string.h>
#include <tang/program/binary.h>

#define VECTOR_GROWTH_FACTOR ((double)1.5)

#define REG_IS_64BIT(reg) ((reg) <= GTA_REG_R15)
#define REG_IS_32BIT(reg) ((reg) <= GTA_REG_ESP && (reg) >= GTA_REG_EAX)
#define REG_IS_16BIT(reg) ((reg) <= GTA_REG_SP && (reg) >= GTA_REG_AX)
#define REG_IS_8BIT(reg) ((reg) <= GTA_REG_DH && (reg) >= GTA_REG_AL)


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
  };
  return reg < sizeof(reg_codes) ? reg_codes[reg] : 0;
}


bool gta_leave__x86_64(GCU_Vector8 * vector) {
  // https://www.felixcloutier.com/x86/leave
  if (!gta_binary_optimistic_increase(vector, 1)) {
    return false;
  }
  vector->data[vector->count++] = GCU_TYPE8_UI8(0xC9);
  return true;
}


bool gta_mov_reg_imm__x86_64(GCU_Vector8 * vector, GTA_Register dst, uint64_t src) {
  // https://www.felixcloutier.com/x86/mov
  if (!gta_binary_optimistic_increase(vector, 10)) {
    return false;
  }
  uint8_t dst_code = gta_binary_get_register_code__x86_64(dst);
  if (REG_IS_64BIT(dst)) {
    // REX prefix
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x48 | ((dst_code & 0x08) >> 3));
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xB8 + (dst_code & 0x7));
    memcpy(&vector->data[vector->count], &src, 8);
    vector->count += 8;
    return true;
  }
  if (REG_IS_32BIT(dst)) {
    // 32-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xB8 + (dst_code & 0x7));
    uint32_t downsized_src = (uint32_t)src;
    memcpy(&vector->data[vector->count], &downsized_src, 4);
    vector->count += 4;
    return true;
  }
  if (REG_IS_16BIT(dst)) {
    // 16-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0x66);
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xB8 + (dst_code & 0x7));
    uint16_t downsized_src = (uint16_t)src;
    memcpy(&vector->data[vector->count], &downsized_src, 2);
    vector->count += 2;
    return true;
  }
  if (REG_IS_8BIT(dst)) {
    // 8-bit register
    vector->data[vector->count++] = GCU_TYPE8_UI8(0xB0 + (dst_code & 0x7));
    vector->data[vector->count++] = GCU_TYPE8_UI8((uint8_t)src);
    return true;
  }

  return false;
}


bool gta_mov_reg_reg__x86_64(GCU_Vector8 * vector, GTA_Register dst, GTA_Register src) {
  // https://www.felixcloutier.com/x86/mov
  if (!gta_binary_optimistic_increase(vector, 3)) {
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


bool gta_pop_reg__x86_64(GCU_Vector8 * vector, GTA_Register reg) {
  // https://www.felixcloutier.com/x86/pop
  if (!gta_binary_optimistic_increase(vector, 2)
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
  if (!gta_binary_optimistic_increase(vector, 2)
    || !REG_IS_64BIT(reg)) {
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
