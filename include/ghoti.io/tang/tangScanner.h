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
 * Declare the GTA_Scanner used to tokenize a Tang script.
 */

#ifndef GHOTI_IO_GTA_TANGSCANNER_H
#define GHOTI_IO_GTA_TANGSCANNER_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define YYSTYPE GTA_PARSER_STYPE
#define YYLTYPE GTA_PARSER_LTYPE

#include "tangParser.h"

// Our scanner will populate an internal buffer so that strings can be properly
// interpreted (and null-terminated) by the parser.  The buffer will be passed
// back to the parser as the tokens (IDENTIFIER, STRING, etc.) are recognized.
//
// In the event of a parse error, the parser will need to be able to clean up
// any memory that was allocated for the buffer, hence passing in this pointer
// by reference.
#undef YY_DECL
#define YY_DECL int gta_scanner_get_next_token(GTA_PARSER_STYPE * yylval_param, GTA_PARSER_LTYPE * yylloc_param , yyscan_t yyscanner)

// The actual declaration of the scanner function.
YY_DECL;

// #define YY_DECL Tang::TangParser::symbol_type Tang::TangScanner::get_next_token()

// Now, the normal header contents.

// #include "tangParser.h"


    /**
     * Helper function to set the scanner to template parsing mode.
     */
    // void setModeTemplate() {
      // Formula is taken from the BEGIN macro in tangScanner.cpp.
      // (2) is the value that Flex has assigned the TEMPLATE rule type from
      // tpl.l.
      // this->yy_start = 1 + 2 * (2);
    // }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_TANGSCANNER_H

