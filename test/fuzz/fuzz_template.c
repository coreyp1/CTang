/**
 * @file
 *
 * libFuzzer harness for the tang template parser and simplifier.
 *
 * The contract being fuzzed: for any byte string at all, gta_tang_parse_template
 * either returns an AST or returns null. It must not crash, read out of
 * bounds, or leak - and an AST it does return must survive being counted,
 * simplified and destroyed.
 *
 * Simplify is included deliberately rather than parsing alone. It is where
 * constant folding lives, so it is where a literal that does not hold its own
 * value shows up, and two of the three float defects fixed in c998084 were on
 * that path.
 *
 * **Execution is not fuzzed, and cannot be yet.** tang is Turing-complete and
 * has no execution count timeout - it is an open TODO in tang.h - so a fuzzer
 * would hang on the first `while (true) {}` it generated, which is a handful
 * of bytes away from any seed. Executing fuzzed input becomes possible when
 * that limit exists, and is worth doing then: the bytecode/JIT differential is
 * a much stronger oracle than "did not crash".
 *
 * Written in C, not C++: ctang's public headers do not compile under
 * clang++ (a [[nodiscard]] placement problem, reported separately), and a C
 * library is the natural thing to fuzz from C anyway.
 *
 * Build with: make fuzz-template     Run: make fuzz-run-template
 *
 * SPDX-License-Identifier: LGPL-3.0-only
 * Copyright (C) 2026 Corey Pennycuff
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ghoti.io/tang/tangLanguage.h>
#include <ghoti.io/tang/ast/astNode.h>

int LLVMFuzzerTestOneInput(const uint8_t * data, size_t size) {
  // The API takes a NUL-terminated string, so that is the contract being
  // tested: a copy with a terminator, and an embedded NUL legitimately ends
  // the source. Passing the raw buffer would be testing an API that does not
  // exist and would report a read past the end as a library bug.
  char * source = malloc(size + 1);
  if (!source) {
    return 0;
  }
  memcpy(source, data, size);
  source[size] = '\0';

  GTA_Ast_Node * ast = gta_tang_parse_template(source);
  if (ast) {
    gta_tang_node_count(ast);
    ast = gta_tang_simplify(ast);
    if (ast) {
      gta_ast_node_destroy(ast);
    }
  }

  free(source);
  return 0;
}
