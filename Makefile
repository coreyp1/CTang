CXX := g++
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -Wno-error=unused-function -Wfatal-errors -std=c++20 -O1 -g
CC := cc
CFLAGS := -pedantic-errors -Wall -Wextra -Werror -Wno-error=unused-function -Wfatal-errors -std=c17 -O0 -g `pkg-config --cflags icu-io icu-i18n icu-uc ghoti.io-cutil-dev`
# -DGHOTIIO_CUTIL_ENABLE_MEMORY_DEBUG
LDFLAGS := -L /usr/lib -lstdc++ -lm `pkg-config --libs --cflags icu-io icu-i18n icu-uc ghoti.io-cutil-dev`
BUILD := ./build
OBJ_DIR := $(BUILD)/objects
GEN_DIR := $(BUILD)/generated
APP_DIR := $(BUILD)/apps

SUITE := ghoti.io
PROJECT := tang
BRANCH := -cdev
BASE_NAME := lib$(SUITE)-$(PROJECT)$(BRANCH).so
MAJOR_VERSION := 0
MINOR_VERSION := 0.0
SO_NAME := $(BASE_NAME).$(MAJOR_VERSION)
TARGET := $(SO_NAME).$(MINOR_VERSION)

INCLUDE := -I include/tang -I include/ -I $(GEN_DIR)/
LIBOBJECTS := \
	$(OBJ_DIR)/tangParser.o \
	$(OBJ_DIR)/tangScanner.o \
	$(OBJ_DIR)/unicodeString.o \
  $(OBJ_DIR)/astNode.o \
	$(OBJ_DIR)/astNodeArray.o \
	$(OBJ_DIR)/astNodeAssign.o \
	$(OBJ_DIR)/astNodeBinary.o \
	$(OBJ_DIR)/astNodeBlock.o \
	$(OBJ_DIR)/astNodeBoolean.o \
	$(OBJ_DIR)/astNodeBreak.o \
	$(OBJ_DIR)/astNodeCast.o \
	$(OBJ_DIR)/astNodeContinue.o \
	$(OBJ_DIR)/astNodeDoWhile.o \
	$(OBJ_DIR)/astNodeFloat.o \
	$(OBJ_DIR)/astNodeFor.o \
	$(OBJ_DIR)/astNodeFunction.o \
	$(OBJ_DIR)/astNodeFunctionCall.o \
	$(OBJ_DIR)/astNodeIdentifier.o \
	$(OBJ_DIR)/astNodeIfElse.o \
	$(OBJ_DIR)/astNodeIndex.o \
	$(OBJ_DIR)/astNodeInteger.o \
	$(OBJ_DIR)/astNodeLibrary.o \
	$(OBJ_DIR)/astNodeMap.o \
	$(OBJ_DIR)/astNodePeriod.o \
	$(OBJ_DIR)/astNodeParseError.o \
	$(OBJ_DIR)/astNodePrint.o \
	$(OBJ_DIR)/astNodeRangedFor.o \
	$(OBJ_DIR)/astNodeReturn.o \
	$(OBJ_DIR)/astNodeSlice.o \
	$(OBJ_DIR)/astNodeString.o \
	$(OBJ_DIR)/astNodeTernary.o \
	$(OBJ_DIR)/astNodeUnary.o \
	$(OBJ_DIR)/astNodeUse.o \
	$(OBJ_DIR)/astNodeWhile.o \
	$(OBJ_DIR)/binaryCompilerContext.o \
	$(OBJ_DIR)/bytecode.o \
	$(OBJ_DIR)/bytecodeCompilerContext.o \
	$(OBJ_DIR)/computedValue.o \
	$(OBJ_DIR)/computedValueBoolean.o \
	$(OBJ_DIR)/computedValueError.o \
	$(OBJ_DIR)/computedValueFloat.o \
	$(OBJ_DIR)/computedValueInteger.o \
	$(OBJ_DIR)/computedValueString.o \
	$(OBJ_DIR)/executionContext.o \
	$(OBJ_DIR)/program.o \
	$(OBJ_DIR)/tangLanguage.o \
	$(OBJ_DIR)/virtualMachine.o \

# \
# $(OBJ_DIR)/computedExpression.o \
# $(OBJ_DIR)/computedExpressionArray.o \
# $(OBJ_DIR)/computedExpressionBoolean.o \
# $(OBJ_DIR)/computedExpressionCompiledFunction.o \
# $(OBJ_DIR)/computedExpressionError.o \
# $(OBJ_DIR)/computedExpressionFloat.o \
# $(OBJ_DIR)/computedExpressionInteger.o \
# $(OBJ_DIR)/computedExpressionIterator.o \
# $(OBJ_DIR)/computedExpressionIteratorEnd.o \
# $(OBJ_DIR)/computedExpressionLibrary.o \
# $(OBJ_DIR)/computedExpressionLibraryMath.o \
# $(OBJ_DIR)/computedExpressionLibraryTang.o \
# $(OBJ_DIR)/computedExpressionMap.o \
# $(OBJ_DIR)/computedExpressionNativeBoundFunction.o \
# $(OBJ_DIR)/computedExpressionNativeFunction.o \
# $(OBJ_DIR)/computedExpressionNativeLibraryFunction.o \
# $(OBJ_DIR)/computedExpressionString.o \
# $(OBJ_DIR)/context.o \
# $(OBJ_DIR)/error.o \
# $(OBJ_DIR)/htmlEscape.o \
# $(OBJ_DIR)/htmlEscapeAscii.o \
# $(OBJ_DIR)/opcode.o \
# $(OBJ_DIR)/percentEncode.o \
# $(OBJ_DIR)/program-analyze.o \
# $(OBJ_DIR)/program-dumpBytecode.o \
# $(OBJ_DIR)/program-execute.o \
# $(OBJ_DIR)/program-optimize.o \
# $(OBJ_DIR)/program.o \
# $(OBJ_DIR)/tangBase.o \
# $(OBJ_DIR)/tangParser.o \
# $(OBJ_DIR)/unescape.o \
# $(OBJ_DIR)/unicodeString.o

TESTFLAGS := `pkg-config --libs --cflags gtest`


TANGLIBRARY := -L $(APP_DIR) -l$(SUITE)-$(PROJECT)$(BRANCH)


all: $(APP_DIR)/$(TARGET) #$(APP_DIR)/tang ## Build the shared library

####################################################################
# Dependency Variables
####################################################################
DEP_LIBVER = \
	include/tang/libver.h
DEP_MACROS = \
	include/tang/macros.h
DEP_UNICODESTRING = \
	include/tang/unicodeString.h \
	$(DEP_MACROS)
DEP_LOCATION = \
  include/tang/location.h
DEP_BYTECODE = \
	include/tang/bytecode.h \
	$(DEP_MACROS)
DEP_BYTECODECOMPILERCONTEXT = \
	include/tang/bytecodeCompilerContext.h \
	$(DEP_MACROS)
DEP_BINARYCOMPILERCONTEXT = \
	include/tang/binaryCompilerContext.h \
	$(DEP_MACROS)

DEP_ASTNODE = \
	include/tang/astNode.h \
	$(GEN_DIR)/tangParser.h \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_LOCATION)
DEP_ASTNODEIDENTIFIER = \
	include/tang/astNodeIdentifier.h \
	$(DEP_ASTNODE)
DEP_ASTNODESTRING = \
	include/tang/astNodeString.h \
	$(DEP_ASTNODE)
DEP_ASTNODEARRAY = \
	include/tang/astNodeArray.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS)
DEP_ASTNODEASSIGN = \
	include/tang/astNodeAssign.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEINDEX = \
	include/tang/astNodeIndex.h \
	$(DEP_ASTNODE)
DEP_ASTNODEBINARY = \
	include/tang/astNodeBinary.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEBLOCK = \
	include/tang/astNodeBlock.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS)
DEP_ASTNODEBOOLEAN = \
	include/tang/astNodeBoolean.h \
	$(DEP_ASTNODE)
DEP_ASTNODEBREAK = \
	include/tang/astNodeBreak.h \
	$(DEP_ASTNODE)
DEP_ASTNODECAST = \
	include/tang/astNodeCast.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODECONTINUE = \
	include/tang/astNodeContinue.h \
	$(DEP_ASTNODE)
DEP_ASTNODEDOWHILE = \
	include/tang/astNodeDoWhile.h \
	$(DEP_ASTNODE)
DEP_ASTNODEFLOAT = \
	include/tang/astNodeFloat.h \
	$(DEP_ASTNODE)
DEP_ASTNODEFOR = \
	include/tang/astNodeFor.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEFUNCTION = \
	include/tang/astNodeFunction.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS)
DEP_ASTNODEFUNCTIONCALL = \
	include/tang/astNodeFunctionCall.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEIFELSE = \
	include/tang/astNodeIfElse.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEINDEX = \
	include/tang/astNodeIndex.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEINTEGER = \
	include/tang/astNodeInteger.h \
	$(DEP_ASTNODE)
DEP_ASTNODELIBRARY = \
	include/tang/astNodeLibrary.h \
	$(DEP_ASTNODE)
DEP_ASTNODEMAP = \
	include/tang/astNodeMap.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS)
DEP_ASTNODEPARSEERROR = \
	include/tang/astNodeParseError.h \
	$(DEP_ASTNODE)
DEP_ASTNODEPERIOD = \
	include/tang/astNodePeriod.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEPRINT = \
	include/tang/astNodePrint.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODERANGEDFOR = \
	include/tang/astNodeFor.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODERETURN = \
	include/tang/astNodeReturn.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODESLICE = \
	include/tang/astNodeSlice.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODETERNARY = \
	include/tang/astNodeTernary.h \
	$(DEP_ASTNODE)
DEP_ASTNODEUNARY = \
	include/tang/astNodeUnary.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEUSE = \
	include/tang/astNodeUse.h \
	$(DEP_ASTNODE)
DEP_ASTNODEWHILE = \
	include/tang/astNodeWhile.h \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)

DEP_ASTNODE_ALL = \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODEARRAY) \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEBLOCK) \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_ASTNODEBREAK) \
	$(DEP_ASTNODECAST) \
	$(DEP_ASTNODECONTINUE) \
	$(DEP_ASTNODEDOWHILE) \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_ASTNODEFOR) \
	$(DEP_ASTNODEFUNCTION) \
	$(DEP_ASTNODEFUNCTIONCALL) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_ASTNODEIFELSE) \
	$(DEP_ASTNODEINDEX) \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_ASTNODELIBRARY) \
	$(DEP_ASTNODEMAP) \
	$(DEP_ASTNODEPARSEERROR) \
	$(DEP_ASTNODEPERIOD) \
	$(DEP_ASTNODEPRINT) \
	$(DEP_ASTNODERANGEDFOR) \
	$(DEP_ASTNODERETURN) \
	$(DEP_ASTNODESLICE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODETERNARY) \
	$(DEP_ASTNODEUNARY) \
	$(DEP_ASTNODEUSE) \
	$(DEP_ASTNODEWHILE)

DEP_COMPUTEDVALUE = \
	include/tang/computedValue.h \
	$(DEP_MACROS)
DEP_COMPUTEDVALUEBOOLEAN = \
	include/tang/computedValueBoolean.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEERROR = \
  include/tang/computedValueError.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEFLOAT = \
	include/tang/computedValueFloat.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEINTEGER = \
	include/tang/computedValueInteger.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUESTRING = \
	include/tang/computedValueString.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_UNICODESTRING)

DEP_COMPUTEDVALUEALL = \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_COMPUTEDVALUEFLOAT) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUESTRING)

DEP_TANGLANGUAGE = \
	$(DEP_ASTNODE)

DEP_EXECUTIONCONTEXT = \
	include/tang/executionContext.h \
	$(DEP_MACROS) \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_UNICODESTRING)

DEP_PROGRAM = \
	include/tang/program.h \
	$(DEP_ASTNODE) \
	$(DEP_BYTECODE) \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_MACROS) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_UNICODESTRING)

DEP_VIRTUALMACHINE = \
	include/tang/virtualMachine.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_MACROS) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_BYTECODE)



DEP_SINGLETONOBJECTPOOL = \
	include/tang/singletonObjectPool.h
DEP_ERROR = \
	include/tang/error.h \
	$(DEP_LOCATION)
DEP_COMPUTEDEXPRESSION = \
	include/tang/computedExpression.h \
	$(DEP_MACROS) \
	$(DEP_ERROR)
DEP_PROGRAMZZZ = \
	include/tang/program.h \
	$(DEP_ASTNODE) \
	$(DEP_COMPUTEDEXPRESSION) \
	$(DEP_CONTEXT) \
	$(DEP_ERROR) \
	$(DEP_MACROS) \
	$(DEP_UNICODESTRING)
DEP_COMPUTEDEXPRESSIONARRAY = \
	include/tang/computedExpressionArray.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONBOOLEAN = \
	include/tang/computedExpressionBoolean.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONCOMPILEDFUNCTION = \
	include/tang/computedExpressionCompiledFunction.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONERROR = \
	include/tang/computedExpressionError.h \
	$(DEP_ERROR) \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONFLOAT = \
	include/tang/computedExpressionFloat.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONINTEGER = \
	include/tang/computedExpressionInteger.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONITERATOR = \
	include/tang/computedExpressionIterator.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONITERATOREND = \
	include/tang/computedExpressionIteratorEnd.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONLIBRARY = \
	include/tang/computedExpressionLibrary.h \
	$(DEP_MACROS) \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONLIBRARYBASE = \
	include/tang/computedExpressionLibraryBase.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONLIBRARYMATH = \
	include/tang/computedExpressionLibraryMath.h \
	$(DEP_COMPUTEDEXPRESSIONLIBRARYBASE)
DEP_COMPUTEDEXPRESSIONLIBRARYTANG = \
	include/tang/computedExpressionLibraryTang.h \
	$(DEP_COMPUTEDEXPRESSIONLIBRARY)
DEP_COMPUTEDEXPRESSIONMAP = \
	include/tang/computedExpressionMap.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONNATIVEBOUNDFUNCTION = \
	include/tang/computedExpressionNativeBoundFunction.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONNATIVEFUNCTION = \
	include/tang/computedExpressionNativeFunction.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONNATIVELIBRARYFUNCTION = \
	include/tang/computedExpressionNativeLibraryFunction.h \
	$(DEP_COMPUTEDEXPRESSION)
DEP_COMPUTEDEXPRESSIONSTRING = \
	include/tang/computedExpressionString.h \
	$(DEP_COMPUTEDEXPRESSION) \
	$(DEP_UNICODESTRING)
DEP_TANGBASE = \
	$(DEP_COMPUTEDEXPRESSION) \
	$(DEP_MACROS) \
	$(DEP_PROGRAM)
DEP_TANG = \
	$(DEP_ASTNODE) \
	$(DEP_COMPUTEDEXPRESSION) \
	$(DEP_COMPUTEDEXPRESSIONERROR) \
	$(DEP_COMPUTEDEXPRESSIONINTEGER) \
	$(DEP_COMPUTEDEXPRESSIONNATIVEFUNCTION) \
	$(DEP_COMPUTEDEXPRESSIONSTRING) \
	$(DEP_CONTEXT) \
	$(DEP_ERROR) \
	$(DEP_MACROS) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM) \
	$(DEP_TANGBASE)

####################################################################
# Bison-Generated Files
####################################################################
$(GEN_DIR)/tangParser.h: \
				bison/tangParser.y \
				include/tang/astNode.h \
				$(DEP_LOCATION)
	@echo "\n### Generating Bison TangParser ###"
	@mkdir -p $(@D)
	bison -v -o $(GEN_DIR)/tangParser.c -d $<

# Ensure that tangParser.c is regenerated properly before
# tangParser.o tries to use it to compile.
$(GEN_DIR)/tangParser.c: $(GEN_DIR)/tangParser.h


####################################################################
# Flex-Generated Files
####################################################################
$(GEN_DIR)/htmlEscape.c: \
				flex/htmlEscape.l
	@echo "\n### Generating The HtmlEscape Scanner ###"
	@mkdir -p $(@D)
	flex -o $@ $<

$(GEN_DIR)/htmlEscapeAscii.c: \
				flex/htmlEscapeAscii.l \
				$(DEP_UNICODESTRING)
	@echo "\n### Generating The HtmlEscapeAscii Scanner ###"
	@mkdir -p $(@D)
	flex -o $@ $<

$(GEN_DIR)/percentEncode.c: \
				flex/percentEncode.l
	@echo "\n### Generating The PercentEncode Scanner ###"
	@mkdir -p $(@D)
	flex -o $@ $<

$(GEN_DIR)/tangScanner.c: \
				flex/tangScanner.l \
				$(GEN_DIR)/tangParser.h \
				$(DEP_UNICODESTRING)
	@echo "\n### Generating Flex TangScanner ###"
	@mkdir -p $(@D)
	flex -o $@ --header-file=$(GEN_DIR)/flexTangScanner.h $<

$(GEN_DIR)/unescape.c: \
				flex/unescape.l
	@echo "\n### Generating The Unescape Scanner ###"
	@mkdir -p $(@D)
	flex -o $@ $<

####################################################################
# Object Files
####################################################################

$(LIBOBJECTS) :
	@echo "\n### Compiling $@ ###"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -MMD -o $@ -fPIC

$(OBJ_DIR)/astNode.o: \
	src/astNode.c \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_BYTECODE) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_PROGRAM)

$(OBJ_DIR)/astNodeArray.o: \
	src/astNodeArray.c \
	$(DEP_ASTNODEARRAY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM)

$(OBJ_DIR)/astNodeAssign.o: \
	src/astNodeAssign.c \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_ASTNODEINDEX) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeBinary.o: \
	src/astNodeBinary.c \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_ASTNODESTRING) \
	$(DEP_UNICODESTRING) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeBlock.o: \
	src/astNodeBlock.c \
	$(DEP_ASTNODEBLOCK) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeBoolean.o: \
	src/astNodeBoolean.c \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeBreak.o: \
	src/astNodeBreak.c \
	$(DEP_ASTNODEBREAK) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeCast.o: \
	src/astNodeCast.c \
	$(DEP_ASTNODECAST) \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_ASTNODESTRING) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeContinue.o: \
	src/astNodeContinue.c \
	$(DEP_ASTNODECONTINUE) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeDoWhile.o: \
	src/astNodeDoWhile.c \
	$(DEP_ASTNODEDOWHILE) \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeFor.o: \
	src/astNodeFor.c \
	$(DEP_ASTNODEFOR) \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeFloat.o: \
	src/astNodeFloat.c \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeFunction.o: \
	src/astNodeFunction.c \
	$(DEP_ASTNODEFUNCTIONDECLARATION) \
	$(DEP_ASTNODESTRING) \
	$(DEP_MACROS) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeFunctionCall.o: \
	src/astNodeFunctionCall.c \
	$(DEP_ASTNODEFUNCTIONCALL) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeIdentifier.o: \
	src/astNodeIdentifier.c \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeIfElse.o: \
	src/astNodeIfElse.c \
	$(DEP_ASTNODEIFELSE) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeIndex.o: \
	src/astNodeIndex.c \
	$(DEP_ASTNODEINDEX) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeInteger.o: \
	src/astNodeInteger.c \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeLibrary.o: \
	src/astNodeLibrary.c \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_ASTNODELIBRARY) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeMap.o: \
	src/astNodeMap.c \
	$(DEP_ASTNODEMAP) \
	$(DEP_ASTNODESTRING) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeParseError.o: \
	src/astNodeParseError.c \
	$(DEP_ASTNODEPARSEERROR)

$(OBJ_DIR)/astNodePeriod.o: \
	src/astNodePeriod.c \
	$(DEP_ASTNODEPERIOD) \
	$(DEP_ASTNODESTRING) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodePrint.o: \
	src/astNodePrint.c \
	$(DEP_ASTNODEPRINT) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeRangedFor.o: \
	src/astNodeRangedFor.c \
	$(DEP_ASTNODERANGEDFOR) \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeReturn.o: \
	src/astNodeReturn.c \
	$(DEP_ASTNODERETURN) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeSlice.o: \
	src/astNodeSlice.c \
	$(DEP_ASTNODESLICE) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeString.o: \
	src/astNodeString.c \
	$(DEP_ASTNODESTRING) \
	$(DEP_UNICODESTRING) \
	$(DEP_COMPUTEDVALUESTRING) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeTernary.o: \
	src/astNodeTernary.c \
	$(DEP_ASTNODETERNARY) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeUnary.o: \
	src/astNodeUnary.c \
	$(DEP_ASTNODEUNARY) \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_ASTNODESTRING) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeUse.o: \
	src/astNodeUse.c \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_ASTNODEUSE) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/astNodeWhile.o: \
	src/astNodeWhile.c \
	$(DEP_ASTNODEWHILE) \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_OPCODE)

$(OBJ_DIR)/tangLanguage.o: \
	src/tangLanguage.c \
	$(DEP_TANGLANGUAGE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_ASTNODEPARSEERROR)

$(OBJ_DIR)/binaryCompilerContext.o : \
	src/binaryCompilerContext.c \
	$(DEP_BINARYCOMPILERCONTEXT) \

$(OBJ_DIR)/bytecode.o: \
	src/bytecode.c \
	$(DEP_BYTECODE)

$(OBJ_DIR)/bytecodeCompilerContext.o: \
	src/bytecodeCompilerContext.c \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_PROGRAM)

$(OBJ_DIR)/computedValue.o: \
	src/computedValue.c \
  $(DEP_COMPUTEDVALUE) \
	$(DEP_MACROS)

$(OBJ_DIR)/computedValueBoolean.o: \
	src/computedValueBoolean.c \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_COMPUTEDVALUEERROR)

$(OBJ_DIR)/computedValueError.o: \
	src/computedValueError.c \
	$(DEP_COMPUTEDVALUEERROR)

$(OBJ_DIR)/computedValueFloat.o: \
	src/computedValueFloat.c \
	$(DEP_COMPUTEDVALUEFLOAT) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_COMPUTEDVALUEINTEGER)

$(OBJ_DIR)/computedValueInteger.o: \
	src/computedValueInteger.c \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_COMPUTEDVALUEFLOAT)

$(OBJ_DIR)/computedValueString.o: \
	src/computedValueString.c \
	$(DEP_COMPUTEDVALUESTRING) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_UNICODESTRING)

$(OBJ_DIR)/program.o: \
	src/program.c \
	$(DEP_PROGRAM) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_TANGLANGUAGE) \
	$(DEP_ASTNODEBLOCK) \
	$(DEP_ASTNODEPARSEERROR) \
	$(DEP_BINARYCOMPILERCONTEXT) \
	$(DEP_BYTECODECOMPILERCONTEXT) \
	$(DEP_VIRTUALMACHINE)

$(OBJ_DIR)/virtualMachine.o: \
	src/virtualMachine.c \
	$(DEP_VIRTUALMACHINE) \
	$(DEP_COMPUTEDVALUEALL)




$(OBJ_DIR)/computedExpression.o: \
				src/computedExpression.c \
				$(DEP_COMPUTEDEXPRESSION) \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONNATIVEBOUNDFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONSTRING) \
				$(DEP_TANGBASE)

$(OBJ_DIR)/computedExpressionArray.o: \
				src/computedExpressionArray.c \
				$(DEP_COMPUTEDEXPRESSIONARRAY) \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONINTEGER) \
				$(DEP_COMPUTEDEXPRESSIONITERATOR) \
				$(DEP_COMPUTEDEXPRESSIONITERATOREND) \
				$(DEP_COMPUTEDEXPRESSIONSTRING)

$(OBJ_DIR)/computedExpressionBoolean.o: \
				src/computedExpressionBoolean.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONFLOAT) \
				$(DEP_COMPUTEDEXPRESSIONINTEGER)

$(OBJ_DIR)/computedExpressionCompiledFunction.o: \
				src/computedExpressionCompiledFunction.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONCOMPILEDFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONFLOAT) \
				$(DEP_COMPUTEDEXPRESSIONSTRING)

$(OBJ_DIR)/computedExpressionError.o: \
				src/computedExpressionError.c \
				$(DEP_COMPUTEDEXPRESSIONERROR)

$(OBJ_DIR)/computedExpressionFloat.o: \
				src/computedExpressionFloat.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONFLOAT) \
				$(DEP_COMPUTEDEXPRESSIONINTEGER) \
				$(DEP_COMPUTEDEXPRESSIONSTRING)

$(OBJ_DIR)/computedExpressionInteger.o: \
				src/computedExpressionInteger.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONFLOAT) \
				$(DEP_COMPUTEDEXPRESSIONINTEGER) \
				$(DEP_COMPUTEDEXPRESSIONSTRING)

$(OBJ_DIR)/computedExpressionIterator.o: \
				src/computedExpressionIterator.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONINTEGER) \
				$(DEP_COMPUTEDEXPRESSIONITERATOR) \
				$(DEP_COMPUTEDEXPRESSIONSTRING)

$(OBJ_DIR)/computedExpressionIteratorEnd.o: \
				src/computedExpressionIteratorEnd.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONINTEGER) \
				$(DEP_COMPUTEDEXPRESSIONITERATOREND) \
				$(DEP_COMPUTEDEXPRESSIONSTRING)

$(OBJ_DIR)/computedExpressionLibrary.o: \
				src/computedExpressionLibrary.c \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONLIBRARY) \
				$(DEP_COMPUTEDEXPRESSIONNATIVELIBRARYFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONSTRING) \
				$(DEP_TANGBASE)

$(OBJ_DIR)/computedExpressionLibraryMath.o: \
				src/computedExpressionLibraryMath.c \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONFLOAT) \
				$(DEP_COMPUTEDEXPRESSIONINTEGER) \
				$(DEP_COMPUTEDEXPRESSIONLIBRARYMATH) \
				$(DEP_COMPUTEDEXPRESSIONNATIVEFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONNATIVELIBRARYFUNCTION) \
				$(DEP_TANGBASE)

$(OBJ_DIR)/computedExpressionLibraryTang.o: \
				src/computedExpressionLibraryTang.c \
				$(DEP_COMPUTEDEXPRESSIONLIBRARYTANG) \
				$(DEP_COMPUTEDEXPRESSIONSTRING) \
				$(DEP_TANGBASE)

$(OBJ_DIR)/computedExpressionMap.o: \
				src/computedExpressionMap.c \
				$(DEP_COMPUTEDEXPRESSIONARRAY) \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONITERATOR) \
				$(DEP_COMPUTEDEXPRESSIONITERATOREND) \
				$(DEP_COMPUTEDEXPRESSIONMAP) \
				$(DEP_COMPUTEDEXPRESSIONSTRING)

$(OBJ_DIR)/computedExpressionNativeBoundFunction.o: \
				src/computedExpressionNativeBoundFunction.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONFLOAT) \
				$(DEP_COMPUTEDEXPRESSIONNATIVEBOUNDFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONSTRING)

$(OBJ_DIR)/computedExpressionNativeFunction.o: \
				src/computedExpressionNativeFunction.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONFLOAT) \
				$(DEP_COMPUTEDEXPRESSIONNATIVEFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONSTRING) \
				$(DEP_CONTEXT)

$(OBJ_DIR)/computedExpressionNativeLibraryFunction.o: \
				src/computedExpressionNativeLibraryFunction.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONFLOAT) \
				$(DEP_COMPUTEDEXPRESSIONNATIVELIBRARYFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONSTRING) \
				$(DEP_CONTEXT)

$(OBJ_DIR)/computedExpressionString.o: \
				src/computedExpressionString.c \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONINTEGER) \
				$(DEP_COMPUTEDEXPRESSIONITERATOR) \
				$(DEP_COMPUTEDEXPRESSIONITERATOREND) \
				$(DEP_COMPUTEDEXPRESSIONSTRING)

$(OBJ_DIR)/executionContext.o: \
	src/executionContext.c \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/error.o: \
				src/error.c \
				$(DEP_ERROR)

$(OBJ_DIR)/htmlEscape.o: \
				$(GEN_DIR)/htmlEscape.c \
				$(DEP_COMPUTEDEXPRESSION) \
				include/tang/htmlEscape.h

$(OBJ_DIR)/htmlEscapeAscii.o: \
				$(GEN_DIR)/htmlEscapeAscii.c \
				$(DEP_COMPUTEDEXPRESSION) \
				include/tang/htmlEscapeAscii.h

$(OBJ_DIR)/opcode.o: \
				src/opcode.c \
				$(DEP_OPCODE)

$(OBJ_DIR)/percentEncode.o: \
				$(GEN_DIR)/percentEncode.c \
				$(DEP_COMPUTEDEXPRESSION) \
				include/tang/percentEncode.h

$(OBJ_DIR)/program-analyze.o: \
				src/program-analyze.c \
				$(DEP_PROGRAM) \
				$(DEP_OPCODE)

$(OBJ_DIR)/program-dumpBytecode.o: \
				src/program-dumpBytecode.c \
				$(DEP_PROGRAM) \
				$(DEP_OPCODE)

$(OBJ_DIR)/program-execute.o: \
				src/program-execute.c \
				$(DEP_COMPUTEDEXPRESSIONARRAY) \
				$(DEP_COMPUTEDEXPRESSIONBOOLEAN) \
				$(DEP_COMPUTEDEXPRESSIONCOMPILEDFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONFLOAT) \
				$(DEP_COMPUTEDEXPRESSIONINTEGER) \
				$(DEP_COMPUTEDEXPRESSIONITERATOREND) \
				$(DEP_COMPUTEDEXPRESSIONLIBRARY) \
				$(DEP_COMPUTEDEXPRESSIONMAP) \
				$(DEP_COMPUTEDEXPRESSIONNATIVEBOUNDFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONNATIVEFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONNATIVELIBRARYFUNCTION) \
				$(DEP_COMPUTEDEXPRESSIONSTRING) \
				$(DEP_CONTEXT) \
				$(DEP_OPCODE) \
				$(DEP_PROGRAM) \
				$(DEP_TANGBASE)

$(OBJ_DIR)/program-optimize.o: \
				src/program-optimize.c \
				$(DEP_PROGRAM) \
				$(DEP_OPCODE)

$(OBJ_DIR)/tangBase.o: \
				src/tangBase.c \
				$(DEP_COMPUTEDEXPRESSIONARRAY) \
				$(DEP_COMPUTEDEXPRESSIONCONTEXT) \
				$(DEP_COMPUTEDEXPRESSIONERROR) \
				$(DEP_COMPUTEDEXPRESSIONLIBRARYMATH) \
				$(DEP_COMPUTEDEXPRESSIONLIBRARYTANG) \
				$(DEP_COMPUTEDEXPRESSIONSTRING) \
				$(DEP_TANGBASE)

$(OBJ_DIR)/tangParser.o: \
				$(GEN_DIR)/tangParser.c \
				$(DEP_ASTNODE)
# $(DEP_ASTNODEARRAY) \
# $(DEP_ASTNODEASSIGN) \
# $(DEP_ASTNODEBINARY) \
# $(DEP_ASTNODEBLOCK) \
# $(DEP_ASTNODEBOOLEAN) \
# $(DEP_ASTNODEBREAK) \
# $(DEP_ASTNODECAST) \
# $(DEP_ASTNODECONTINUE) \
# $(DEP_ASTNODEDOWHILE) \
# $(DEP_ASTNODEFLOAT) \
# $(DEP_ASTNODEFOR) \
# $(DEP_ASTNODEFUNCTIONCALL) \
# $(DEP_ASTNODEFUNCTIONDECLARATION) \
# $(DEP_ASTNODEIDENTIFIER) \
# $(DEP_ASTNODEIFELSE) \
# $(DEP_ASTNODEINDEX) \
# $(DEP_ASTNODEINTEGER) \
# $(DEP_ASTNODELIBRARY) \
# $(DEP_ASTNODEMAP) \
# $(DEP_ASTNODEPERIOD) \
# $(DEP_ASTNODEPRINT) \
# $(DEP_ASTNODERANGEDFOR) \
# $(DEP_ASTNODERETURN) \
# $(DEP_ASTNODESLICE) \
# $(DEP_ASTNODESTRING) \
# $(DEP_ASTNODETERNARY) \
# $(DEP_ASTNODEUNARY) \
# $(DEP_ASTNODEUSE) \
# $(DEP_ASTNODEWHILE) \
# $(DEP_ERROR) \
# $(DEP_LOCATION) \
# $(DEP_UNICODESTRING)

$(OBJ_DIR)/tangScanner.o: \
				$(GEN_DIR)/tangScanner.c \
				$(DEP_UNICODESTRING) \
				include/tang/tangScanner.h \
				$(DEP_LOCATION)
	@echo "\n### Compiling $@ ###"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -MMD -o $@ -fPIC -Wno-unused-function


$(OBJ_DIR)/unescape.o: \
				$(GEN_DIR)/unescape.c \
				include/tang/unescape.h

$(OBJ_DIR)/unicodeString.o: \
				src/unicodeString.c \
				$(DEP_UNICODESTRING)

####################################################################
# Shared Library
####################################################################

$(APP_DIR)/$(TARGET): \
				$(LIBOBJECTS)
	@echo "\n### Compiling Tang Shared Library ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LDFLAGS) -Wl,-soname,$(SO_NAME)
	@ln -f -s $(TARGET) $(APP_DIR)/$(SO_NAME)
	@ln -f -s $(SO_NAME) $(APP_DIR)/$(BASE_NAME)

####################################################################
# Command Line Utility
####################################################################

$(APP_DIR)/tang: \
				src/tang.c \
				$(DEP_TANG) \
				$(APP_DIR)/$(TARGET)
	@echo "\n### Compiling Tang Command Line Utility ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TANGLIBRARY)

####################################################################
# Unit Tests
####################################################################

$(APP_DIR)/libtestLibrary.so: \
				test/libtestLibrary.cpp \
				$(APP_DIR)/$(TARGET) \
				include/tang/tang.h
	@echo "\n### Compiling Test Library ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -shared -o $@ $< $(LDFLAGS) -fPIC

$(APP_DIR)/testUnicodeString: \
				test/test-unicodeString.cpp \
				$(OBJ_DIR)/unicodeString.o
#				$(OBJ_DIR)/htmlEscape.o \
#				$(OBJ_DIR)/htmlEscapeAscii.o \
#				$(OBJ_DIR)/percentEncode.o \
#				$(OBJ_DIR)/unescape.o
	@echo "\n### Compiling UnicodeString Test ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^ $(LDFLAGS) $(TESTFLAGS)

$(APP_DIR)/testTangLanguageParse: \
	test/test-tangLanguageParse.cpp \
	$(DEP_ASTNODE_ALL)
	@echo "\n### Compiling Tang Language Parse Test ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testTangLanguageExecuteSimple: \
	test/test-tangLanguageExecuteSimple.cpp \
	$(DEP_ASTNODE_ALL) \
	$(DEP_COMPUTEDVALUE_ALL) \
	$(DEP_PROGRAM) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_BYTECODE)
	@echo "\n### Compiling Tang Language Execution Simple Test ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/test: \
				test/test.cpp \
				$(DEP_TANG) \
				$(APP_DIR)/$(TARGET)
	@echo "\n### Compiling Tang Test ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

####################################################################
# Commands
####################################################################

.PHONY: all clean cloc docs docs-pdf install test test-watch watch

watch: ## Watch the file directory for changes and compile the target
	@while true; do \
          make --no-print-directory $(GEN_DIR)/tangParser.h; \
					make --no-print-directory all; \
					echo "\033[0;32m"; \
					echo "#########################"; \
					echo "# Waiting for changes.. #"; \
					echo "#########################"; \
					echo "\033[0m"; \
					inotifywait -qr -e modify -e create -e delete -e move src include bison flex test Makefile --exclude '/\.'; \
					done

test-watch: ## Watch the file directory for changes and run the unit tests
	@while true; do \
					make --no-print-directory test; \
					echo "\033[0;32m"; \
					echo "#########################"; \
					echo "# Waiting for changes.. #"; \
					echo "#########################"; \
					echo "\033[0m"; \
					inotifywait -qr -e modify -e create -e delete -e move src include bison flex test Makefile --exclude '/\.'; \
					done

test: ## Make and run the Unit tests
test: \
				$(APP_DIR)/$(TARGET) \
				$(APP_DIR)/testUnicodeString \
				$(APP_DIR)/testTangLanguageParse \
				$(APP_DIR)/testTangLanguageExecuteSimple
#				$(APP_DIR)/libtestLibrary.so \
#				$(APP_DIR)/test \
#				$(APP_DIR)/tang
	@echo "\033[0;32m"
	@echo "############################"
	@echo "### Running string tests ###"
	@echo "############################"
	@echo "\033[0m"
	$(APP_DIR)/testUnicodeString --gtest_brief=1
	@echo "\033[0;32m"
	@echo "####################################"
	@echo "### Running Language Parse tests ###"
	@echo "####################################"
	@echo "\033[0m"
	env LD_LIBRARY_PATH="$(APP_DIR)" $(APP_DIR)/testTangLanguageParse --gtest_brief=1 --gtest_fail_fast
	@echo "\033[0;32m"
	@echo "########################################################"
	@echo "### Running Bytecode Language Execution Simple tests ###"
	@echo "########################################################"
	@echo "\033[0m"
	env LD_LIBRARY_PATH="$(APP_DIR)" env TANG_DISABLE_BINARY= $(APP_DIR)/testTangLanguageExecuteSimple --gtest_brief=1
	@echo "\033[0;32m"
	@echo "########################################################"
	@echo "### Running Binary Language Execution Simple tests   ###"
	@echo "########################################################"
	@echo "\033[0m"
	env LD_LIBRARY_PATH="$(APP_DIR)" env TANG_DISABLE_BYTECODE= $(APP_DIR)/testTangLanguageExecuteSimple --gtest_brief=1
#	@echo "\033[0;32m"
#	@echo "############################"
#	@echo "### Running normal tests ###"
#	@echo "############################"
#	@echo "\033[0m"
#	env LD_LIBRARY_PATH="$(APP_DIR)" $(APP_DIR)/test --gtest_brief=1

clean: ## Remove all contents of the build directories.
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
	-@rm -rvf $(GEN_DIR)/*

# Files will be as follows:
# /usr/local/lib/(SUITE)/
#   lib(SUITE)-(PROJECT)(BRANCH).so.(MAJOR).(MINOR)
#   lib(SUITE)-(PROJECT)(BRANCH).so.(MAJOR) link to previous
#   lib(SUITE)-(PROJECT)(BRANCH).so link to previous
# /etc/ld.so.conf.d/(SUITE)-(PROJECT)(BRANCH).conf will point to /usr/local/lib/(SUITE)
# /usr/local/include/(SUITE)/(PROJECT)(BRANCH)
#   *.h copied from ./include/(PROJECT)
# /usr/local/share/pkgconfig
#   (SUITE)-(PROJECT)(BRANCH).pc created

install: ## Install the library globally, requires sudo
	# Installing the shared library.
	@mkdir -p /usr/local/lib/$(SUITE)
	@cp $(APP_DIR)/$(TARGET) /usr/local/lib/$(SUITE)/
	@ln -f -s $(TARGET) /usr/local/lib/$(SUITE)/$(SO_NAME)
	@ln -f -s $(SO_NAME) /usr/local/lib/$(SUITE)/$(BASE_NAME)
	# Installing the ld configuration file.
	@echo "/usr/local/lib/$(SUITE)" > /etc/ld.so.conf.d/$(SUITE)-$(PROJECT)$(BRANCH).conf
	# Installing the headers.
	@mkdir -p /usr/local/include/$(SUITE)/$(PROJECT)$(BRANCH)/$(PROJECT)
	@cp include/tang/*.h /usr/local/include/$(SUITE)/$(PROJECT)$(BRANCH)/$(PROJECT)
	@cp build/generated/*.h /usr/local/include/$(SUITE)/$(PROJECT)$(BRANCH)/$(PROJECT)
	# Installing the pkg-config files.
	@mkdir -p /usr/local/share/pkgconfig
	@cat pkgconfig/$(SUITE)-$(PROJECT).pc | sed 's/(SUITE)/$(SUITE)/g; s/(PROJECT)/$(PROJECT)/g; s/(BRANCH)/$(BRANCH)/g; s/(VERSION)/$(VERSION)/g' > /usr/local/share/pkgconfig/$(SUITE)-$(PROJECT)$(BRANCH).pc
	# Running ldconfig.
	@ldconfig >> /dev/null 2>&1
	@echo "Ghoti.io $(PROJECT)$(BRANCH) installed"

uninstall: ## Delete the globally-installed files.  Requires sudo.
	# Deleting the shared library.
	@rm -f /usr/local/lib/$(SUITE)/$(BASE_NAME)*
	# Deleting the ld configuration file.
	@rm -f /etc/ld.so.conf.d/$(SUITE)-$(PROJECT)$(BRANCH).conf
	# Deleting the headers.
	@rm -rf /usr/local/include/$(SUITE)/$(PROJECT)$(BRANCH)
	# Deleting the pkg-config files.
	@rm -f /usr/local/share/pkgconfig/$(SUITE)-$(PROJECT)$(BRANCH).pc
	# Cleaning up (potentially) no longer needed directories.
	@rmdir --ignore-fail-on-non-empty /usr/local/include/$(SUITE)
	@rmdir --ignore-fail-on-non-empty /usr/local/lib/$(SUITE)
	# Running ldconfig.
	@ldconfig >> /dev/null 2>&1
	@echo "Ghoti.io $(PROJECT)$(BRANCH) has been uninstalled"

docs: ## Generate the documentation in the ./docs subdirectory
	doxygen

docs-pdf: docs ## Generate the documentation as a pdf, at ./docs/(SUITE)-(PROJECT)(BRANCH).pdf
	cd ./docs/latex/ && make
	mv -f ./docs/latex/refman.pdf ./docs/$(SUITE)-$(PROJECT)$(BRANCH)-docs.pdf

cloc: ## Count the lines of code used in the project
	cloc src include flex bison test Makefile

help: ## Display this help
	@grep -E '^[ a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "%-15s %s\n", $$1, $$2}' | sed "s/(SUITE)/$(SUITE)/g; s/(PROJECT)/$(PROJECT)/g; s/(BRANCH)/$(BRANCH)/g"

