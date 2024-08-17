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
	$(OBJ_DIR)/ast/astNode.o \
	$(OBJ_DIR)/ast/astNodeArray.o \
	$(OBJ_DIR)/ast/astNodeAssign.o \
	$(OBJ_DIR)/ast/astNodeBinary.o \
	$(OBJ_DIR)/ast/astNodeBlock.o \
	$(OBJ_DIR)/ast/astNodeBoolean.o \
	$(OBJ_DIR)/ast/astNodeBreak.o \
	$(OBJ_DIR)/ast/astNodeCast.o \
	$(OBJ_DIR)/ast/astNodeContinue.o \
	$(OBJ_DIR)/ast/astNodeDoWhile.o \
	$(OBJ_DIR)/ast/astNodeFloat.o \
	$(OBJ_DIR)/ast/astNodeFor.o \
	$(OBJ_DIR)/ast/astNodeFunction.o \
	$(OBJ_DIR)/ast/astNodeFunctionCall.o \
	$(OBJ_DIR)/ast/astNodeGlobal.o \
	$(OBJ_DIR)/ast/astNodeIdentifier.o \
	$(OBJ_DIR)/ast/astNodeIfElse.o \
	$(OBJ_DIR)/ast/astNodeIndex.o \
	$(OBJ_DIR)/ast/astNodeInteger.o \
	$(OBJ_DIR)/ast/astNodeLibrary.o \
	$(OBJ_DIR)/ast/astNodeMap.o \
	$(OBJ_DIR)/ast/astNodePeriod.o \
	$(OBJ_DIR)/ast/astNodeParseError.o \
	$(OBJ_DIR)/ast/astNodePrint.o \
	$(OBJ_DIR)/ast/astNodeRangedFor.o \
	$(OBJ_DIR)/ast/astNodeReturn.o \
	$(OBJ_DIR)/ast/astNodeSlice.o \
	$(OBJ_DIR)/ast/astNodeString.o \
	$(OBJ_DIR)/ast/astNodeTernary.o \
	$(OBJ_DIR)/ast/astNodeUnary.o \
	$(OBJ_DIR)/ast/astNodeUse.o \
	$(OBJ_DIR)/ast/astNodeWhile.o \
	$(OBJ_DIR)/computedValue/computedValue.o \
	$(OBJ_DIR)/computedValue/computedValueArray.o \
	$(OBJ_DIR)/computedValue/computedValueBoolean.o \
	$(OBJ_DIR)/computedValue/computedValueError.o \
	$(OBJ_DIR)/computedValue/computedValueFloat.o \
	$(OBJ_DIR)/computedValue/computedValueFunction.o \
	$(OBJ_DIR)/computedValue/computedValueFunctionNative.o \
	$(OBJ_DIR)/computedValue/computedValueInteger.o \
	$(OBJ_DIR)/computedValue/computedValueIterator.o \
	$(OBJ_DIR)/computedValue/computedValueMap.o \
	$(OBJ_DIR)/computedValue/computedValueString.o \
	$(OBJ_DIR)/program/binary.o \
	$(OBJ_DIR)/program/bytecode.o \
	$(OBJ_DIR)/program/compilerContext.o \
	$(OBJ_DIR)/program/executionContext.o \
	$(OBJ_DIR)/program/garbageCollector.o \
	$(OBJ_DIR)/program/program.o \
	$(OBJ_DIR)/program/variable.o \
	$(OBJ_DIR)/tangLanguage.o \
	$(OBJ_DIR)/program/virtualMachine.o \


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
	include/tang/program/bytecode.h \
	$(DEP_MACROS)
DEP_PROGRAM_BINARY = \
	include/tang/program/binary.h \
	$(DEP_MACROS)
DEP_PROGRAM_COMPILERCONTEXT = \
	include/tang/program/compilerContext.h \
	$(DEP_BYTECODE) \
	$(DEP_MACROS)

DEP_ASTNODE = \
	include/tang/ast/astNode.h \
	$(GEN_DIR)/tangParser.h \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_LOCATION) \
	$(DEP_MACROS)
DEP_ASTNODEIDENTIFIER = \
	include/tang/ast/astNodeIdentifier.h \
	$(DEP_ASTNODE)
DEP_ASTNODESTRING = \
	include/tang/ast/astNodeString.h \
	$(DEP_ASTNODE)
DEP_ASTNODEARRAY = \
	include/tang/ast/astNodeArray.h \
	$(DEP_ASTNODE)
DEP_ASTNODEASSIGN = \
	include/tang/ast/astNodeAssign.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEINDEX = \
	include/tang/ast/astNodeIndex.h \
	$(DEP_ASTNODE)
DEP_ASTNODEBINARY = \
	include/tang/ast/astNodeBinary.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEBLOCK = \
	include/tang/ast/astNodeBlock.h \
	$(DEP_ASTNODE)
DEP_ASTNODEBOOLEAN = \
	include/tang/ast/astNodeBoolean.h \
	$(DEP_ASTNODE)
DEP_ASTNODEBREAK = \
	include/tang/ast/astNodeBreak.h \
	$(DEP_ASTNODE)
DEP_ASTNODECAST = \
	include/tang/ast/astNodeCast.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODECONTINUE = \
	include/tang/ast/astNodeContinue.h \
	$(DEP_ASTNODE)
DEP_ASTNODEDOWHILE = \
	include/tang/ast/astNodeDoWhile.h \
	$(DEP_ASTNODE)
DEP_ASTNODEFLOAT = \
	include/tang/ast/astNodeFloat.h \
	$(DEP_ASTNODE)
DEP_ASTNODEFOR = \
	include/tang/ast/astNodeFor.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEFUNCTION = \
	include/tang/ast/astNodeFunction.h \
	$(DEP_ASTNODE)
DEP_ASTNODEFUNCTIONCALL = \
	include/tang/ast/astNodeFunctionCall.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEGLOBAL = \
	include/tang/ast/astNodeGlobal.h \
	$(DEP_ASTNODE)
DEP_ASTNODEIFELSE = \
	include/tang/ast/astNodeIfElse.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEINDEX = \
	include/tang/ast/astNodeIndex.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEINTEGER = \
	include/tang/ast/astNodeInteger.h \
	$(DEP_ASTNODE)
DEP_ASTNODELIBRARY = \
	include/tang/ast/astNodeLibrary.h \
	$(DEP_ASTNODE)
DEP_ASTNODEMAP = \
	include/tang/ast/astNodeMap.h \
	$(DEP_ASTNODE)
DEP_ASTNODEPARSEERROR = \
	include/tang/ast/astNodeParseError.h \
	$(DEP_ASTNODE)
DEP_ASTNODEPERIOD = \
	include/tang/ast/astNodePeriod.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEPRINT = \
	include/tang/ast/astNodePrint.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODERANGEDFOR = \
	include/tang/ast/astNodeFor.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODERETURN = \
	include/tang/ast/astNodeReturn.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODESLICE = \
	include/tang/ast/astNodeSlice.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODETERNARY = \
	include/tang/ast/astNodeTernary.h \
	$(DEP_ASTNODE)
DEP_ASTNODEUNARY = \
	include/tang/ast/astNodeUnary.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEIDENTIFIER)
DEP_ASTNODEUSE = \
	include/tang/ast/astNodeUse.h \
	$(DEP_ASTNODE)
DEP_ASTNODEWHILE = \
	include/tang/ast/astNodeWhile.h \
	$(DEP_ASTNODE) \
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
	$(DEP_ASTNODEGLOBAL) \
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
	include/tang/computedValue/computedValue.h \
	$(DEP_MACROS)
DEP_COMPUTEDVALUEARRAY = \
	include/tang/computedValue/computedValueArray.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEBOOLEAN = \
	include/tang/computedValue/computedValueBoolean.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEERROR = \
  include/tang/computedValue/computedValueError.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEFLOAT = \
	include/tang/computedValue/computedValueFloat.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEFUNCTION = \
    include/tang/computedValue/computedValueFunction.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEFUNCTIONNATIVE = \
	include/tang/computedValue/computedValueFunctionNative.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEINTEGER = \
	include/tang/computedValue/computedValueInteger.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEITERATOR = \
	include/tang/computedValue/computedValueIterator.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUEMAP = \
	include/tang/computedValue/computedValueMap.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUESTRING = \
	include/tang/computedValue/computedValueString.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_UNICODESTRING)

DEP_COMPUTEDVALUEALL = \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEARRAY) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_COMPUTEDVALUEFLOAT) \
	$(DEP_COMPUTEDVALUEFUNCTION) \
	$(DEP_COMPUTEDVALUEFUNCTIONNATIVE) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUEITERATOR) \
	$(DEP_COMPUTEDVALUEMAP) \
	$(DEP_COMPUTEDVALUESTRING)

DEP_TANGLANGUAGE = \
	$(DEP_ASTNODE)

DEP_EXECUTIONCONTEXT = \
	include/tang/program/executionContext.h \
	$(DEP_MACROS) \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_UNICODESTRING)

DEP_GARBAGECOLLECTOR = \
	include/tang/program/garbageCollector.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_MACROS)

DEP_PROGRAM_VARIABLE = \
	include/tang/program/variable.h \
	$(DEP_MACROS) \
	$(DEP_ASTNODE)

DEP_PROGRAM = \
	include/tang/program/program.h \
	$(DEP_ASTNODE) \
	$(DEP_BYTECODE) \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_UNICODESTRING)

DEP_VIRTUALMACHINE = \
	include/tang/program/virtualMachine.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_PROGRAM)


####################################################################
# Bison-Generated Files
####################################################################
$(GEN_DIR)/tangParser.h: \
				bison/tangParser.y \
				include/tang/ast/astNode.h \
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

$(OBJ_DIR)/ast/astNode.o: \
	src/ast/astNode.c \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_BINARY) \
	$(DEP_BYTECODE) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_PROGRAM) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeArray.o: \
	src/ast/astNodeArray.c \
	$(DEP_ASTNODEARRAY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUEARRAY) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeAssign.o: \
	src/ast/astNodeAssign.c \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_ASTNODEINDEX) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_VARIABLE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeBinary.o: \
	src/ast/astNodeBinary.c \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_ASTNODESTRING) \
	$(DEP_UNICODESTRING) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeBlock.o: \
	src/ast/astNodeBlock.c \
	$(DEP_ASTNODEBLOCK) \
	$(DEP_ASTNODEFUNCTION) \
	$(DEP_ASTNODEGLOBAL) \
	$(DEP_ASTNODEUSE) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeBoolean.o: \
	src/ast/astNodeBoolean.c \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeBreak.o: \
	src/ast/astNodeBreak.c \
	$(DEP_ASTNODEBREAK) \
	$(DEP_BINARY) \
	$(DEP_COMPUTED_VALUE) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeCast.o: \
	src/ast/astNodeCast.c \
	$(DEP_ASTNODECAST) \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_ASTNODESTRING) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeContinue.o: \
	src/ast/astNodeContinue.c \
	$(DEP_ASTNODECONTINUE) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeDoWhile.o: \
	src/ast/astNodeDoWhile.c \
	$(DEP_ASTNODEDOWHILE) \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeFor.o: \
	src/ast/astNodeFor.c \
	$(DEP_ASTNODEFOR) \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeFloat.o: \
	src/ast/astNodeFloat.c \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUEFLOAT) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeFunction.o: \
	src/ast/astNodeFunction.c \
	$(DEP_ASTNODEFUNCTIONDECLARATION) \
	$(DEP_ASTNODEPARSEERROR) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_ASTNODESTRING) \
	$(DEP_COMPUTEDVALUEFUNCTION) \
	$(DEP_MACROS) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_PROGRAM_VARIABLE) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeFunctionCall.o: \
	src/ast/astNodeFunctionCall.c \
	$(DEP_ASTNODEFUNCTIONCALL) \
	$(DEP_BINARY) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_COMPUTEDVALUEFUNCTION) \
	$(DEP_COMPUTEDVALUEFUNCTIONNATIVE) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeGlobal.o: \
	src/ast/astNodeGlobal.c \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEGLOBAL) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeIdentifier.o: \
	src/ast/astNodeIdentifier.c \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_ASTNODEFUNCTION) \
	$(DEP_ASTNODEGLOBAL) \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_ASTNODEPARSEERROR) \
	$(DEP_ASTNODESTRING) \
	$(DEP_ASTNODEUSE) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_VARIABLE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeIfElse.o: \
	src/ast/astNodeIfElse.c \
	$(DEP_ASTNODEIFELSE) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeIndex.o: \
	src/ast/astNodeIndex.c \
	$(DEP_ASTNODEINDEX) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeInteger.o: \
	src/ast/astNodeInteger.c \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeLibrary.o: \
	src/ast/astNodeLibrary.c \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_ASTNODELIBRARY) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeMap.o: \
	src/ast/astNodeMap.c \
	$(DEP_ASTNODEMAP) \
	$(DEP_ASTNODESTRING) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUEMAP) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY) \
	$(DEP_UNICODESTRING)

$(OBJ_DIR)/ast/astNodeParseError.o: \
	src/ast/astNodeParseError.c \
	$(DEP_ASTNODEPARSEERROR)

$(OBJ_DIR)/ast/astNodePeriod.o: \
	src/ast/astNodePeriod.c \
	$(DEP_ASTNODEPERIOD) \
	$(DEP_ASTNODESTRING) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodePrint.o: \
	src/ast/astNodePrint.c \
	$(DEP_ASTNODEPRINT) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeRangedFor.o: \
	src/ast/astNodeRangedFor.c \
	$(DEP_ASTNODERANGEDFOR) \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARY) \
	$(DEP_COMPUTEDVALUEITERATOR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_PROGRAM_VARIABLE)

$(OBJ_DIR)/ast/astNodeReturn.o: \
	src/ast/astNodeReturn.c \
	$(DEP_ASTNODERETURN) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeSlice.o: \
	src/ast/astNodeSlice.c \
	$(DEP_ASTNODESLICE) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeString.o: \
	src/ast/astNodeString.c \
	$(DEP_ASTNODESTRING) \
	$(DEP_UNICODESTRING) \
	$(DEP_COMPUTEDVALUESTRING) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeTernary.o: \
	src/ast/astNodeTernary.c \
	$(DEP_ASTNODETERNARY) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeUnary.o: \
	src/ast/astNodeUnary.c \
	$(DEP_ASTNODEUNARY) \
	$(DEP_ASTNODEINTEGER) \
	$(DEP_ASTNODEFLOAT) \
	$(DEP_ASTNODEBOOLEAN) \
	$(DEP_ASTNODESTRING) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeUse.o: \
	src/ast/astNodeUse.c \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_ASTNODEPARSEERROR) \
	$(DEP_ASTNODEUSE) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_VARIABLE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeWhile.o: \
	src/ast/astNodeWhile.c \
	$(DEP_ASTNODEWHILE) \
	$(DEP_ASTNODEASSIGN) \
	$(DEP_ASTNODEBINARY) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/tangLanguage.o: \
	src/tangLanguage.c \
	$(DEP_TANGLANGUAGE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODEIDENTIFIER) \
	$(DEP_ASTNODEPARSEERROR)

$(OBJ_DIR)/computedValue/computedValue.o: \
	src/computedValue/computedValue.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_COMPUTEDVALUEFLOAT) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUESTRING) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_MACROS)

$(OBJ_DIR)/computedValue/computedValueArray.o: \
	src/computedValue/computedValueArray.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEARRAY) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_MACROS)

$(OBJ_DIR)/computedValue/computedValueBoolean.o: \
	src/computedValue/computedValueBoolean.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_COMPUTEDVALUEFLOAT) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUESTRING) \
	$(DEP_COMPUTEDVALUEERROR)

$(OBJ_DIR)/computedValue/computedValueError.o: \
	src/computedValue/computedValueError.c \
	$(DEP_COMPUTEDVALUEERROR)

$(OBJ_DIR)/computedValue/computedValueFloat.o: \
	src/computedValue/computedValueFloat.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_COMPUTEDVALUEFLOAT) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUESTRING) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueFunction.o: \
	src/computedValue/computedValueFunction.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueFunctionNative.o: \
	src/computedValue/computedValueFunctionNative.c \
	$(DEP_COMPUTEDVALUEFUNCTIONNATIVE) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueInteger.o: \
	src/computedValue/computedValueInteger.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_COMPUTEDVALUEFLOAT) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUESTRING) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueIterator.o: \
	src/computedValue/computedValueIterator.c \
	$(DEP_COMPUTEDVALUEITERATOR) \
	$(DEP_COMPUTEDVALUEITERATOREND) \
	$(DEP_COMPUTEDVALUEERROR)

$(OBJ_DIR)/computedValue/computedValueMap.o: \
	src/computedValue/computedValueMap.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_COMPUTEDVALUEMAP) \
	$(DEP_COMPUTEDVALUESTRING) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_UNICODESTRING)

$(OBJ_DIR)/computedValue/computedValueString.o: \
	src/computedValue/computedValueString.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEBOOLEAN) \
	$(DEP_COMPUTEDVALUEFLOAT) \
	$(DEP_COMPUTEDVALUEINTEGER) \
	$(DEP_COMPUTEDVALUESTRING) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_UNICODESTRING)

$(OBJ_DIR)/program/binary.o: \
	src/program/binary.c \
	$(DEP_PROGRAM_BINARY) \
	$(DEP_MACROS)

$(OBJ_DIR)/program/bytecode.o: \
	src/program/bytecode.c \
	$(DEP_BYTECODE)

$(OBJ_DIR)/program/compilerContext.o: \
	src/program/compilerContext.c \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_PROGRAM) \
	$(DEP_PROGRAM_VARIABLE)

$(OBJ_DIR)/program/executionContext.o: \
	src/program/executionContext.c \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/program/garbageCollector.o: \
	src/program/garbageCollector.c \
	$(DEP_GARBAGECOLLECTOR)

$(OBJ_DIR)/program/program.o: \
	src/program/program.c \
	$(DEP_PROGRAM) \
	$(DEP_COMPUTEDVALUEERROR) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_TANGLANGUAGE) \
	$(DEP_ASTNODEALL) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUEFUNCTION) \
	$(DEP_PROGRAM_BINARY) \
	$(DEP_PROGRAM_VARIABLE) \
	$(DEP_VIRTUALMACHINE)

$(OBJ_DIR)/program/variable.o: \
	src/program/variable.c \
	$(DEP_PROGRAM_VARIABLE)

$(OBJ_DIR)/program/virtualMachine.o: \
	src/program/virtualMachine.c \
	$(DEP_VIRTUALMACHINE) \
	$(DEP_BYTECODE) \
	$(DEP_COMPUTEDVALUEALL)

$(OBJ_DIR)/tangParser.o: \
				$(GEN_DIR)/tangParser.c \
				$(DEP_ASTNODE)

$(OBJ_DIR)/tangScanner.o: \
				$(GEN_DIR)/tangScanner.c \
				$(DEP_UNICODESTRING) \
				include/tang/tangScanner.h \
				$(DEP_LOCATION)
	@echo "\n### Compiling $@ ###"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -MMD -o $@ -fPIC -Wno-unused-function

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

$(APP_DIR)/testTangLanguageExecuteComplex: \
	test/test-tangLanguageExecuteComplex.cpp \
	$(DEP_ASTNODE_ALL) \
	$(DEP_COMPUTEDVALUE_ALL) \
	$(DEP_PROGRAM) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_BYTECODE)
	@echo "\n### Compiling Tang Language Execution Complex Test ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testBinary: \
	test/test-binary.cpp \
	$(DEP_BINARY)
	@echo "\n### Compiling Binary JIT functions Test ###"
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
				$(APP_DIR)/testTangLanguageExecuteSimple \
				$(APP_DIR)/testTangLanguageExecuteComplex \
				$(APP_DIR)/testBinary
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
	@echo "################################"
	@echo "### Running Binary JIT tests ###"
	@echo "################################"
	@echo "\033[0m"
	env LD_LIBRARY_PATH="$(APP_DIR)" env TANG_DISABLE_BINARY= $(APP_DIR)/testBinary --gtest_brief=1
	@echo "\033[0;32m"
	@echo "########################################################"
	@echo "### Running Bytecode Language Execution Simple tests ###"
	@echo "########################################################"
	@echo "\033[0m"
	env LD_LIBRARY_PATH="$(APP_DIR)" env TANG_DISABLE_BINARY= $(APP_DIR)/testTangLanguageExecuteSimple --gtest_brief=1
	@echo "\033[0;32m"
	@echo "#########################################################"
	@echo "### Running Bytecode Language Execution Complex tests ###"
	@echo "#########################################################"
	@echo "\033[0m"
	env LD_LIBRARY_PATH="$(APP_DIR)" env TANG_DISABLE_BINARY= $(APP_DIR)/testTangLanguageExecuteComplex --gtest_brief=1
	@echo "\033[0;32m"
	@echo "########################################################"
	@echo "### Running Binary Language Execution Simple tests   ###"
	@echo "########################################################"
	@echo "\033[0m"
	env LD_LIBRARY_PATH="$(APP_DIR)" env TANG_DISABLE_BYTECODE= $(APP_DIR)/testTangLanguageExecuteSimple --gtest_brief=1
	@echo "\033[0;32m"
	@echo "########################################################"
	@echo "### Running Binary Language Execution Complex tests  ###"
	@echo "########################################################"
	@echo "\033[0m"
	env LD_LIBRARY_PATH="$(APP_DIR)" env TANG_DISABLE_BYTECODE= $(APP_DIR)/testTangLanguageExecuteComplex --gtest_brief=1
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

