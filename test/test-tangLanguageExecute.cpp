#include <gtest/gtest.h>
#include <iostream>

#include "tang/tang.h"
#include "tang/macros.h"
#include "tang/program.h"
#include "tang/context.h"
#include "tang/bytecode.h"

using namespace std;

TEST(Tang, Program) {
  GTA_Program * program = gta_program_create("");
  cout << "Program: " << (void *)program->ast << endl;
  GTA_Context context;
  gta_program_execute(program, &context);
  gta_program_destroy(program);
  gta_context_destroy_in_place(&context);
}


int main(int argc, char **argv) {
  cout <<
    (GTA_BIG_ENDIAN ? "Big Endian" : "Little Endian") << endl;
  cout <<
    (GTA_32_BIT ? "32-bit" : "64-bit") << endl;
  cout <<
    (GTA_X86 ? "x86" : "Not x86") << endl;
  cout <<
    (GTA_X86_64 ? "x86_64" : "Not x86_64") << endl;
  cout <<
    (GTA_ARM ? "ARM" : "Not ARM") << endl;
  cout <<
    (GTA_ARM64 ? "ARM64" : "Not ARM64") << endl;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
