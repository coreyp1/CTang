/*
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * Copyright (C) 2024-2026 Corey Pennycuff
 *
 * This file is part of Ghoti.io Tang.
 *
 * Ghoti.io Tang is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Ghoti.io Tang is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * This file is the command line interpreter for Tang.
 */

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ghoti.io/cutil/array.h>
#include <ghoti.io/cutil/file.h>
#include <ghoti.io/cutil/memory.h>
#include <ghoti.io/tang/allocator.h>
#include <ghoti.io/tang/macros.h>
#include <ghoti.io/tang/tang.h>

void print_help_text(void) {
  int width = 15;
  int indent = 20;
  printf(
    "%-*s%s\n"
    "%-*s%s\n"
    "%-*s%s\n"
    "%-*s%s\n",
    width, "--evaluate ARGUMENT, -e ARGUMENT", "Evaluate supplied ARGUMENT instead of stdin",
    indent, "--script, -s", "Evaluate the code as a script rather than a template",
    indent, "--cleanup, -c", "Cleanup memory before shutdown (useful for valgrind debugging)",
    width, "--help, -h", "Display help message"
  );
}

int main(int argc, const char * argv[]) {
  // The name of the file to read from (if any).
  const char * file_name = NULL;
  // The code to evaluate.
  const char * eval = NULL;
  // A buffer to read the file into (if necessary).
  char * buffer = NULL;
  // Whether the code is a script or a template.
  bool is_script = false;
  // Whether to cleanup memory before shutdown.
  bool cleanup = false;
  // The error code to return.
  int error = 0;
  
  // Parse Command line arguments.
  for (int i = 1; i < argc; ++i) {
    // Evaluate the provided argument instead of reading from stdin.
    if (!strcmp(argv[i], "--evaluate") || !strcmp(argv[i], "-e")) {
      if (i + 1 >= argc) {
        // Error: not enough arguments.
        fprintf(stderr, "Error, not enough arguments");
        return 1;
      }
      eval = argv[i + 1];
      ++i;
    }
    // Print the help text.
    else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
      print_help_text();
      return 0;
    }
    // Evaluate the code as a script rather than a template.
    else if (!strcmp(argv[i], "--script") || !strcmp(argv[i], "-s")) {
      is_script = true;
    }
    // Cleanup memory before shutdown.
    else if (!strcmp(argv[i], "--cleanup") || !strcmp(argv[i], "-c")) {
      cleanup = true;
    }
    else if (!file_name) {
      file_name = argv[i];
    }
    else {
      // Error: too many arguments.
      fprintf(stderr, "Error, too many arguments");
      return -1;
    }
  }
  // Make sure that no conflicting arguments were provided.
  if (eval && file_name) {
    // Error: conflicting arguments.
    fprintf(stderr, "Error, conflicting arguments.  Please provide either a file name or an argument to evaluate, but not both.");
    return -2;
  }

  // If a file name was provided, read the file into the buffer.
  if (file_name) {
    // cutil reads the whole file, in chunks rather than by seeking to the
    // end first, so this works on a pipe or a character device as well as on
    // an ordinary file - and it puts a NUL one past the end, which is what
    // lets the result be handed straight to the compiler as a C string.
    //
    // What it replaces sized the file with ftell(), never checked what
    // fread() actually returned, and leaked the FILE * if the allocation
    // failed. Opened in text mode, as it was, ftell() over-reports on
    // Windows by the number of line endings, so the tail of the buffer was
    // whatever the allocator last left there.
    void * contents = NULL;
    size_t length = 0;
    GCU_File_Result result = gcu_file_read(file_name, GCU_FILE_UNLIMITED,
      gta_allocator(), &contents, &length);
    if (result != GCU_FILE_OK) {
      fprintf(stderr, "Error, failed to read the file %s: %s\n", file_name,
        gcu_file_result_string(result));
      return -3;
    }
    buffer = contents;
    eval = buffer;
  }
  // If no code was provided, read from stdin.
  else if (!eval) {
    // The array owns the length and the growth. The loop this replaces read
    // one character at a time into a char, and `c == EOF` is false for every
    // char value on a platform where char is unsigned, so it never
    // terminated there; where char is signed, a 0xFF byte ended the input
    // early. It also doubled its capacity without checking for overflow, and
    // dropped the old pointer on a failed realloc.
    GCU_Array * input = gcu_array_create(1, 1024, gta_allocator());
    if (!input) {
      fprintf(stderr, "Error, failed to allocate memory\n");
      return -5;
    }
    char chunk[4096];
    size_t read_count;
    while ((read_count = fread(chunk, 1, sizeof(chunk), stdin)) > 0) {
      if (!gcu_array_append_n(input, chunk, read_count)) {
        gcu_array_destroy(input);
        fprintf(stderr, "Error, failed to allocate memory\n");
        return -6;
      }
    }
    if (ferror(stdin)) {
      gcu_array_destroy(input);
      fprintf(stderr, "Error, failed to read from stdin\n");
      return -7;
    }
    // The terminator is in the buffer but not in the code, which is what
    // lets an empty stdin compile as an empty program rather than as
    // whatever follows it in memory.
    if (!gcu_array_append(input, "")) {
      gcu_array_destroy(input);
      fprintf(stderr, "Error, failed to allocate memory\n");
      return -6;
    }
    buffer = gcu_array_steal(input, NULL);
    gcu_array_destroy(input);
    eval = buffer;
  }

  // Initialize the language.
  GTA_Language * language = gta_language_create();
  if (!language) {
    // Error: failed to create the language.
    fprintf(stderr, "Error, failed to create the language\n");
    error = 1;
    goto LANGUAGE_CREATE_FAILED;
  }
  
  // Compile the code into a Program.
  GTA_Program * program = gta_program_create_with_flags(language, eval, GTA_PROGRAM_FLAG_PRINT_TO_STDOUT | (is_script ? 0 : GTA_PROGRAM_FLAG_IS_TEMPLATE));
  if (!program) {
    // Error: failed to compile the program.
    fprintf(stderr, "Error, failed to compile the program\n");
    error = 1;
    goto COMPILE_FAILED;
  }

  GTA_Execution_Context * context = gta_execution_context_create(program);
  if (!context) {
    // Error: failed to create the execution context.
    fprintf(stderr, "Error, failed to create the execution context\n");
    error = 2;
    goto CONTEXT_CREATE_FAILED;
  }

  if (!gta_program_execute(context)) {
    // Error: failed to execute the program.
    fprintf(stderr, "Error, failed to execute the program\n");
    error = 3;
  }

  // Intentional fall-through.
  if (cleanup) {
    gta_execution_context_destroy(context);
  }
CONTEXT_CREATE_FAILED:
  if (cleanup) {
    gta_program_destroy(program);
  }
COMPILE_FAILED:
  if (cleanup) {
    gta_language_destroy(language);
  }
LANGUAGE_CREATE_FAILED:
  if (cleanup) {
    // Only under --cleanup, as with everything else here: the point of the
    // flag is a run that valgrind can read, and the buffer holding the code
    // is as much a part of that as the library objects are.
    gcu_free(buffer);
  }
  return error;
}
