/**
 * @file
 *
 * Test for consistency of the binary code generation.
 */

#include <stdint.h>
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

// x86_64 Assembly code checked at: https://defuse.ca/online-x86-assembler.htm
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


TEST(x86_64, lea_reg_ind) {
  // General case. r16, m16
  JIT(gta_lea_reg_ind__x86_64(v, GTA_REG_CX, GTA_REG_RBX, GTA_REG_RDX, 4, 42), "\x66\x8D\x4C\x93\x2A");
  JIT(gta_lea_reg_ind__x86_64(v, GTA_REG_DX, GTA_REG_RDX, GTA_REG_RCX, 1, 42), "\x66\x8D\x54\x0A\x2A");
  JIT(gta_lea_reg_ind__x86_64(v, GTA_REG_AX, GTA_REG_R9, GTA_REG_RAX, 2, 0x7EADBEEF), "\x66\x41\x8D\x84\x41\xEF\xBE\xAD\x7E");
  // General case. r32, m32
  JIT(gta_lea_reg_ind__x86_64(v, GTA_REG_ESP, GTA_REG_RCX, GTA_REG_RDX, 4, 42), "\x8D\x64\x91\x2A");
  JIT(gta_lea_reg_ind__x86_64(v, GTA_REG_EBP, GTA_REG_RDX, GTA_REG_RCX, 1, 42), "\x8D\x6C\x0A\x2A");
  JIT(gta_lea_reg_ind__x86_64(v, GTA_REG_ESI, GTA_REG_R9, GTA_REG_RAX, 2, 0x7EADBEEF), "\x41\x8D\xB4\x41\xEF\xBE\xAD\x7E");
  // General case. r64, m64
  JIT(gta_lea_reg_ind__x86_64(v, GTA_REG_RBX, GTA_REG_RCX, GTA_REG_RDX, 4, 42), "\x48\x8D\x5C\x91\x2A");
  JIT(gta_lea_reg_ind__x86_64(v, GTA_REG_R9, GTA_REG_RDX, GTA_REG_RCX, 1, 42), "\x4C\x8D\x4C\x0A\x2A");
  JIT(gta_lea_reg_ind__x86_64(v, GTA_REG_R9, GTA_REG_R9, GTA_REG_RAX, 2, 0x7EADBEEF), "\x4D\x8D\x8C\x41\xEF\xBE\xAD\x7E");
  // Failures. SIB registers must be 64-bit.
  JIT_FAIL(gta_lea_reg_ind__x86_64(v, GTA_REG_EAX, GTA_REG_BX, GTA_REG_RDX, 4, 42));
  JIT_FAIL(gta_lea_reg_ind__x86_64(v, GTA_REG_EAX, GTA_REG_RBX, GTA_REG_EDX, 4, 42));
}


TEST(x86_64, leave) {
  // General case.
  JIT(gta_leave__x86_64(v), "\xC9");
}


TEST(x86_64, mov_ind_reg) {
  // General case. m8, r8
  // mov byte ptr [base + index * scale + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RCX, 4, 42, GTA_REG_BL), "\x88\x5C\x88\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RDX, 1, 42, GTA_REG_BH), "\x88\x7C\x10\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RDX, 2, 0x7EADBEEF, GTA_REG_DH), "\x88\xB4\x50\xEF\xBE\xAD\x7E");
  JIT_FAIL(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_R8, 1, 42, GTA_REG_BH));
  // mov byte ptr [base + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RBX, GTA_REG_NONE, 0, 13, GTA_REG_CL), "\x88\x4b\x0D");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, 42, GTA_REG_BH), "\x88\x78\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R10, GTA_REG_NONE, 0, 0x01234567, GTA_REG_DL), "\x41\x88\x92\x67\x45\x23\x01");
  JIT_FAIL(gta_mov_ind_reg__x86_64(v, GTA_REG_R8, GTA_REG_NONE, 0, 42, GTA_REG_BH));
  // mov byte ptr [base], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, 0, GTA_REG_BL), "\x88\x18");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RBX, GTA_REG_NONE, 0, 0, GTA_REG_AH), "\x88\x23");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R10, GTA_REG_NONE, 0, 0, GTA_REG_CL), "\x41\x88\x0A");
  JIT_FAIL(gta_mov_ind_reg__x86_64(v, GTA_REG_R8, GTA_REG_NONE, 0, 0, GTA_REG_BH));
  // mov byte ptr [RIP + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, 0x01234567, GTA_REG_DH), "\x88\x35\x67\x45\x23\x01");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, 0, GTA_REG_BH), string("\x88\x3D\x00\x00\x00\x00", 6));
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, -42, GTA_REG_AL), "\x88\x05\xD6\xFF\xFF\xFF");
  // Special cases for R12, which forces the use of the SIB byte.
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, 0, GTA_REG_AL), "\x41\x88\x04\x24");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, 0x5A5A5A5A, GTA_REG_BL), "\x41\x88\x9C\x24\x5A\x5A\x5A\x5A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_R8, 1, 1, GTA_REG_CL), "\x43\x88\x4C\x04\x01");
  JIT_FAIL(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, 0, GTA_REG_BH));

  // General case. m16, r16
  // mov word ptr [base + index * scale + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RCX, 4, 42, GTA_REG_CX), "\x66\x89\x4C\x88\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RDX, 1, 42, GTA_REG_DX), "\x66\x89\x54\x10\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RDX, 2, 0x7EADBEEF, GTA_REG_DX), "\x66\x89\x94\x50\xEF\xBE\xAD\x7E");
  // mov word ptr [base + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RBX, GTA_REG_NONE, 0, 13, GTA_REG_DX), "\x66\x89\x53\x0D");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, 42, GTA_REG_DX), "\x66\x89\x50\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R10, GTA_REG_NONE, 0, 0x01234567, GTA_REG_DX), "\x66\x41\x89\x92\x67\x45\x23\x01");
  // mov word ptr [base], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, 0, GTA_REG_DX), "\x66\x89\x10");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RBX, GTA_REG_NONE, 0, 0, GTA_REG_DX), "\x66\x89\x13");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R10, GTA_REG_NONE, 0, 0, GTA_REG_DX), "\x66\x41\x89\x12");
  // mov word ptr [RIP + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, 0x01234567, GTA_REG_DX), "\x66\x89\x15\x67\x45\x23\x01");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, 0, GTA_REG_DX), string("\x66\x89\x15\x00\x00\x00\x00", 7));
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, -42, GTA_REG_DX), "\x66\x89\x15\xD6\xFF\xFF\xFF");
  // Special cases for R12, which forces the use of the SIB byte.
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, 0, GTA_REG_DX), "\x66\x41\x89\x14\x24");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, 0x5A5A5A5A, GTA_REG_DX), "\x66\x41\x89\x94\x24\x5A\x5A\x5A\x5A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_R8, 1, 1, GTA_REG_DX), "\x66\x43\x89\x54\x04\x01");

  // General case. m32, r32
  // mov dword ptr [base + index * scale + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RCX, 4, 42, GTA_REG_ECX), "\x89\x4C\x88\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RDX, 1, 42, GTA_REG_EDX), "\x89\x54\x10\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RDX, 2, 0x7EADBEEF, GTA_REG_EDX), "\x89\x94\x50\xEF\xBE\xAD\x7E");
  // mov dword ptr [base + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RBX, GTA_REG_NONE, 0, 13, GTA_REG_EDX), "\x89\x53\x0D");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, 42, GTA_REG_EDX), "\x89\x50\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R10, GTA_REG_NONE, 0, 0x01234567, GTA_REG_EDX), "\x41\x89\x92\x67\x45\x23\x01");
  // mov dword ptr [base], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, 0, GTA_REG_EDX), "\x89\x10");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RBX, GTA_REG_NONE, 0, 0, GTA_REG_EDX), "\x89\x13");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R10, GTA_REG_NONE, 0, 0, GTA_REG_EDX), "\x41\x89\x12");
  // mov dword ptr [RIP + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, 0x01234567, GTA_REG_EDX), "\x89\x15\x67\x45\x23\x01");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, 0, GTA_REG_EDX), string("\x89\x15\x00\x00\x00\x00", 6));
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, -42, GTA_REG_EDX), "\x89\x15\xD6\xFF\xFF\xFF");
  // Special cases for R12, which forces the use of the SIB byte.
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, 0, GTA_REG_EDX), "\x41\x89\x14\x24");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, 0x5A5A5A5A, GTA_REG_EDX), "\x41\x89\x94\x24\x5A\x5A\x5A\x5A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_R8, 1, 1, GTA_REG_EDX), "\x43\x89\x54\x04\x01");

  // General case. m64, r64
  // mov qword ptr [base + index * scale + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RCX, 4, 42, GTA_REG_RCX), "\x48\x89\x4C\x88\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RDX, 1, 42, GTA_REG_RDX), "\x48\x89\x54\x10\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RDX, 2, 0x7EADBEEF, GTA_REG_RDX), "\x48\x89\x94\x50\xEF\xBE\xAD\x7E");
  // mov qword ptr [base + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RBX, GTA_REG_NONE, 0, 13, GTA_REG_RDX), "\x48\x89\x53\x0D");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, 42, GTA_REG_RDX), "\x48\x89\x50\x2A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R10, GTA_REG_NONE, 0, 0x01234567, GTA_REG_RDX), "\x49\x89\x92\x67\x45\x23\x01");
  // mov qword ptr [base], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, 0, GTA_REG_RDX), "\x48\x89\x10");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_RBX, GTA_REG_NONE, 0, 0, GTA_REG_RDX), "\x48\x89\x13");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R10, GTA_REG_NONE, 0, 0, GTA_REG_RDX), "\x49\x89\x12");
  // mov qword ptr [RIP + offset], src
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, 0x01234567, GTA_REG_RDX), "\x48\x89\x15\x67\x45\x23\x01");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, 0, GTA_REG_RDX), string("\x48\x89\x15\x00\x00\x00\x00", 7));
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_NONE, GTA_REG_NONE, 0, -42, GTA_REG_RDX), "\x48\x89\x15\xD6\xFF\xFF\xFF");
  // Special cases for R12, which forces the use of the SIB byte.
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, 0, GTA_REG_RDX), "\x49\x89\x14\x24");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_NONE, 0, 0x5A5A5A5A, GTA_REG_RDX), "\x49\x89\x94\x24\x5A\x5A\x5A\x5A");
  JIT(gta_mov_ind_reg__x86_64(v, GTA_REG_R12, GTA_REG_R8, 1, 1, GTA_REG_RDX), "\x4B\x89\x54\x04\x01");

  // Failures. SIB registers must be 64-bit.
  JIT_FAIL(gta_mov_ind_reg__x86_64(v, GTA_REG_RAX, GTA_REG_BX, 4, 42, GTA_REG_CL));
  JIT_FAIL(gta_mov_ind_reg__x86_64(v, GTA_REG_EAX, GTA_REG_RBX, 4, 42, GTA_REG_CL));
}


TEST(x86_64, mov_reg_imm) {
  // General case. r8, imm8
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_AL, 0x7F), "\xB0\x7F");
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_BH, -0x42), "\xB7\xBE");
  JIT_FAIL(gta_mov_reg_imm__x86_64(v, GTA_REG_AL, (int64_t)INT8_MAX + 1));
  JIT_FAIL(gta_mov_reg_imm__x86_64(v, GTA_REG_BH, (int64_t)INT8_MIN - 1));
  // General case. r16, imm16
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_CX, 0x1EAD), "\x66\xB9\xAD\x1E");
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_DX, -0x1EAD), "\x66\xBA\x53\xE1");
  JIT_FAIL(gta_mov_reg_imm__x86_64(v, GTA_REG_CX, (int64_t)INT16_MAX + 1));
  JIT_FAIL(gta_mov_reg_imm__x86_64(v, GTA_REG_DX, (int64_t)INT16_MIN - 1));
  // General case. r32, imm32
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_ESP, 0x1EADBEEF), "\xBC\xEF\xBE\xAD\x1E");
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_EBP, -0x1EADBEEF), "\xBD\x11\x41\x52\xE1");
  JIT_FAIL(gta_mov_reg_imm__x86_64(v, GTA_REG_ESP, (int64_t)INT32_MAX + 1));
  JIT_FAIL(gta_mov_reg_imm__x86_64(v, GTA_REG_EBP, (int64_t)INT32_MIN - 1));
  // General case. r64, imm32 (sign extended)
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_R8, 0x1EADBEEF), "\x49\xC7\xC0\xEF\xBE\xAD\x1E");
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_R9, -0x1EADBEEF), "\x49\xC7\xC1\x11\x41\x52\xE1");
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_RBX, 0x03), string("\x48\xC7\xC3\x03\x00\x00\x00", 7));
  // General case. r64, imm64
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_R8, 0x1EADBEEFDEADBEEF), "\x49\xB8\xEF\xBE\xAD\xDE\xEF\xBE\xAD\x1E");
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_R9, -0x1EADBEEFDEADBEEF), "\x49\xB9\x11\x41\x52\x21\x10\x41\x52\xe1");
  JIT(gta_mov_reg_imm__x86_64(v, GTA_REG_RBX, 0x1EADBEEFDEADBEEF), "\x48\xBB\xEF\xBE\xAD\xDE\xEF\xBE\xAD\x1E");
}


TEST(x86_64, mov_reg_ind) {
  // General case. r8, m8
  // mov r8, byte ptr [base + index * scale + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_AL, GTA_REG_RCX, GTA_REG_RDX, 4, 42), "\x8A\x44\x91\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_CH, GTA_REG_RDX, GTA_REG_RCX, 1, 42), "\x8A\x6C\x0A\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_DH, GTA_REG_RBX, GTA_REG_RAX, 2, 0x7EADBEEF), "\x8A\xB4\x43\xEF\xBE\xAD\x7E");
  JIT_FAIL(gta_mov_reg_ind__x86_64(v, GTA_REG_BH, GTA_REG_R8, GTA_REG_RDX, 1, 42));
  // mov r8, byte ptr [base + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_BL, GTA_REG_RBX, GTA_REG_NONE, 0, 13), "\x8A\x5B\x0D");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_AH, GTA_REG_RAX, GTA_REG_NONE, 0, 42), "\x8A\x60\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_CL, GTA_REG_R10, GTA_REG_NONE, 0, 0x01234567), "\x41\x8A\x8A\x67\x45\x23\x01");
  JIT_FAIL(gta_mov_reg_ind__x86_64(v, GTA_REG_CH, GTA_REG_R8, GTA_REG_NONE, 0, 42));
  // mov r8, byte ptr [base]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_DL, GTA_REG_RAX, GTA_REG_NONE, 0, 0), "\x8A\x10");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_BH, GTA_REG_RBX, GTA_REG_NONE, 0, 0), "\x8A\x3B");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_CL, GTA_REG_R10, GTA_REG_NONE, 0, 0), "\x41\x8A\x0A");
  JIT_FAIL(gta_mov_reg_ind__x86_64(v, GTA_REG_DH, GTA_REG_R8, GTA_REG_NONE, 0, 0));
  // mov r8, byte ptr [RIP + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_AL, GTA_REG_NONE, GTA_REG_NONE, 0, 0x01234567), "\x8A\x05\x67\x45\x23\x01");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_BH, GTA_REG_NONE, GTA_REG_NONE, 0, 0), string("\x8A\x3D\x00\x00\x00\x00", 6));

  // General case. r16, m16
  // mov r16, word ptr [base + index * scale + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_CX, GTA_REG_RCX, GTA_REG_RDX, 4, 42), "\x66\x8B\x4C\x91\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_DX, GTA_REG_RDX, GTA_REG_RCX, 1, 42), "\x66\x8B\x54\x0A\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_DX, GTA_REG_R9, GTA_REG_RAX, 2, 0x7EADBEEF), "\x66\x41\x8B\x94\x41\xEF\xBE\xAD\x7E");
  // mov r16, word ptr [base + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_BX, GTA_REG_RBX, GTA_REG_NONE, 0, 13), "\x66\x8B\x5B\x0D");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_AX, GTA_REG_RAX, GTA_REG_NONE, 0, 42), "\x66\x8B\x40\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_DX, GTA_REG_R10, GTA_REG_NONE, 0, 0x01234567), "\x66\x41\x8B\x92\x67\x45\x23\x01");
  // mov r16, word ptr [base]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_CX, GTA_REG_RAX, GTA_REG_NONE, 0, 0), "\x66\x8B\x08");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_DX, GTA_REG_RBX, GTA_REG_NONE, 0, 0), "\x66\x8B\x13");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_AX, GTA_REG_R10, GTA_REG_NONE, 0, 0), "\x66\x41\x8B\x02");
  // mov r16, word ptr [RIP + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_CX, GTA_REG_NONE, GTA_REG_NONE, 0, 0x01234567), "\x66\x8B\x0D\x67\x45\x23\x01");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_DX, GTA_REG_NONE, GTA_REG_NONE, 0, 0), string("\x66\x8B\x15\x00\x00\x00\x00", 7));

  // General case. r32, m32
  // mov r32, dword ptr [base + index * scale + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_ESP, GTA_REG_RCX, GTA_REG_RDX, 4, 42), "\x8B\x64\x91\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_EBP, GTA_REG_RDX, GTA_REG_RCX, 1, 42), "\x8B\x6c\x0A\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_ESI, GTA_REG_R9, GTA_REG_RAX, 2, 0x7EADBEEF), "\x41\x8B\xB4\x41\xEF\xBE\xAD\x7E");
  // mov r32, dword ptr [base + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_ESP, GTA_REG_RBX, GTA_REG_NONE, 0, 13), "\x8B\x63\x0D");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_EBP, GTA_REG_RAX, GTA_REG_NONE, 0, 42), "\x8B\x68\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_ESI, GTA_REG_R10, GTA_REG_NONE, 0, 0x01234567), "\x41\x8B\xb2\x67\x45\x23\x01");
  // mov r32, dword ptr [base]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_ESP, GTA_REG_RAX, GTA_REG_NONE, 0, 0), "\x8B\x20");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_EBP, GTA_REG_RBX, GTA_REG_NONE, 0, 0), "\x8B\x2B");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_ESI, GTA_REG_R10, GTA_REG_NONE, 0, 0), "\x41\x8B\x32");
  // mov r32, dword ptr [RIP + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_ESP, GTA_REG_NONE, GTA_REG_NONE, 0, 0x01234567), "\x8B\x25\x67\x45\x23\x01");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_EBP, GTA_REG_NONE, GTA_REG_NONE, 0, 0), string("\x8B\x2D\x00\x00\x00\x00", 6));

  // General case. r64, m64
  // mov r64, qword ptr [base + index * scale + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_RCX, GTA_REG_RDX, 4, 42), "\x48\x8B\x44\x91\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RDX, GTA_REG_RDX, GTA_REG_RCX, 1, 42), "\x48\x8B\x54\x0A\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RDX, GTA_REG_R9, GTA_REG_R10, 2, 0x7EADBEEF), "\x4B\x8B\x94\x51\xEF\xBE\xAD\x7E");
  // mov r64, qword ptr [base + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RBX, GTA_REG_RBX, GTA_REG_NONE, 0, 13), "\x48\x8B\x5B\x0D");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_RAX, GTA_REG_NONE, 0, 42), "\x48\x8B\x40\x2A");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RDX, GTA_REG_R10, GTA_REG_NONE, 0, 0x01234567), "\x49\x8B\x92\x67\x45\x23\x01");
  // mov r64, qword ptr [base]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_RAX, GTA_REG_NONE, 0, 0), string("\x48\x8B\x00", 3));
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RDX, GTA_REG_RBX, GTA_REG_NONE, 0, 0), "\x48\x8B\x13");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_R10, GTA_REG_NONE, 0, 0), "\x49\x8B\x02");
  // mov r64, qword ptr [RIP + offset]
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, GTA_REG_NONE, 0, 0x01234567), "\x48\x8B\x05\x67\x45\x23\x01");
  JIT(gta_mov_reg_ind__x86_64(v, GTA_REG_R10, GTA_REG_NONE, GTA_REG_NONE, 0, 0), string("\x4C\x8B\x15\x00\x00\x00\x00", 7));

  // Failures. SIB registers must be 64-bit.
  JIT_FAIL(gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_BX, GTA_REG_RDX, 4, 42));
  JIT_FAIL(gta_mov_reg_ind__x86_64(v, GTA_REG_EAX, GTA_REG_RBX, GTA_REG_ECX, 1, 42));
}


TEST(x86_64, mov_reg_reg) {
  // General case. r8, r8
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_AL, GTA_REG_BL), "\x88\xD8");
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_BH, GTA_REG_CL), "\x88\xCF");
  JIT_FAIL(gta_mov_reg_reg__x86_64(v, GTA_REG_AL, GTA_REG_AX));
  JIT_FAIL(gta_mov_reg_reg__x86_64(v, GTA_REG_BH, GTA_REG_RBX));
  // General case. r16, r16
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_CX, GTA_REG_DX), "\x66\x89\xD1");
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_DX, GTA_REG_SI), "\x66\x89\xF2");
  JIT_FAIL(gta_mov_reg_reg__x86_64(v, GTA_REG_CX, GTA_REG_EAX));
  JIT_FAIL(gta_mov_reg_reg__x86_64(v, GTA_REG_DX, GTA_REG_AH));
  // General case. r32, r32
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_ESP, GTA_REG_EBP), "\x89\xEC");
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_EBP, GTA_REG_ESI), "\x89\xF5");
  JIT_FAIL(gta_mov_reg_reg__x86_64(v, GTA_REG_ESP, GTA_REG_R8));
  JIT_FAIL(gta_mov_reg_reg__x86_64(v, GTA_REG_EBP, GTA_REG_DX));
  // General case. r64, r64
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_R8, GTA_REG_R9), "\x4D\x89\xC8");
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_R9, GTA_REG_R10), "\x4D\x89\xD1");
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_RAX, GTA_REG_RBX), "\x48\x89\xD8");
  JIT(gta_mov_reg_reg__x86_64(v, GTA_REG_RBX, GTA_REG_R12), "\x4C\x89\xE3");
  JIT_FAIL(gta_mov_reg_reg__x86_64(v, GTA_REG_R8, GTA_REG_EAX));
}


TEST(x86_64, movq_reg_reg) {
  // General case. xmm, r64
  JIT(gta_movq_reg_reg__x86_64(v, GTA_REG_XMM0, GTA_REG_RAX), "\x66\x48\x0F\x6E\xC0");
  JIT(gta_movq_reg_reg__x86_64(v, GTA_REG_XMM1, GTA_REG_R9), "\x66\x49\x0F\x6E\xC9");
  JIT(gta_movq_reg_reg__x86_64(v, GTA_REG_XMM2, GTA_REG_R10), "\x66\x49\x0F\x6E\xD2");
  // General case. r64, xmm
  JIT(gta_movq_reg_reg__x86_64(v, GTA_REG_RBX, GTA_REG_XMM3), "\x66\x48\x0F\x7E\xDB");
  JIT(gta_movq_reg_reg__x86_64(v, GTA_REG_R12, GTA_REG_XMM4), "\x66\x49\x0F\x7E\xE4");
  JIT(gta_movq_reg_reg__x86_64(v, GTA_REG_R13, GTA_REG_XMM5), "\x66\x49\x0F\x7E\xED");
}


TEST(x86_64, or_reg_reg) {
  // General case. r8, r8
  JIT(gta_or_reg_reg__x86_64(v, GTA_REG_AL, GTA_REG_BL), "\x08\xD8");
  JIT(gta_or_reg_reg__x86_64(v, GTA_REG_BH, GTA_REG_CL), "\x08\xCF");
  JIT_FAIL(gta_or_reg_reg__x86_64(v, GTA_REG_AL, GTA_REG_AX));
  JIT_FAIL(gta_or_reg_reg__x86_64(v, GTA_REG_BH, GTA_REG_RBX));
  // General case. r16, r16
  JIT(gta_or_reg_reg__x86_64(v, GTA_REG_CX, GTA_REG_DX), "\x66\x09\xD1");
  JIT(gta_or_reg_reg__x86_64(v, GTA_REG_DX, GTA_REG_SI), "\x66\x09\xF2");
  JIT_FAIL(gta_or_reg_reg__x86_64(v, GTA_REG_CX, GTA_REG_EAX));
  JIT_FAIL(gta_or_reg_reg__x86_64(v, GTA_REG_DX, GTA_REG_AH));
  // General case. r32, r32
  JIT(gta_or_reg_reg__x86_64(v, GTA_REG_ESP, GTA_REG_EBP), "\x09\xEC");
  JIT(gta_or_reg_reg__x86_64(v, GTA_REG_EBP, GTA_REG_ESI), "\x09\xF5");
  JIT_FAIL(gta_or_reg_reg__x86_64(v, GTA_REG_ESP, GTA_REG_R8));
  JIT_FAIL(gta_or_reg_reg__x86_64(v, GTA_REG_EBP, GTA_REG_DX));
  // General case. r64, r64
  JIT(gta_or_reg_reg__x86_64(v, GTA_REG_R8, GTA_REG_R9), "\x4D\x09\xC8");
  JIT(gta_or_reg_reg__x86_64(v, GTA_REG_RBX, GTA_REG_R12), "\x4C\x09\xE3");
  JIT_FAIL(gta_or_reg_reg__x86_64(v, GTA_REG_R8, GTA_REG_EAX));
}


TEST(x86_64, pop_reg) {
  // General case. r64
  JIT(gta_pop_reg__x86_64(v, GTA_REG_RAX), "\x58");
  JIT(gta_pop_reg__x86_64(v, GTA_REG_RBX), "\x5B");
  JIT(gta_pop_reg__x86_64(v, GTA_REG_R12), "\x41\x5C");
  JIT(gta_pop_reg__x86_64(v, GTA_REG_R15), "\x41\x5F");
  JIT_FAIL(gta_pop_reg__x86_64(v, GTA_REG_AX));
}


TEST(x86_64, push_reg) {
  // General case. r64
  JIT(gta_push_reg__x86_64(v, GTA_REG_RAX), "\x50");
  JIT(gta_push_reg__x86_64(v, GTA_REG_RBX), "\x53");
  JIT(gta_push_reg__x86_64(v, GTA_REG_R12), "\x41\x54");
  JIT(gta_push_reg__x86_64(v, GTA_REG_R15), "\x41\x57");
  JIT_FAIL(gta_push_reg__x86_64(v, GTA_REG_AX));
}


TEST(x86_64, ret) {
  // General case.
  JIT(gta_ret__x86_64(v), "\xC3");
}


TEST(x86_64, xor_reg_reg) {
  // General case. r8, r8
  JIT(gta_xor_reg_reg__x86_64(v, GTA_REG_AL, GTA_REG_BL), "\x30\xD8");
  JIT(gta_xor_reg_reg__x86_64(v, GTA_REG_BH, GTA_REG_CL), "\x30\xCF");
  JIT_FAIL(gta_xor_reg_reg__x86_64(v, GTA_REG_AL, GTA_REG_AX));
  JIT_FAIL(gta_xor_reg_reg__x86_64(v, GTA_REG_BH, GTA_REG_RBX));
  // General case. r16, r16
  JIT(gta_xor_reg_reg__x86_64(v, GTA_REG_CX, GTA_REG_DX), "\x66\x31\xD1");
  JIT(gta_xor_reg_reg__x86_64(v, GTA_REG_DX, GTA_REG_SI), "\x66\x31\xF2");
  JIT_FAIL(gta_xor_reg_reg__x86_64(v, GTA_REG_CX, GTA_REG_EAX));
  JIT_FAIL(gta_xor_reg_reg__x86_64(v, GTA_REG_DX, GTA_REG_AH));
  // General case. r32, r32
  JIT(gta_xor_reg_reg__x86_64(v, GTA_REG_ESP, GTA_REG_EBP), "\x31\xEC");
  JIT(gta_xor_reg_reg__x86_64(v, GTA_REG_EBP, GTA_REG_ESI), "\x31\xF5");
  JIT_FAIL(gta_xor_reg_reg__x86_64(v, GTA_REG_ESP, GTA_REG_R8));
  JIT_FAIL(gta_xor_reg_reg__x86_64(v, GTA_REG_EBP, GTA_REG_DX));
  // General case. r64, r64
  JIT(gta_xor_reg_reg__x86_64(v, GTA_REG_R8, GTA_REG_R9), "\x4D\x31\xC8");
  JIT(gta_xor_reg_reg__x86_64(v, GTA_REG_RBX, GTA_REG_R12), "\x4C\x31\xE3");
  JIT_FAIL(gta_xor_reg_reg__x86_64(v, GTA_REG_R8, GTA_REG_EAX));
}


int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


