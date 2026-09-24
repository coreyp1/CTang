/**
 * @file
 *
 * libFuzzer harness for the bytecode/x86-64 differential.
 *
 * Section 1 of the language reference says the two backends are required to
 * agree.  That is a falsifiable claim about every program, and it needs no
 * reference implementation to check, which makes it the strongest oracle this
 * library has: "did not crash" can only find the bugs that crash, and the
 * defects cleared in section 13 were mostly wrong answers rather than crashes.
 * Two of them - the ranged `for` that ended with its own sentinel, and the
 * x86-64 engine binding every parameter to the wrong argument - were found by
 * running the engines against each other by hand, over a shell loop of 81
 * cases.  This replaces that loop.
 *
 * Each input becomes a program (see tangGenerator.h), which is then compiled
 * twice - once with the binary disabled and once with the bytecode disabled,
 * because a program holds one or the other and never both - and run once under
 * each.  Three things must match:
 *
 *   - whether it compiled at all,
 *   - the program's result, and
 *   - the bytes it printed.
 *
 * A mismatch aborts, which is how libFuzzer is told to keep the input.
 *
 * ## What "the same result" means here
 *
 * Nothing is normalised.  There was one exception for a while - a function
 * value printed its entry point, which is a bytecode offset under one engine
 * and a machine address under the other - and the right answer turned out to
 * be that the printed form should not contain it (13.37), not that the
 * comparison should look away.  An error is compared by its full text, because
 * which error is exactly what the two engines have disagreed about before.
 *
 * A harness with no exceptions is worth keeping.  Every exception is a shape
 * the oracle stops covering, and the reason for it ages out of view.
 *
 * ## Reading the corpus
 *
 * An input here is generator bytes, not Tang source, so a corpus file and an
 * artifact are both unreadable on their own.  Print what one becomes with
 *
 *     TANG_DIFF_SHOW=1 build/linux/release-fuzz/apps/fuzz_differential <file>
 *
 * The seeds in test/fuzz/seeds/differential are the inputs that found the
 * defects listed against this harness in section 13 of the language
 * reference, kept under the name of what each one found.  They are seeds
 * rather than tests because what they reproduce is now covered by tests; what
 * they are still good for is starting a campaign somewhere interesting.
 *
 * Build with: make fuzz-differential     Run: make fuzz-run-differential
 *
 * SPDX-License-Identifier: LGPL-3.0-only
 * Copyright (C) 2026 Corey Pennycuff
 */

// First, before any system header: it sets a feature test macro.
#include "lastInput.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ghoti.io/cutil/memory.h>
#include <ghoti.io/tang/tang.h>
#include <ghoti.io/tang/macros.h>
#include <ghoti.io/tang/computedValue/computedValueAll.h>
#include <ghoti.io/tang/program/executionContext.h>
#include <ghoti.io/tang/program/program.h>
#include <ghoti.io/tang/unicodeString.h>

#include "tangGenerator.h"

/// What one engine made of one program.
typedef struct {
  /// Whether the program compiled for this engine at all.
  bool compiled;
  /// Whether the run reported success.
  bool executed;
  /// The result, rendered.  NULL when there was none.
  char * result;
  /// The output buffer, rendered as a host would see it.
  char * output;
  size_t output_length;
} Engine_Run;

static GTA_Language * language;
/// TANG_DIFF_SHOW in the environment prints each generated program.  An input
/// here is generator bytes rather than Tang source, so without this there is
/// no way to read an artifact back:
///
///     TANG_DIFF_SHOW=1 build/linux/release-fuzz/apps/fuzz_differential <file>
static bool show_programs;
/// Set when this machine has no JIT, in which case there is nothing to
/// compare and the harness says so once instead of reporting every program.
static bool differential_possible;

static unsigned long long counted_total;
static unsigned long long counted_truncated;
static unsigned long long counted_uncompilable;
static unsigned long long counted_compared;

static void report_counts(void) {
  fprintf(stderr,
    "\n### differential: %llu inputs, %llu compared, "
    "%llu did not compile, %llu truncated ###\n",
    counted_total, counted_compared, counted_uncompilable, counted_truncated);
}

static void engine_run_free(Engine_Run * run) {
  gcu_free(run->result);
  gcu_free(run->output);
  run->result = NULL;
  run->output = NULL;
}

/**
 * Compile for one engine and run it.
 *
 * `flags` carries the flag that turns the *other* engine off, so that the
 * program this builds can only be executed the one way.
 */
static void engine_run(Engine_Run * run, const char * source,
    GTA_Program_Flags flags, bool use_binary) {
  memset(run, 0, sizeof(*run));

  GTA_Program * program = gta_program_create_with_flags(language, source, flags);
  if (!program) {
    return;
  }
  run->compiled = true;

  GTA_Execution_Context * context = gta_execution_context_create(program);
  if (!context) {
    gta_program_destroy(program);
    return;
  }

  run->executed = use_binary
    ? gta_program_execute_binary(context)
    : gta_program_execute_bytecode(context);

  if (context->result) {
    run->result = gta_computed_value_to_string(context->result);
  }
  if (context->output) {
    GTA_Unicode_Rendered_String rendered = gta_unicode_string_render(context->output);
    run->output = rendered.buffer;
    run->output_length = rendered.buffer ? rendered.length : 0;
  }

  gta_execution_context_destroy(context);
  gta_program_destroy(program);
}

static bool strings_match(const char * a, const char * b) {
  return (!a && !b) || (a && b && strcmp(a, b) == 0);
}

static void report_divergence(const char * what, const char * source,
    const Engine_Run * vm, const Engine_Run * jit) {
  fprintf(stderr, "\n### the two engines disagree: %s ###\n", what);
  fprintf(stderr, "source:   %s\n", source);
  fprintf(stderr, "bytecode: compiled=%d executed=%d result=[%s] output=[%.*s]\n",
    vm->compiled, vm->executed, vm->result ? vm->result : "(none)",
    (int)vm->output_length, vm->output ? vm->output : "");
  fprintf(stderr, "x86-64:   compiled=%d executed=%d result=[%s] output=[%.*s]\n",
    jit->compiled, jit->executed, jit->result ? jit->result : "(none)",
    (int)jit->output_length, jit->output ? jit->output : "");
  fflush(stderr);
}

int LLVMFuzzerTestOneInput(const uint8_t * data, size_t size) {
  // Record the input before touching it. A crash the sanitizer cannot
  // report takes libFuzzer's artifact with it; see lastInput.h.
  gta_fuzz_record_last_input("build/last-input-differential.bin", data, size);

  if (!language) {
    language = gta_language_create();
    if (!language) {
      return 0;
    }
    atexit(report_counts);
    show_programs = getenv("TANG_DIFF_SHOW") != NULL;

    // Is there a second engine on this machine at all?  On anything but
    // x86-64 the JIT produces nothing, `gta_program_create` then has neither
    // form and fails, and every program would be reported as a compile
    // disagreement.  That is the port being absent, not a defect.
    GTA_Program * probe = gta_program_create_with_flags(language, "1;",
      GTA_PROGRAM_FLAG_DISABLE_BYTECODE | GTA_PROGRAM_FLAG_IGNORE_ENVIRONMENT);
    differential_possible = probe != NULL;
    if (probe) {
      gta_program_destroy(probe);
    }
    else {
      fprintf(stderr, "### note: no binary engine on this machine; "
        "there is nothing to run a differential against ###\n");
    }
  }
  if (!differential_possible) {
    return 0;
  }

  static GTA_Gen generator;
  bool is_template = false;
  const char * source = gta_gen_program(&generator, data, size, &is_template);

  if (show_programs) {
    fprintf(stderr, "%s %s\n", is_template ? "template:" : "script:  ", source);
  }

  ++counted_total;
  if (generator.full) {
    // Cut off mid-construct, so it is a syntax error about the generator
    // rather than about the language.
    ++counted_truncated;
    return 0;
  }

  // IGNORE_ENVIRONMENT because TANG_DISABLE_BYTECODE and TANG_DISABLE_BINARY
  // are read from the environment otherwise, and either one set would quietly
  // turn this into a comparison of one engine with itself.
  GTA_Program_Flags common = GTA_PROGRAM_FLAG_IGNORE_ENVIRONMENT
    | (is_template ? GTA_PROGRAM_FLAG_IS_TEMPLATE : 0);

  Engine_Run vm;
  Engine_Run jit;
  engine_run(&vm, source, common | GTA_PROGRAM_FLAG_DISABLE_BINARY, false);
  engine_run(&jit, source, common | GTA_PROGRAM_FLAG_DISABLE_BYTECODE, true);

  bool diverged = false;
  if (vm.compiled != jit.compiled) {
    report_divergence("one compiled it and the other did not", source, &vm, &jit);
    diverged = true;
  }
  else if (!vm.compiled) {
    ++counted_uncompilable;
  }
  else {
    ++counted_compared;
    if (vm.executed != jit.executed) {
      report_divergence("one ran it and the other did not", source, &vm, &jit);
      diverged = true;
    }
    else if (!strings_match(vm.result, jit.result)) {
      report_divergence("different results", source, &vm, &jit);
      diverged = true;
    }
    else if ((vm.output_length != jit.output_length)
      || (vm.output_length && memcmp(vm.output, jit.output, vm.output_length))) {
      report_divergence("different output", source, &vm, &jit);
      diverged = true;
    }
  }

  engine_run_free(&vm);
  engine_run_free(&jit);

  if (diverged) {
    // libFuzzer writes the artifact from a crash handler, so the way to keep
    // an input is to die on it.
    abort();
  }
  return 0;
}
