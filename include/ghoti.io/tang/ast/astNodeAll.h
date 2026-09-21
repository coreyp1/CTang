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
 * Include all AST node headers.
 */

#ifndef GHOTI_IO_GTA_AST_ASTNODEALL_H
#define GHOTI_IO_GTA_AST_ASTNODEALL_H

#include <ghoti.io/tang/macros.h>

#include <ghoti.io/tang/ast/astNode.h>
#include <ghoti.io/tang/ast/astNodeArray.h>
#include <ghoti.io/tang/ast/astNodeAssign.h>
#include <ghoti.io/tang/ast/astNodeBinary.h>
#include <ghoti.io/tang/ast/astNodeBlock.h>
#include <ghoti.io/tang/ast/astNodeBoolean.h>
#include <ghoti.io/tang/ast/astNodeBreak.h>
#include <ghoti.io/tang/ast/astNodeCast.h>
#include <ghoti.io/tang/ast/astNodeContinue.h>
#include <ghoti.io/tang/ast/astNodeDoWhile.h>
#include <ghoti.io/tang/ast/astNodeFloat.h>
#include <ghoti.io/tang/ast/astNodeFor.h>
#include <ghoti.io/tang/ast/astNodeFunction.h>
#include <ghoti.io/tang/ast/astNodeFunctionCall.h>
#include <ghoti.io/tang/ast/astNodeGlobal.h>
#include <ghoti.io/tang/ast/astNodeIdentifier.h>
#include <ghoti.io/tang/ast/astNodeIfElse.h>
#include <ghoti.io/tang/ast/astNodeIndex.h>
#include <ghoti.io/tang/ast/astNodeInteger.h>
#include <ghoti.io/tang/ast/astNodeLibrary.h>
#include <ghoti.io/tang/ast/astNodeMap.h>
#include <ghoti.io/tang/ast/astNodeParseError.h>
#include <ghoti.io/tang/ast/astNodePeriod.h>
#include <ghoti.io/tang/ast/astNodePrint.h>
#include <ghoti.io/tang/ast/astNodeRangedFor.h>
#include <ghoti.io/tang/ast/astNodeReturn.h>
#include <ghoti.io/tang/ast/astNodeSlice.h>
#include <ghoti.io/tang/ast/astNodeString.h>
#include <ghoti.io/tang/ast/astNodeTernary.h>
#include <ghoti.io/tang/ast/astNodeUnary.h>
#include <ghoti.io/tang/ast/astNodeUse.h>
#include <ghoti.io/tang/ast/astNodeWhile.h>

#endif // GHOTI_IO_GTA_AST_ASTNODEALL_H
