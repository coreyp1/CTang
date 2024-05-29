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


TEST(x86_64, lea_reg_mem) {
  // TODO: Implement after full SIB support is added.
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


int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


