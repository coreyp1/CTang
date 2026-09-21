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
 */

#ifndef GHOTI_IO_GTA_PROGRAM_LANGUAGE_H
#define GHOTI_IO_GTA_PROGRAM_LANGUAGE_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * This structure holds the metadata pertaining to a language.
 *
 * A program, when created, is associated with a language.  This allows a
 * single host program to support multiple incarnations of Tang by tailoring
 * the details of the language (such as the available libraries) to the needs
 * of the program(s) with which it is interacting.
 */
struct GTA_Language {
  /**
   * The general libraries available to the language.
   */
  GTA_Library * library;
};

/**
 * Create a new language.
 *
 * @return A pointer to the new language structure, or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Language * gta_language_create(void);

/**
 * Destroy the given language.
 *
 * @param language The language to destroy.
 */
GTA_API void gta_language_destroy(GTA_Language * language);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_PROGRAM_LANGUAGE_H
