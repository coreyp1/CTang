/**
 * @file
 *
 * Test for consistency of the binary code generation.
 */

#include <string>
#include <gtest/gtest.h>
#include <cutil/vector.h>
#include <tang/program/binary.h>
#include <iostream>

using namespace std;


#define JIT(A,B) \
  { \
    GCU_Vector8 * v = gcu_vector8_create(0); \
    ASSERT_TRUE(v); \
    ASSERT_TRUE(A); \
    ASSERT_EQ(string((char *)v->data, v->count), string(B)); \
    gcu_vector8_destroy(v); \
  }


#define JIT_FAIL(A) \
  { \
    GCU_Vector8 * v = gcu_vector8_create(0); \
    ASSERT_TRUE(v); \
    ASSERT_FALSE(A); \
    gcu_vector8_destroy(v); \
  }


TEST(x86_64, and_reg_imm) {
  // Special cases for forms of the AX register.
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_AL, (int8_t)0xDE), "\x24\xDE");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_AX, (int16_t)0xDEAD), "\x66\x25\xAD\xDE");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_EAX, (int32_t)0xDEADBEEF), "\x25\xEF\xBE\xAD\xDE");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_RAX, (int32_t)0xDEADBEEF), "\x48\x25\xEF\xBE\xAD\xDE");
  // General case. r8, imm8
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_BL, (int8_t)0x7F), "\x80\xE3\x7F");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_BH, (int8_t)0xBE), "\x80\xE7\xBE");
  // General case. r16, imm16
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_CX, (int16_t)0x1EAD), "\x66\x81\xE1\xAD\x1E");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_DX, (int16_t)-0x1EAD), "\x66\x81\xE2\x53\xE1");
  // General case. r32, imm32
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_ESP, (int32_t)0x1EADBEEF), "\x81\xE4\xEF\xBE\xAD\x1E");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_EBP, (int32_t)-0x1EADBEEF), "\x81\xE5\x11\x41\x52\xE1");
  // General case. r64, imm32
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_R8, (int32_t)0x1EADBEEF), "\x49\x81\xE0\xEF\xBE\xAD\x1E");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_R9, (int32_t)-0x1EADBEEF), "\x49\x81\xE1\x11\x41\x52\xE1");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_RBX, (int32_t)0x1EADBEEF), "\x48\x81\xE3\xEF\xBE\xAD\x1E");
  // Sign extension. r16, imm8
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_BX, (int8_t)0x7F), "\x66\x83\xE3\x7F");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_BP, (int8_t)-0x7F), "\x66\x83\xE5\x81");
  // Sign extension. r32, imm8
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_ESI, (int8_t)0x7F), "\x83\xE6\x7F");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_EDI, (int8_t)-0x7F), "\x83\xE7\x81");
  // Sign extension. r64, imm8
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_R10, (int8_t)0x7F), "\x49\x83\xE2\x7F");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_R11, (int8_t)-0x7F), "\x49\x83\xE3\x81");
  JIT(gta_and_reg_imm__x86_64(v, GTA_REG_RBX, (int8_t)0x7F), "\x48\x83\xE3\x7F");
}


TEST(x86_64, call_reg) {
  // General case. r64
  JIT(gta_call_reg__x86_64(v, GTA_REG_RBX), "\xFF\xD3");
  JIT(gta_call_reg__x86_64(v, GTA_REG_R12), "\x41\xFF\xD4");
  // Invalid cases.
  JIT_FAIL(gta_call_reg__x86_64(v, GTA_REG_EAX));
  JIT_FAIL(gta_call_reg__x86_64(v, GTA_REG_BX));
  JIT_FAIL(gta_call_reg__x86_64(v, GTA_REG_CL));
  JIT_FAIL(gta_call_reg__x86_64(v, GTA_REG_DH));
}


TEST(x86_64, jnz) {
  // General case.
  JIT(gta_jnz__x86_64(v, 0x12345678), "\x0F\x85\x78\x56\x34\x12");
  JIT(gta_jnz__x86_64(v, -0x12345678), "\x0F\x85\x88\xA9\xCB\xED");
}


int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


