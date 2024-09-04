/**
 * @file
 *
 * This file is the command line interpreter for Tang.
 */

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <cutil/memory.h>
#include <tang/tang.h>

void print_help_text() {
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
    FILE * file = fopen(file_name, "r");
    if (!file) {
      // Error: failed to open the file.
      fprintf(stderr, "Error, failed to open the file %s\n", file_name);
      // Print the current directory.
      system("pwd");
      return -3;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = gcu_malloc(length + 1);
    if (!buffer) {
      // Error: failed to allocate memory.
      fprintf(stderr, "Error, failed to allocate memory\n");
      return -4;
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    eval = buffer;
  }
  // If no code was provided, read from stdin.
  else if (!eval) {
    size_t size = 0;
    size_t capacity = 1024;
    buffer = gcu_malloc(capacity);
    if (!buffer) {
      // Error: failed to allocate memory.
      fprintf(stderr, "Error, failed to allocate memory\n");
      return -5;
    }
    while (true) {
      if (size + 1 >= capacity) {
        capacity *= 2;
        buffer = gcu_realloc(buffer, capacity);
        if (!buffer) {
          // Error: failed to allocate memory.
          fprintf(stderr, "Error, failed to allocate memory\n");
          return -6;
        }
      }
      char c = fgetc(stdin);
      if (c == EOF) {
        break;
      }
      buffer[size++] = c;
    }
    buffer[size] = '\0';
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
  return error;
}
