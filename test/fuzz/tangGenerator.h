/**
 * @file
 *
 * Turn a byte string into a Tang program that is guaranteed to terminate.
 *
 * The two engines are required to agree (language reference, section 1), so a
 * program run under both is a falsifiable claim with no reference
 * implementation needed.  What has been missing is a supply of programs.  The
 * parse harnesses feed raw bytes to the parser, which is the right shape for
 * testing a parser and the wrong one for testing execution: a random byte
 * string is a syntax error essentially always, and the handful that are not
 * are `;` and `0;`.  This builds a program out of the bytes instead, so that
 * every input is a program and libFuzzer's coverage feedback is steering the
 * *shape* of the program rather than hunting for balanced parentheses.
 *
 * ## Why it terminates
 *
 * The harness runs inside the fuzzer's process, so a program that does not
 * come back ends the campaign.  Tang is Turing-complete and has no execution
 * limit, so termination has to come from the generator.  It comes from three
 * bounds, each maintained as the program is written:
 *
 * - **Work.**  `work` is the number of statement executions the rest of the
 *   program may still spend.  Every statement charges `multiplier`, the number
 *   of times the point being generated will run; a loop multiplies it by the
 *   loop's iteration count for the body, and a call charges the callee's whole
 *   cost again.  So the total is bounded by the initial budget, by
 *   construction rather than by hope.
 *
 * - **Loops.**  Every generated loop counts a variable down to zero, that
 *   variable is written nowhere else (the `q` names are not in the pool the
 *   expression generator draws from), and the decrement is the first statement
 *   of the body so that a `continue` cannot skip it.  A ranged `for` walks an
 *   array literal built on the spot, which nothing in the body can reach - the
 *   language permits mutating the array being iterated, and that is a way to
 *   make one run forever.
 *
 * - **Recursion.**  A function may call only functions declared before it, so
 *   the call graph is acyclic.  Recursion is bounded by the interpreter now
 *   (13.14), but 512 frames of a function containing loops is slow rather than
 *   safe, and the depth limit already has tests of its own.
 *
 * ## Why values stay small
 *
 * Termination is not enough: `s = s + s` forty times is finite and still
 * exhausts memory, and an out-of-memory kill takes the campaign down just as a
 * hang would.  So the generator also carries a static upper bound on the
 * printed size of every variable, and refuses any expression whose bound would
 * exceed `GTA_GEN_SIZE_CAP`.  Bounds compose the way printing does: `+` adds,
 * an array literal sums its elements, a cast to string keeps the operand's.
 *
 * Two operators grow a value, not one.  `*` repeats an array or a string
 * (4.2), and it multiplies rather than adds, which no additive bound
 * survives: this argument was first written as though `+` were the only one,
 * and a campaign duly wrote `[10, 10] * (10 * 16) * ((10 * 10) * (10 * 10))`
 * and asked for 10^14 elements.  So the right operand of `*` is a literal no
 * larger than `GTA_GEN_REPEAT_MAX`, and the left is built inside a budget
 * divided by it.  The cost is that `expression * expression` is never
 * written; the value on the right is what decides the size, so there is no
 * bound to be had while it is anything the generator does not choose.
 *
 * Inside a loop the arithmetic has to account for repetition, and the shape
 * that defeats an additive bound is a variable that reads itself: `v = v + v`
 * doubles, so after `m` iterations it is `2^m`, not `m` times anything.  The
 * rule that makes the additive form sound is that within a loop body, the
 * right-hand side of an assignment may not read any variable that the body
 * assigns - except as the leading operand of the assignment to that same
 * variable, which is the accumulator shape `v += e`.  Growth per iteration is
 * then at most the bound of `e`, and `multiplier * bound(e)` is a true bound
 * on the whole loop.  Statement-level reads are unrestricted, because the
 * recurrence only runs through assignments.
 *
 * ## What it does not generate
 *
 * - `use`, and so the `random` and `math` libraries.  An unseeded generator
 *   would differ between the two runs of the same program and every difference
 *   would be reported as a divergence.
 * - `global`, whose whole point is to let a function write a variable the
 *   generator is tracking a bound for in another scope.
 * - Date literals, which the parser does not accept (13.13).
 * - Functions declared inside functions.
 *
 * Each of those is a gap in what the differential covers, not a statement that
 * the construct works.
 *
 * SPDX-License-Identifier: LGPL-3.0-only
 * Copyright (C) 2026 Corey Pennycuff
 */

#ifndef GHOTIIO_TANG_FUZZ_GENERATOR_H
#define GHOTIIO_TANG_FUZZ_GENERATOR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// Characters of source the generator may write.
#define GTA_GEN_MAX_SOURCE 4096
/// Where the generator stops starting new things.  Hitting GTA_GEN_MAX_SOURCE
/// itself truncates mid-expression, and a truncated program is a syntax error
/// rather than a shorter program - so growth is stopped with room left for
/// every construct already open to be closed.
#define GTA_GEN_SOFT_SOURCE 3000
/// Program-scope variables, and function-local variables, by these names.
#define GTA_GEN_VARS 6
/// Functions a program may declare.
#define GTA_GEN_FUNCS 3
/// Parameters a generated function may take.
#define GTA_GEN_MAX_PARAMS 3
/// How deeply an expression may nest.
#define GTA_GEN_EXPR_DEPTH 4
/// How deeply a loop may nest.
#define GTA_GEN_LOOP_DEPTH 2
/// Statement executions the whole program may spend.
#define GTA_GEN_WORK 3000
/// Printed size, in characters, that no value may exceed.
#define GTA_GEN_SIZE_CAP 4096
/// Printed size of a call argument, a parameter, and a return value.
#define GTA_GEN_ARG_CAP 256
/// Printed size of any scalar: 20 digits, a sign, and room to spare.
#define GTA_GEN_SCALAR 24
/// Bytes the program may print.
#define GTA_GEN_OUTPUT 65536
/// Iterations any one generated loop may run.
#define GTA_GEN_LOOP_MAX 4
/// Largest right operand of `*`.  `*` repeats an array or a string (4.2), so
/// this is a growth factor, not just a number.
#define GTA_GEN_REPEAT_MAX 4

typedef struct {
  // The byte source.
  const uint8_t * data;
  size_t size;
  size_t pos;

  // The source being written.
  char source[GTA_GEN_MAX_SOURCE];
  size_t length;
  /// Set when the source filled up.  Every generator checks it and collapses
  /// to its simplest production, so generation always finishes.
  bool full;

  // Budgets.  See the file comment.
  long work;
  long multiplier;
  long output;

  int loop_depth;
  int expr_depth;

  /// Printed-size bound of each variable's value.  The slots past
  /// GTA_GEN_VARS are the current function's parameters, visible only while
  /// `params` says how many there are.
  int bound[GTA_GEN_VARS + GTA_GEN_MAX_PARAMS];
  /// Variables assigned somewhere in the loop nest being generated.  Reading
  /// one of these from an assignment's right-hand side is what the additive
  /// growth bound cannot survive.
  unsigned loop_assigned;
  /// True while generating the right-hand side of an assignment.
  bool in_assign_rhs;

  struct {
    int arity;
    long work;
  } fn[GTA_GEN_FUNCS];
  int fn_count;

  /// Zero at program scope; the parameter count inside a function body.
  bool in_function;
  int params;

  /// Serial for the loop counter variables, so nested loops get their own.
  int loop_serial;
} GTA_Gen;

/// Variable names.  `f` is reserved for function names and none of these is a
/// keyword.
static const char * const gta_gen_var_names[GTA_GEN_VARS] = {
  "a", "b", "c", "d", "e", "g",
};

/// Map keys, which are bare identifiers in a literal and strings at run time.
static const char * const gta_gen_key_names[4] = { "k0", "k1", "k2", "k3" };

/**
 * One byte of the input, or zero once it runs out.
 *
 * Exhaustion returning zero is load-bearing: choice 0 of every switch below is
 * a production that does not recurse, so an input that runs out mid-expression
 * finishes as a literal rather than as something unbalanced.
 */
static uint8_t gta_gen_byte(GTA_Gen * g) {
  return g->pos < g->size ? g->data[g->pos++] : 0;
}

/// A choice in [0, n).
static uint32_t gta_gen_pick(GTA_Gen * g, uint32_t n) {
  return n ? (uint32_t)gta_gen_byte(g) % n : 0;
}

/// True with probability 1 in n.
static bool gta_gen_rare(GTA_Gen * g, uint32_t n) {
  return gta_gen_pick(g, n) == 0;
}

/// True once the generator should stop starting anything new.
static bool gta_gen_tight(GTA_Gen * g) {
  return g->full || (g->length > GTA_GEN_SOFT_SOURCE);
}

static void gta_gen_emit(GTA_Gen * g, const char * text) {
  size_t n = strlen(text);
  if (g->full || (g->length + n + 1 > GTA_GEN_MAX_SOURCE)) {
    g->full = true;
    return;
  }
  memcpy(g->source + g->length, text, n);
  g->length += n;
  g->source[g->length] = '\0';
}

static void gta_gen_emitf(GTA_Gen * g, const char * format, ...) {
  char buffer[128];
  va_list args;
  va_start(args, format);
  int n = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  if (n < 0 || (size_t)n >= sizeof(buffer)) {
    g->full = true;
    return;
  }
  gta_gen_emit(g, buffer);
}

/**
 * Integer literals.
 *
 * Weighted toward small values, because those are what index, slice and loop
 * expressions need to reach anything, with the edges of the 64-bit range
 * mixed in because overflow is reported rather than wrapped (4.2) and the
 * report is a value the two engines have to agree on.
 *
 * 9223372036854775808 is deliberately absent: it is one past the maximum and
 * a syntax error everywhere except directly after unary minus (2.6), and a
 * program that does not compile tests nothing.  The unary generator spells the
 * most negative integer itself.
 */
static const char * const gta_gen_integers[] = {
  "0", "1", "2", "3", "7", "8", "10", "16", "63", "64", "255", "256",
  "1000", "65535", "65536", "2147483647", "2147483648", "4294967295",
  "9223372036854775806", "9223372036854775807",
};

static const char * const gta_gen_floats[] = {
  "0.", ".5", "1.", "1.5", "2.", "3.14", "0.1", "100.", "0.0",
  // Past the integer range, so that `as int` has something to refuse.
  "100000000000000000000000000000.",
};

/**
 * String literal bodies, and the three prefixes that set a literal's encoding.
 *
 * The escapes are here because they are decided in the scanner, which is
 * shared, but what the decoded bytes then do is not: an embedded NUL is what
 * makes string comparison a memcmp over the byte length rather than a strcmp
 * (13.3), and a multi-byte grapheme is what makes indexing a string different
 * from indexing its bytes.
 */
static const char * const gta_gen_strings[] = {
  "", "a", "ab", "abc", "xyz", " ", "0", "-1",
  "<b>", "&amp;", "\\\"", "\\\\", "a\\nb", "\\x41", "\\101", "a\\0b",
  "\\xc3\\xa9", "caf\\xc3\\xa9", "  ", "]",
};

static const char * const gta_gen_string_prefixes[] = { "", "!", "%" };

/**
 * Attribute names.
 *
 * The string attributes, the array's `size`, and two names that exist on
 * nothing - because "this type has no such attribute" is itself an answer the
 * engines have to give alike.
 */
static const char * const gta_gen_attributes[] = {
  "length", "byte_length", "html", "html_attribute", "percent",
  "javascript", "render", "raw", "size", "k0", "nope",
};

static const char * const gta_gen_cast_types[] = { "int", "float", "bool", "string" };

/// Binary operators, and whether the result can be as large as an operand.
static const char * const gta_gen_binary_ops[] = {
  "+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "||",
};

static int gta_gen_expression(GTA_Gen * g, int cap);

/// The variables an expression may read right now.
static bool gta_gen_var_readable(GTA_Gen * g, int index, int cap) {
  if (g->bound[index] > cap) {
    return false;
  }
  // See the file comment: an assignment's right-hand side may not read a
  // variable the surrounding loop body assigns, or the additive growth bound
  // is not a bound.
  return !(g->in_assign_rhs && (g->loop_assigned & (1u << index)));
}

/// A variable to read, or -1 when none is available.
static int gta_gen_pick_readable(GTA_Gen * g, int cap) {
  int usable[GTA_GEN_VARS + GTA_GEN_MAX_PARAMS];
  int count = 0;
  for (int i = 0; i < GTA_GEN_VARS + g->params; ++i) {
    if (gta_gen_var_readable(g, i, cap)) {
      usable[count++] = i;
    }
  }
  return count ? usable[gta_gen_pick(g, (uint32_t)count)] : -1;
}

/// Emit the name of variable `index`, or of a parameter when index >= VARS.
static void gta_gen_emit_var(GTA_Gen * g, int index) {
  if (index < GTA_GEN_VARS) {
    gta_gen_emit(g, gta_gen_var_names[index]);
  }
  else {
    gta_gen_emitf(g, "p%d", index - GTA_GEN_VARS);
  }
}

/// A short, scalar-sized expression: an index, a slice bound, a step.
static int gta_gen_small_expression(GTA_Gen * g, int cap) {
  if (gta_gen_tight(g) || gta_gen_pick(g, 4) == 0) {
    gta_gen_emit(g, gta_gen_integers[gta_gen_pick(g,
      (uint32_t)(sizeof(gta_gen_integers) / sizeof(*gta_gen_integers)))]);
    return GTA_GEN_SCALAR;
  }
  return gta_gen_expression(g, cap);
}

/**
 * The right-hand side of an assignment, with the loop rule in force.
 */
static int gta_gen_assign_rhs(GTA_Gen * g, int cap) {
  bool saved = g->in_assign_rhs;
  g->in_assign_rhs = true;
  int bound = gta_gen_expression(g, cap);
  g->in_assign_rhs = saved;
  return bound;
}

/**
 * Write an assignment and update the bound of what it assigns.
 *
 * Returns the bound of the assigned value, which is also the value of the
 * expression (4.13).  `as_statement` appends the semicolon.
 */
static int gta_gen_assignment(GTA_Gen * g, bool as_statement) {
  // Which variables this may assign.  In a loop, only those the loop already
  // knows it assigns, so that no expression in the body read one of them
  // before it started growing.
  int usable[GTA_GEN_VARS];
  int count = 0;
  for (int i = 0; i < GTA_GEN_VARS; ++i) {
    if (!g->loop_assigned || (g->loop_assigned & (1u << i))) {
      usable[count++] = i;
    }
  }
  if (!count) {
    gta_gen_emit(g, "0");
    if (as_statement) {
      gta_gen_emit(g, ";");
    }
    return GTA_GEN_SCALAR;
  }
  int target = usable[gta_gen_pick(g, (uint32_t)count)];

  int form = (int)gta_gen_pick(g, 8);
  int bound;
  if (form < 4) {
    // target = expression
    gta_gen_emit_var(g, target);
    gta_gen_emit(g, " = ");
    bound = gta_gen_assign_rhs(g, GTA_GEN_ARG_CAP);
    if (bound > g->bound[target]) {
      g->bound[target] = bound;
    }
  }
  else if (form < 6) {
    // target op= expression.  The accumulator shape: the only one that may
    // grow across iterations, and the growth is charged for here.
    static const char * const compound[] = { "+=", "-=", "*=", "/=", "%=" };
    gta_gen_emit_var(g, target);
    gta_gen_emitf(g, " %s ", compound[gta_gen_pick(g, 5)]);
    int rhs = gta_gen_assign_rhs(g, GTA_GEN_ARG_CAP);
    long grown = g->bound[target] + g->multiplier * rhs;
    g->bound[target] = grown > GTA_GEN_SIZE_CAP ? GTA_GEN_SIZE_CAP : (int)grown;
    bound = g->bound[target];
  }
  else if (form == 6) {
    // target[index] = expression.  A literal index, so that the container
    // cannot be grown past a fixed number of elements however many times the
    // assignment runs.
    gta_gen_emit_var(g, target);
    gta_gen_emitf(g, "[%d] = ", (int)gta_gen_pick(g, 13) - 4);
    bound = gta_gen_assign_rhs(g, GTA_GEN_ARG_CAP);
    g->bound[target] = GTA_GEN_SIZE_CAP;
  }
  else {
    // target.name = expression, which the language intends for map members
    // and currently accepts on everything (4.13).
    gta_gen_emit_var(g, target);
    gta_gen_emitf(g, ".%s = ", gta_gen_key_names[gta_gen_pick(g, 4)]);
    bound = gta_gen_assign_rhs(g, GTA_GEN_ARG_CAP);
    g->bound[target] = GTA_GEN_SIZE_CAP;
  }

  if (as_statement) {
    gta_gen_emit(g, ";");
  }
  return bound;
}

/**
 * An expression whose printed size is bounded by `cap`.
 *
 * Every production either returns a bound no larger than `cap` or is not
 * chosen.  `cap` is never below GTA_GEN_SCALAR, which is what a single number
 * can print to, so there is always a production available.
 */
static int gta_gen_expression(GTA_Gen * g, int cap) {
  if (cap < GTA_GEN_SCALAR) {
    cap = GTA_GEN_SCALAR;
  }

  // Out of depth, out of source, or out of bytes: a literal, which recurses
  // into nothing.
  if (gta_gen_tight(g) || (g->expr_depth >= GTA_GEN_EXPR_DEPTH)) {
    gta_gen_emit(g, gta_gen_integers[gta_gen_pick(g,
      (uint32_t)(sizeof(gta_gen_integers) / sizeof(*gta_gen_integers)))]);
    return GTA_GEN_SCALAR;
  }

  ++g->expr_depth;
  int bound = GTA_GEN_SCALAR;
  int half = cap / 2;
  if (half < GTA_GEN_SCALAR) {
    half = 0;
  }

  switch (gta_gen_pick(g, 24)) {
    case 0: {
      gta_gen_emit(g, gta_gen_integers[gta_gen_pick(g,
        (uint32_t)(sizeof(gta_gen_integers) / sizeof(*gta_gen_integers)))]);
      break;
    }
    case 1: {
      gta_gen_emit(g, gta_gen_floats[gta_gen_pick(g,
        (uint32_t)(sizeof(gta_gen_floats) / sizeof(*gta_gen_floats)))]);
      break;
    }
    case 2: {
      static const char * const atoms[] = { "true", "false", "null" };
      gta_gen_emit(g, atoms[gta_gen_pick(g, 3)]);
      break;
    }
    case 3:
    case 4: {
      const char * body = gta_gen_strings[gta_gen_pick(g,
        (uint32_t)(sizeof(gta_gen_strings) / sizeof(*gta_gen_strings)))];
      const char * prefix = gta_gen_string_prefixes[gta_gen_pick(g, 3)];
      gta_gen_emitf(g, "%s\"%s\"", prefix, body);
      // An escape decodes to fewer characters than it is written with, and a
      // percent- or HTML-encoded one renders to more.  Six times the written
      // length covers both directions.
      bound = (int)strlen(body) * 6 + 8;
      if (bound > cap) {
        bound = cap;
      }
      break;
    }
    case 5:
    case 6:
    case 7: {
      int v = gta_gen_pick_readable(g, cap);
      if (v < 0) {
        gta_gen_emit(g, "null");
        bound = GTA_GEN_SCALAR;
      }
      else {
        gta_gen_emit_var(g, v);
        bound = v < GTA_GEN_VARS ? g->bound[v] : GTA_GEN_ARG_CAP;
      }
      break;
    }
    case 8:
    case 9:
    case 10: {
      // A binary operator.  `+` and `*` can produce something larger than
      // either operand - `+` concatenates and `*` repeats - so those two
      // split the budget; the rest return an operand or a number.
      const char * op = gta_gen_binary_ops[gta_gen_pick(g,
        (uint32_t)(sizeof(gta_gen_binary_ops) / sizeof(*gta_gen_binary_ops)))];
      bool concatenates = (op[0] == '+');
      if (concatenates && !half) {
        op = "-";
        concatenates = false;
      }
      if (op[0] == '*') {
        // The growth is multiplicative, so the factor has to be a number the
        // generator picked rather than whatever an expression evaluates to.
        int factor = 1 + (int)gta_gen_pick(g, GTA_GEN_REPEAT_MAX);
        int left_cap = cap / factor;
        if (left_cap < GTA_GEN_SCALAR) {
          left_cap = GTA_GEN_SCALAR;
          factor = 1;
        }
        gta_gen_emit(g, "(");
        int left = gta_gen_expression(g, left_cap);
        gta_gen_emitf(g, " * %d)", factor);
        bound = left * factor;
        if (bound > cap) {
          bound = cap;
        }
        break;
      }
      gta_gen_emit(g, "(");
      int left = gta_gen_expression(g, concatenates ? half : cap);
      gta_gen_emitf(g, " %s ", op);
      int right = gta_gen_expression(g, concatenates ? half : cap);
      gta_gen_emit(g, ")");
      bound = concatenates
        ? left + right
        : (left > right ? left : right);
      break;
    }
    case 11: {
      if (gta_gen_rare(g, 4)) {
        // The most negative integer, which has no literal spelling of its own
        // (2.6) and so can only be reached through this one form.
        gta_gen_emit(g, "(-9223372036854775808)");
        bound = GTA_GEN_SCALAR;
      }
      else {
        gta_gen_emitf(g, "(%s", gta_gen_pick(g, 2) ? "-" : "!");
        bound = gta_gen_expression(g, cap);
        gta_gen_emit(g, ")");
        if (bound > GTA_GEN_SCALAR) {
          bound = GTA_GEN_SCALAR;
        }
      }
      break;
    }
    case 12: {
      gta_gen_emit(g, "(");
      (void)gta_gen_expression(g, GTA_GEN_SCALAR);
      gta_gen_emit(g, " ? ");
      int a = gta_gen_expression(g, cap);
      gta_gen_emit(g, " : ");
      int b = gta_gen_expression(g, cap);
      gta_gen_emit(g, ")");
      bound = a > b ? a : b;
      break;
    }
    case 13: {
      const char * type = gta_gen_cast_types[gta_gen_pick(g, 4)];
      gta_gen_emit(g, "(");
      int operand = gta_gen_expression(g, cap);
      gta_gen_emitf(g, " as %s)", type);
      // `as string` is the printed form, so it keeps the operand's bound; the
      // other three produce a number or a boolean.
      bound = (type[0] == 's') ? operand : GTA_GEN_SCALAR;
      break;
    }
    case 14:
    case 15: {
      // Index.  Reading past either end is defined (4.8), so the index is an
      // ordinary expression rather than something kept in range.
      gta_gen_emit(g, "(");
      bound = gta_gen_expression(g, cap);
      gta_gen_emit(g, "[");
      (void)gta_gen_small_expression(g, GTA_GEN_SCALAR);
      gta_gen_emit(g, "])");
      break;
    }
    case 16: {
      // Slice.  Each part is optional, and a slice never returns more than it
      // was given.
      gta_gen_emit(g, "(");
      bound = gta_gen_expression(g, cap);
      gta_gen_emit(g, "[");
      if (gta_gen_pick(g, 3)) {
        (void)gta_gen_small_expression(g, GTA_GEN_SCALAR);
      }
      gta_gen_emit(g, ":");
      if (gta_gen_pick(g, 3)) {
        (void)gta_gen_small_expression(g, GTA_GEN_SCALAR);
      }
      if (gta_gen_rare(g, 3)) {
        gta_gen_emit(g, ":");
        (void)gta_gen_small_expression(g, GTA_GEN_SCALAR);
      }
      gta_gen_emit(g, "])");
      break;
    }
    case 17: {
      // The operand is parenthesised because `255.size` does not mean what it
      // looks like: `255.` is already a complete float literal (2.6), so the
      // scanner hands the parser a float followed by an identifier and the
      // program is a syntax error rather than an attribute read.
      gta_gen_emit(g, "((");
      int operand = gta_gen_expression(g, cap);
      gta_gen_emitf(g, ").%s)", gta_gen_attributes[gta_gen_pick(g,
        (uint32_t)(sizeof(gta_gen_attributes) / sizeof(*gta_gen_attributes)))]);
      // `.render`, `.html` and friends return an encoded form of the operand,
      // and percent-encoding is three characters per byte.
      long grown = (long)operand * 3 + 8;
      bound = grown > cap ? cap : (int)grown;
      break;
    }
    case 18: {
      // A call.  Only functions already declared: a name declared later is a
      // compile error (13.15), and a program that does not compile is a
      // wasted execution rather than a test.
      if (!g->fn_count) {
        gta_gen_emit(g, "null");
        break;
      }
      int which = (int)gta_gen_pick(g, (uint32_t)g->fn_count);
      if (gta_gen_rare(g, 8)) {
        // The function itself, uncalled.  A function is a value (section 7),
        // and this is the only way one reaches a position where it is printed
        // or compared - which is where the two engines had different things to
        // say about it (13.37).
        gta_gen_emitf(g, "f%d", which);
        bound = GTA_GEN_SCALAR;
        break;
      }
      int arity = g->fn[which].arity;
      // Occasionally the wrong number of arguments, which is an error value
      // (4.11) and so something the two engines have to produce alike.
      if (gta_gen_rare(g, 16)) {
        arity = (int)gta_gen_pick(g, GTA_GEN_MAX_PARAMS + 1);
      }
      g->work -= g->multiplier * g->fn[which].work;
      gta_gen_emitf(g, "f%d(", which);
      for (int i = 0; i < arity; ++i) {
        if (i) {
          gta_gen_emit(g, ", ");
        }
        (void)gta_gen_expression(g, GTA_GEN_ARG_CAP);
      }
      gta_gen_emit(g, ")");
      bound = GTA_GEN_ARG_CAP;
      break;
    }
    case 19: {
      // print, which yields null and appends to the output.
      gta_gen_emit(g, "print(");
      int printed = gta_gen_expression(g, cap);
      gta_gen_emit(g, ")");
      g->output -= g->multiplier * printed;
      bound = GTA_GEN_SCALAR;
      break;
    }
    case 20: {
      int count = (int)gta_gen_pick(g, 4);
      int per = count ? cap / count : cap;
      if (per < GTA_GEN_SCALAR) {
        per = GTA_GEN_SCALAR;
        count = cap / per;
      }
      gta_gen_emit(g, "[");
      bound = 2;
      for (int i = 0; i < count; ++i) {
        if (i) {
          gta_gen_emit(g, ", ");
          bound += 2;
        }
        bound += gta_gen_expression(g, per);
      }
      gta_gen_emit(g, "]");
      if (bound > cap) {
        bound = cap;
      }
      break;
    }
    case 21: {
      int count = (int)gta_gen_pick(g, 4);
      if (!count) {
        gta_gen_emit(g, "{:}");
        bound = 3;
        break;
      }
      int per = cap / count;
      if (per < GTA_GEN_SCALAR) {
        per = GTA_GEN_SCALAR;
        count = cap / per;
      }
      gta_gen_emit(g, "{");
      bound = 2;
      for (int i = 0; i < count; ++i) {
        if (i) {
          gta_gen_emit(g, ", ");
        }
        gta_gen_emitf(g, "%s: ", gta_gen_key_names[gta_gen_pick(g, 4)]);
        bound += gta_gen_expression(g, per) + 8;
      }
      gta_gen_emit(g, "}");
      if (bound > cap) {
        bound = cap;
      }
      break;
    }
    case 22: {
      gta_gen_emit(g, "(");
      bound = gta_gen_assignment(g, false);
      gta_gen_emit(g, ")");
      break;
    }
    default: {
      gta_gen_emit(g, "(");
      bound = gta_gen_expression(g, cap);
      gta_gen_emit(g, ")");
      break;
    }
  }

  --g->expr_depth;
  return bound > cap ? cap : bound;
}

static void gta_gen_statement(GTA_Gen * g, bool template_mode);

/// A brace-enclosed run of statements, which is what every loop body is: the
/// loop generators put their own bookkeeping statement inside it.
static void gta_gen_statements(GTA_Gen * g, int count, bool template_mode) {
  for (int i = 0; i < count && !gta_gen_tight(g); ++i) {
    gta_gen_statement(g, template_mode);
    gta_gen_emit(g, " ");
  }
}

/**
 * Open a loop body: choose which variables it may assign, and charge the
 * iteration count to the multiplier.
 *
 * The set has to be chosen before the body is written rather than collected as
 * it is written, because a statement early in the body reads what a statement
 * later in the body will have grown by the next iteration.  Collecting it
 * would make the first statement's read legal on the evidence available at the
 * time and wrong by the end.
 */
static void gta_gen_loop_enter(GTA_Gen * g, long iterations, unsigned * saved_assigned,
    long * saved_multiplier) {
  *saved_assigned = g->loop_assigned;
  *saved_multiplier = g->multiplier;
  unsigned chosen = 0;
  int how_many = 1 + (int)gta_gen_pick(g, 3);
  for (int i = 0; i < how_many; ++i) {
    chosen |= 1u << gta_gen_pick(g, GTA_GEN_VARS);
  }
  g->loop_assigned |= chosen;
  g->multiplier *= iterations;
  ++g->loop_depth;
}

static void gta_gen_loop_leave(GTA_Gen * g, unsigned saved_assigned, long saved_multiplier) {
  --g->loop_depth;
  g->loop_assigned = saved_assigned;
  g->multiplier = saved_multiplier;
}

/// Text a template may emit.  Nothing here can be read as `<%` or `%>`.
static const char * const gta_gen_text[] = {
  "hello", "<p>", "&", " ", "\n", "x", "100%", "a<b", "", "=",
};

static void gta_gen_emit_body(GTA_Gen * g, bool template_mode) {
  gta_gen_emit(g, "{ ");
  gta_gen_statements(g, 1 + (int)gta_gen_pick(g, 3), template_mode);
  gta_gen_emit(g, "} ");
}

static void gta_gen_statement(GTA_Gen * g, bool template_mode) {
  // Out of source, out of budget, or too deep: the cheapest statement there
  // is, which contains no sub-statement and no loop.
  if (gta_gen_tight(g) || (g->work <= 0) || (g->output <= 0)) {
    gta_gen_emit(g, "0;");
    return;
  }
  g->work -= g->multiplier;

  uint32_t choice = gta_gen_pick(g, 24);

  // Loops only where one more is affordable.  GTA_GEN_LOOP_MAX iterations at
  // GTA_GEN_LOOP_DEPTH nesting is the whole multiplier this can reach.
  bool may_loop = (g->loop_depth < GTA_GEN_LOOP_DEPTH)
    && (g->work > g->multiplier * GTA_GEN_LOOP_MAX * 4);

  switch (choice) {
    case 0: case 1: case 2: case 3: case 4: {
      (void)gta_gen_expression(g, GTA_GEN_SIZE_CAP);
      gta_gen_emit(g, ";");
      break;
    }
    case 5: case 6: case 7: case 8: case 9: {
      (void)gta_gen_assignment(g, true);
      break;
    }
    case 10: case 11: {
      gta_gen_emit(g, "if (");
      (void)gta_gen_expression(g, GTA_GEN_SCALAR);
      gta_gen_emit(g, ") ");
      gta_gen_emit_body(g, template_mode);
      if (gta_gen_pick(g, 2)) {
        gta_gen_emit(g, "else ");
        gta_gen_emit_body(g, template_mode);
      }
      break;
    }
    case 12: case 13: {
      if (!may_loop) {
        (void)gta_gen_expression(g, GTA_GEN_SIZE_CAP);
        gta_gen_emit(g, ";");
        break;
      }
      // while, counted down by a variable the expression generator cannot
      // name.  The decrement is the first statement of the body so that a
      // `continue` further down cannot skip it.
      long iterations = 1 + gta_gen_pick(g, GTA_GEN_LOOP_MAX);
      int counter = g->loop_depth;
      gta_gen_emitf(g, "q%d = %ld; ", counter, iterations);
      unsigned saved_assigned;
      long saved_multiplier;
      gta_gen_loop_enter(g, iterations, &saved_assigned, &saved_multiplier);
      gta_gen_emitf(g, "while (q%d) { q%d = q%d - 1; ", counter, counter, counter);
      gta_gen_statements(g, 1 + (int)gta_gen_pick(g, 3), template_mode);
      gta_gen_emit(g, "} ");
      gta_gen_loop_leave(g, saved_assigned, saved_multiplier);
      break;
    }
    case 14: {
      if (!may_loop) {
        (void)gta_gen_assignment(g, true);
        break;
      }
      long iterations = 1 + gta_gen_pick(g, GTA_GEN_LOOP_MAX);
      int counter = g->loop_depth;
      gta_gen_emitf(g, "q%d = %ld; ", counter, iterations);
      unsigned saved_assigned;
      long saved_multiplier;
      gta_gen_loop_enter(g, iterations, &saved_assigned, &saved_multiplier);
      gta_gen_emitf(g, "do { q%d = q%d - 1; ", counter, counter);
      gta_gen_statements(g, 1 + (int)gta_gen_pick(g, 3), template_mode);
      gta_gen_emitf(g, "} while (q%d); ", counter);
      gta_gen_loop_leave(g, saved_assigned, saved_multiplier);
      break;
    }
    case 15: case 16: {
      if (!may_loop) {
        (void)gta_gen_assignment(g, true);
        break;
      }
      long iterations = 1 + gta_gen_pick(g, GTA_GEN_LOOP_MAX);
      int counter = g->loop_depth;
      unsigned saved_assigned;
      long saved_multiplier;
      gta_gen_loop_enter(g, iterations, &saved_assigned, &saved_multiplier);
      gta_gen_emitf(g, "for (q%d = %ld; q%d; q%d = q%d - 1) ",
        counter, iterations, counter, counter, counter);
      gta_gen_emit_body(g, template_mode);
      gta_gen_loop_leave(g, saved_assigned, saved_multiplier);
      break;
    }
    case 17: case 18: {
      if (!may_loop) {
        (void)gta_gen_expression(g, GTA_GEN_SIZE_CAP);
        gta_gen_emit(g, ";");
        break;
      }
      // Ranged for.  The iterable is written on the spot rather than named,
      // because mutating the array being iterated is permitted (5.6) and is a
      // way to make the loop run forever; an anonymous one the body cannot
      // reach cannot be mutated.
      int elements = (int)gta_gen_pick(g, GTA_GEN_LOOP_MAX + 1);
      int variable = (int)gta_gen_pick(g, GTA_GEN_VARS);
      unsigned saved_assigned;
      long saved_multiplier;
      gta_gen_emit(g, "for (");
      gta_gen_emit_var(g, variable);
      gta_gen_emit(g, " : [");
      for (int i = 0; i < elements; ++i) {
        if (i) {
          gta_gen_emit(g, ", ");
        }
        (void)gta_gen_expression(g, GTA_GEN_ARG_CAP);
      }
      gta_gen_emit(g, "]) ");
      gta_gen_loop_enter(g, elements ? elements : 1, &saved_assigned, &saved_multiplier);
      // The loop binds the variable, so it is one the body assigns.
      g->loop_assigned |= 1u << variable;
      if (g->bound[variable] < GTA_GEN_ARG_CAP) {
        g->bound[variable] = GTA_GEN_ARG_CAP;
      }
      gta_gen_emit_body(g, template_mode);
      gta_gen_loop_leave(g, saved_assigned, saved_multiplier);
      break;
    }
    case 19: {
      gta_gen_emit_body(g, template_mode);
      break;
    }
    case 20: {
      // break and continue.  Outside a loop they are not errors: they end the
      // program at top level and return null from a function (5.7), which is
      // worth reaching, but only rarely - everything after one is dead.
      if (g->loop_depth || gta_gen_rare(g, 16)) {
        gta_gen_emit(g, gta_gen_pick(g, 2) ? "break;" : "continue;");
      }
      else {
        (void)gta_gen_assignment(g, true);
      }
      break;
    }
    case 21: {
      if (g->in_function || gta_gen_rare(g, 16)) {
        if (gta_gen_pick(g, 4)) {
          gta_gen_emit(g, "return ");
          (void)gta_gen_expression(g, GTA_GEN_ARG_CAP);
          gta_gen_emit(g, ";");
        }
        else {
          gta_gen_emit(g, "return;");
        }
      }
      else {
        (void)gta_gen_expression(g, GTA_GEN_SIZE_CAP);
        gta_gen_emit(g, ";");
      }
      break;
    }
    case 22: {
      if (!template_mode) {
        (void)gta_gen_expression(g, GTA_GEN_SIZE_CAP);
        gta_gen_emit(g, ";");
        break;
      }
      // Leave code, emit literal text, come back.  Template text is TRUSTED
      // (section 8), so what it proves is that the tag of what surrounds an
      // interpolation survives the join.
      const char * text = gta_gen_text[gta_gen_pick(g,
        (uint32_t)(sizeof(gta_gen_text) / sizeof(*gta_gen_text)))];
      gta_gen_emitf(g, "%%> %s <%%", text);
      g->output -= g->multiplier * ((long)strlen(text) + 2);
      break;
    }
    default: {
      if (!template_mode) {
        (void)gta_gen_assignment(g, true);
        break;
      }
      gta_gen_emit(g, "%><%= ");
      int printed = gta_gen_expression(g, GTA_GEN_ARG_CAP);
      gta_gen_emit(g, " %><%");
      g->output -= g->multiplier * printed;
      break;
    }
  }
}

/**
 * Declare a function.
 *
 * A function body is generated in its own scope: assigning to an identifier
 * inside a function binds it locally and there is no implicit capture (section
 * 6), so the bounds the program scope is carrying say nothing about what these
 * names hold here.  They are saved and replaced, not shared.
 */
static void gta_gen_function(GTA_Gen * g, int index, bool template_mode) {
  int arity = (int)gta_gen_pick(g, GTA_GEN_MAX_PARAMS + 1);

  int saved_bound[GTA_GEN_VARS + GTA_GEN_MAX_PARAMS];
  memcpy(saved_bound, g->bound, sizeof(saved_bound));
  unsigned saved_assigned = g->loop_assigned;
  long saved_multiplier = g->multiplier;
  bool saved_in_function = g->in_function;
  int saved_params = g->params;
  long work_before = g->work;

  for (int i = 0; i < GTA_GEN_VARS; ++i) {
    // A name never assigned in this function reads as null (section 6).
    g->bound[i] = GTA_GEN_SCALAR;
  }
  for (int i = 0; i < arity; ++i) {
    // A parameter holds whatever the call passed, and calls pass arguments
    // generated under this cap.
    g->bound[GTA_GEN_VARS + i] = GTA_GEN_ARG_CAP;
  }
  g->loop_assigned = 0;
  g->multiplier = 1;
  g->in_function = true;
  g->params = arity;

  gta_gen_emitf(g, "function f%d(", index);
  for (int i = 0; i < arity; ++i) {
    gta_gen_emitf(g, "%sp%d", i ? ", " : "", i);
  }
  gta_gen_emit(g, ") { ");
  gta_gen_statements(g, 1 + (int)gta_gen_pick(g, 3), template_mode);
  gta_gen_emit(g, "} ");

  long spent = work_before - g->work;
  g->fn[index].arity = arity;
  // What one call costs.  Charged again at every call site, which is what
  // keeps a chain of calls inside a loop inside the budget.
  g->fn[index].work = spent > 0 ? spent : 1;

  memcpy(g->bound, saved_bound, sizeof(saved_bound));
  g->loop_assigned = saved_assigned;
  g->multiplier = saved_multiplier;
  g->in_function = saved_in_function;
  g->params = saved_params;
}

/**
 * Write a whole program.
 *
 * Returns the source, which lives in `g` and is NUL-terminated, and says
 * through `is_template` which of the two parse entry points it is for.
 */
static const char * gta_gen_program(GTA_Gen * g, const uint8_t * data, size_t size,
    bool * is_template) {
  memset(g, 0, sizeof(*g));
  g->data = data;
  g->size = size;
  g->work = GTA_GEN_WORK;
  g->multiplier = 1;
  g->output = GTA_GEN_OUTPUT;
  for (int i = 0; i < GTA_GEN_VARS + GTA_GEN_MAX_PARAMS; ++i) {
    g->bound[i] = GTA_GEN_SCALAR;
  }

  // One byte decides the mode, so that a mutation can flip a program between
  // the two parsers without changing anything else about it.
  bool template_mode = (gta_gen_byte(g) & 3) == 0;
  *is_template = template_mode;
  if (template_mode) {
    gta_gen_emit(g, "<% ");
  }

  int functions = (int)gta_gen_pick(g, GTA_GEN_FUNCS + 1);
  for (int i = 0; i < functions && !gta_gen_tight(g); ++i) {
    gta_gen_function(g, i, template_mode);
    g->fn_count = i + 1;
  }

  gta_gen_statements(g, 2 + (int)gta_gen_pick(g, 8), template_mode);

  if (template_mode) {
    gta_gen_emit(g, " %>");
  }
  // `g->full` says whether the hard limit was reached, which means the source
  // is cut off mid-construct and will not parse.  The soft limit above exists
  // to keep that rare; the harness counts them so that "rare" is a figure
  // rather than an intention.
  return g->source;
}

#endif // GHOTIIO_TANG_FUZZ_GENERATOR_H
