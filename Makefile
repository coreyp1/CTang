SUITE := ghoti.io
PROJECT := tang
BRANCH := -cdev
BASE_NAME := lib$(SUITE)-$(PROJECT)$(BRANCH).so
BASE_NAME_PREFIX := lib$(SUITE)-$(PROJECT)$(BRANCH)
MAJOR_VERSION := 0
MINOR_VERSION := 0.0
SO_NAME := $(BASE_NAME).$(MAJOR_VERSION)

# Detect OS
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
	OS_NAME := Linux
	LIB_EXTENSION := so
	OS_SPECIFIC_CXX_FLAGS := -shared -fPIC
	OS_SPECIFIC_LIBRARY_NAME_FLAG := -Wl,-soname,$(SO_NAME)
	TARGET := $(SO_NAME).$(MINOR_VERSION)
	EXE_EXTENSION :=
	# Additional Linux-specific variables
	PKG_CONFIG_PATH := /usr/local/share/pkgconfig
	INCLUDE_INSTALL_PATH := /usr/local/include
	LIB_INSTALL_PATH := /usr/local/lib

else ifeq ($(UNAME_S), Darwin)
	OS_NAME := Mac
	LIB_EXTENSION := dylib
	OS_SPECIFIC_CXX_FLAGS := -shared
	OS_SPECIFIC_LIBRARY_NAME_FLAG := -Wl,-install_name,$(BASE_NAME_PREFIX).dylib
	TARGET := $(BASE_NAME_PREFIX).dylib
	EXE_EXTENSION :=
	# Additional macOS-specific variables

else ifeq ($(findstring MINGW32_NT,$(UNAME_S)),MINGW32_NT)  # 32-bit Windows
	OS_NAME := Windows
	LIB_EXTENSION := dll
	OS_SPECIFIC_CXX_FLAGS := -shared
	OS_SPECIFIC_LIBRARY_NAME_FLAG := -Wl,--out-implib,$(APP_DIR)/$(BASE_NAME_PREFIX).dll.a
	TARGET := $(BASE_NAME_PREFIX).dll
	EXE_EXTENSION := .exe
	# Additional Windows-specific variables
	# This is the path to the pkg-config files on MSYS2
	PKG_CONFIG_PATH := /mingw32/lib/pkgconfig
	INCLUDE_INSTALL_PATH := /mingw32/include
	LIB_INSTALL_PATH := /mingw32/lib
	BIN_INSTALL_PATH := /mingw32/bin

else ifeq ($(findstring MINGW64_NT,$(UNAME_S)),MINGW64_NT)  # 64-bit Windows
	OS_NAME := Windows
	LIB_EXTENSION := dll
	OS_SPECIFIC_CXX_FLAGS := -shared
	OS_SPECIFIC_LIBRARY_NAME_FLAG := -Wl,--out-implib,$(APP_DIR)/$(BASE_NAME_PREFIX).dll.a
	TARGET := $(BASE_NAME_PREFIX).dll
	EXE_EXTENSION := .exe
	# Additional Windows-specific variables
	# This is the path to the pkg-config files on MSYS2
	PKG_CONFIG_PATH := /mingw64/lib/pkgconfig
	INCLUDE_INSTALL_PATH := /mingw64/include
	LIB_INSTALL_PATH := /mingw64/lib
	BIN_INSTALL_PATH := /mingw64/bin

else
    $(error Unsupported OS: $(UNAME_S))

endif


CXX := g++
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -Wno-error=unused-function -Wfatal-errors -std=c++20 -O1 -g
CC := cc
CFLAGS := -pedantic-errors -Wall -Wextra -Werror -Wno-error=unused-function -Wfatal-errors -std=c17 -O0 -g `PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags icu-io icu-i18n icu-uc ghoti.io-cutil-dev`
# -DGHOTIIO_CUTIL_ENABLE_MEMORY_DEBUG
LDFLAGS := -L /usr/lib -lstdc++ -lm `PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --cflags icu-io icu-i18n icu-uc ghoti.io-cutil-dev`
BUILD := ./build
OBJ_DIR := $(BUILD)/objects
GEN_DIR := $(BUILD)/generated
APP_DIR := $(BUILD)/apps


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
	$(OBJ_DIR)/computedValue/computedValueLibrary.o \
	$(OBJ_DIR)/computedValue/computedValueMap.o \
	$(OBJ_DIR)/computedValue/computedValueRNG.o \
	$(OBJ_DIR)/computedValue/computedValueString.o \
	$(OBJ_DIR)/library/library.o \
	$(OBJ_DIR)/library/libraryMath.o \
	$(OBJ_DIR)/library/libraryRandom.o \
	$(OBJ_DIR)/program/binary.o \
	$(OBJ_DIR)/program/bytecode.o \
	$(OBJ_DIR)/program/compilerContext.o \
	$(OBJ_DIR)/program/executionContext.o \
	$(OBJ_DIR)/program/garbageCollector.o \
	$(OBJ_DIR)/program/language.o \
	$(OBJ_DIR)/program/program.o \
	$(OBJ_DIR)/program/variable.o \
	$(OBJ_DIR)/tangLanguage.o \
	$(OBJ_DIR)/program/virtualMachine.o \


TESTFLAGS := `PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --cflags gtest`


TANGLIBRARY := -L $(APP_DIR) -l$(SUITE)-$(PROJECT)$(BRANCH)


all: $(APP_DIR)/$(TARGET) $(APP_DIR)/tang$(EXE_EXTENSION) ## Build the shared library

####################################################################
# Dependency Variables
####################################################################
DEP_LIBVER = \
	include/tang/libver.h
DEP_MACROS = \
	include/tang/macros.h \
	$(DEP_LIBVER)
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
DEP_ASTNODE_IDENTIFIER = \
	include/tang/ast/astNodeIdentifier.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_STRING = \
	include/tang/ast/astNodeString.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_ARRAY = \
	include/tang/ast/astNodeArray.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_ASSIGN = \
	include/tang/ast/astNodeAssign.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_INDEX = \
	include/tang/ast/astNodeIndex.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_BINARY = \
	include/tang/ast/astNodeBinary.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_BLOCK = \
	include/tang/ast/astNodeBlock.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_BOOLEAN = \
	include/tang/ast/astNodeBoolean.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_BREAK = \
	include/tang/ast/astNodeBreak.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_CAST = \
	include/tang/ast/astNodeCast.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_CONTINUE = \
	include/tang/ast/astNodeContinue.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_DOWHILE = \
	include/tang/ast/astNodeDoWhile.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_FLOAT = \
	include/tang/ast/astNodeFloat.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_FOR = \
	include/tang/ast/astNodeFor.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_FUNCTION = \
	include/tang/ast/astNodeFunction.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_FUNCTIONCALL = \
	include/tang/ast/astNodeFunctionCall.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_GLOBAL = \
	include/tang/ast/astNodeGlobal.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_IFELSE = \
	include/tang/ast/astNodeIfElse.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_INDEX = \
	include/tang/ast/astNodeIndex.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_INTEGER = \
	include/tang/ast/astNodeInteger.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_LIBRARY = \
	include/tang/ast/astNodeLibrary.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_MAP = \
	include/tang/ast/astNodeMap.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_PARSEERROR = \
	include/tang/ast/astNodeParseError.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_PERIOD = \
	include/tang/ast/astNodePeriod.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_PRINT = \
	include/tang/ast/astNodePrint.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_RANGEDFOR = \
	include/tang/ast/astNodeFor.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_RETURN = \
	include/tang/ast/astNodeReturn.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_SLICE = \
	include/tang/ast/astNodeSlice.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_TERNARY = \
	include/tang/ast/astNodeTernary.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_UNARY = \
	include/tang/ast/astNodeUnary.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)
DEP_ASTNODE_USE = \
	include/tang/ast/astNodeUse.h \
	$(DEP_ASTNODE)
DEP_ASTNODE_WHILE = \
	include/tang/ast/astNodeWhile.h \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_IDENTIFIER)

DEP_ASTNODE_ALL = \
	$(DEP_ASTNODE) \
	$(DEP_ASTNODE_ARRAY) \
	$(DEP_ASTNODE_ASSIGN) \
	$(DEP_ASTNODE_BINARY) \
	$(DEP_ASTNODE_BLOCK) \
	$(DEP_ASTNODE_BOOLEAN) \
	$(DEP_ASTNODE_BREAK) \
	$(DEP_ASTNODE_CAST) \
	$(DEP_ASTNODE_CONTINUE) \
	$(DEP_ASTNODE_DOWHILE) \
	$(DEP_ASTNODE_FLOAT) \
	$(DEP_ASTNODE_FOR) \
	$(DEP_ASTNODE_FUNCTION) \
	$(DEP_ASTNODE_FUNCTIONCALL) \
	$(DEP_ASTNODE_GLOBAL) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_ASTNODE_IFELSE) \
	$(DEP_ASTNODE_INDEX) \
	$(DEP_ASTNODE_INTEGER) \
	$(DEP_ASTNODE_LIBRARY) \
	$(DEP_ASTNODE_MAP) \
	$(DEP_ASTNODE_PARSEERROR) \
	$(DEP_ASTNODE_PERIOD) \
	$(DEP_ASTNODE_PRINT) \
	$(DEP_ASTNODE_RANGEDFOR) \
	$(DEP_ASTNODE_RETURN) \
	$(DEP_ASTNODE_SLICE) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_TERNARY) \
	$(DEP_ASTNODE_UNARY) \
	$(DEP_ASTNODE_USE) \
	$(DEP_ASTNODE_WHILE)

DEP_COMPUTEDVALUE = \
	include/tang/computedValue/computedValue.h \
	$(DEP_MACROS)
DEP_COMPUTEDVALUE_ARRAY = \
	include/tang/computedValue/computedValueArray.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUE_BOOLEAN = \
	include/tang/computedValue/computedValueBoolean.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUE_ERROR = \
  include/tang/computedValue/computedValueError.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUE_FLOAT = \
	include/tang/computedValue/computedValueFloat.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUE_FUNCTION = \
    include/tang/computedValue/computedValueFunction.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUE_FUNCTIONNATIVE = \
	include/tang/computedValue/computedValueFunctionNative.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUE_INTEGER = \
	include/tang/computedValue/computedValueInteger.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUE_ITERATOR = \
	include/tang/computedValue/computedValueIterator.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUE_LIBRARY = \
	include/tang/computedValue/computedValueLibrary.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_LIBRARY)
DEP_COMPUTEDVALUE_MAP = \
	include/tang/computedValue/computedValueMap.h \
	$(DEP_COMPUTEDVALUE)
DEP_COMPUTEDVALUE_RNG = \
	include/tang/computedValue/computedValueRNG.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_FLOAT)
DEP_COMPUTEDVALUE_STRING = \
	include/tang/computedValue/computedValueString.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_UNICODESTRING)

DEP_COMPUTEDVALUE_ALL = \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_ARRAY) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_COMPUTEDVALUE_FLOAT) \
	$(DEP_COMPUTEDVALUE_FUNCTION) \
	$(DEP_COMPUTEDVALUE_FUNCTIONNATIVE) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_ITERATOR) \
	$(DEP_COMPUTEDVALUE_LIBRARY) \
	$(DEP_COMPUTEDVALUE_MAP) \
	$(DEP_COMPUTEDVALUE_RNG) \
	$(DEP_COMPUTEDVALUE_STRING)

DEP_LIBRARY = \
	include/tang/library/library.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_MACROS)

DEP_LIBRARY_MATH = \
	include/tang/library/libraryMath.h \
	$(DEP_LIBRARY)
DEP_LIBRARY_RANDOM = \
	include/tang/library/libraryRandom.h \
	$(DEP_LIBRARY)

DEP_LIBRARYALL = \
	$(DEP_LIBRARY) \
	$(DEP_LIBRARY_MATH) \
	$(DEP_LIBRARY_RANDOM)

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

DEP_PROGRAM_LANGUAGE = \
	include/tang/program/language.h \
	$(DEP_LIBRARY) \
	$(DEP_MACROS)

DEP_PROGRAM = \
	include/tang/program/program.h \
	$(DEP_ASTNODE) \
	$(DEP_BYTECODE) \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_LIBRARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_PROGRAM_LANGUAGE) \
	$(DEP_UNICODESTRING)

DEP_VIRTUALMACHINE = \
	include/tang/program/virtualMachine.h \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_PROGRAM)

DEP_TANG = \
	include/tang/tang.h \
	$(DEP_COMPUTEDVALUE_ALL) \
	$(DEP_LIBRARYALL) \
	$(DEP_MACROS) \
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
	@printf "\n### Compiling $@ ###\n"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -MMD -o $@ $(OS_SPECIFIC_CXX_FLAGS)

$(OBJ_DIR)/ast/astNode.o: \
	src/ast/astNode.c \
	$(DEP_ASTNODE) \
	$(DEP_MACROS) \
	$(DEP_BYTECODE) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_PROGRAM) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeArray.o: \
	src/ast/astNodeArray.c \
	$(DEP_ASTNODE_ARRAY) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUE_ARRAY) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeAssign.o: \
	src/ast/astNodeAssign.c \
	$(DEP_ASTNODE_ASSIGN) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_ASTNODE_INDEX) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_VARIABLE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeBinary.o: \
	src/ast/astNodeBinary.c \
	$(DEP_ASTNODE_BINARY) \
	$(DEP_ASTNODE_INTEGER) \
	$(DEP_ASTNODE_FLOAT) \
	$(DEP_ASTNODE_BOOLEAN) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_UNICODESTRING) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeBlock.o: \
	src/ast/astNodeBlock.c \
	$(DEP_ASTNODE_BLOCK) \
	$(DEP_ASTNODE_FUNCTION) \
	$(DEP_ASTNODE_GLOBAL) \
	$(DEP_ASTNODE_USE) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeBoolean.o: \
	src/ast/astNodeBoolean.c \
	$(DEP_ASTNODE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeBreak.o: \
	src/ast/astNodeBreak.c \
	$(DEP_ASTNODE_BREAK) \
	$(DEP_COMPUTED_VALUE) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeCast.o: \
	src/ast/astNodeCast.c \
	$(DEP_ASTNODE_CAST) \
	$(DEP_ASTNODE_INTEGER) \
	$(DEP_ASTNODE_FLOAT) \
	$(DEP_ASTNODE_BOOLEAN) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeContinue.o: \
	src/ast/astNodeContinue.c \
	$(DEP_ASTNODE_CONTINUE) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeDoWhile.o: \
	src/ast/astNodeDoWhile.c \
	$(DEP_ASTNODE_DOWHILE) \
	$(DEP_ASTNODE_ASSIGN) \
	$(DEP_ASTNODE_BINARY) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeFor.o: \
	src/ast/astNodeFor.c \
	$(DEP_ASTNODE_FOR) \
	$(DEP_ASTNODE_ASSIGN) \
	$(DEP_ASTNODE_BINARY) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeFloat.o: \
	src/ast/astNodeFloat.c \
	$(DEP_ASTNODE_FLOAT) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUE_FLOAT) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeFunction.o: \
	src/ast/astNodeFunction.c \
	$(DEP_ASTNODE_FUNCTIONDECLARATION) \
	$(DEP_ASTNODE_PARSEERROR) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_COMPUTEDVALUE_FUNCTION) \
	$(DEP_MACROS) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_PROGRAM_VARIABLE) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeFunctionCall.o: \
	src/ast/astNodeFunctionCall.c \
	$(DEP_ASTNODE_FUNCTIONCALL) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_COMPUTEDVALUE_FUNCTION) \
	$(DEP_COMPUTEDVALUE_FUNCTIONNATIVE) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeGlobal.o: \
	src/ast/astNodeGlobal.c \
	$(DEP_ASTNODE_ASSIGN) \
	$(DEP_ASTNODE_GLOBAL) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeIdentifier.o: \
	src/ast/astNodeIdentifier.c \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_ASTNODE_INTEGER) \
	$(DEP_ASTNODE_FLOAT) \
	$(DEP_ASTNODE_FUNCTION) \
	$(DEP_ASTNODE_GLOBAL) \
	$(DEP_ASTNODE_BOOLEAN) \
	$(DEP_ASTNODE_PARSEERROR) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_ASTNODE_USE) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_VARIABLE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeIfElse.o: \
	src/ast/astNodeIfElse.c \
	$(DEP_ASTNODE_IFELSE) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeIndex.o: \
	src/ast/astNodeIndex.c \
	$(DEP_ASTNODE_INDEX) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeInteger.o: \
	src/ast/astNodeInteger.c \
	$(DEP_ASTNODE_INTEGER) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeLibrary.o: \
	src/ast/astNodeLibrary.c \
	$(DEP_ASTNODE_LIBRARY) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeMap.o: \
	src/ast/astNodeMap.c \
	$(DEP_ASTNODE_MAP) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUE_MAP) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY) \
	$(DEP_UNICODESTRING)

$(OBJ_DIR)/ast/astNodeParseError.o: \
	src/ast/astNodeParseError.c \
	$(DEP_ASTNODE_PARSEERROR)

$(OBJ_DIR)/ast/astNodePeriod.o: \
	src/ast/astNodePeriod.c \
	$(DEP_ASTNODE_PERIOD) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodePrint.o: \
	src/ast/astNodePrint.c \
	$(DEP_ASTNODE_PRINT) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeRangedFor.o: \
	src/ast/astNodeRangedFor.c \
	$(DEP_ASTNODE_RANGEDFOR) \
	$(DEP_ASTNODE_ASSIGN) \
	$(DEP_ASTNODE_BINARY) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_COMPUTEDVALUE_ITERATOR) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_PROGRAM_VARIABLE)

$(OBJ_DIR)/ast/astNodeReturn.o: \
	src/ast/astNodeReturn.c \
	$(DEP_ASTNODE_RETURN) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeSlice.o: \
	src/ast/astNodeSlice.c \
	$(DEP_ASTNODE_SLICE) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeString.o: \
	src/ast/astNodeString.c \
	$(DEP_ASTNODE_STRING) \
	$(DEP_UNICODESTRING) \
	$(DEP_COMPUTEDVALUE_STRING) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeTernary.o: \
	src/ast/astNodeTernary.c \
	$(DEP_ASTNODE_TERNARY) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeUnary.o: \
	src/ast/astNodeUnary.c \
	$(DEP_ASTNODE_UNARY) \
	$(DEP_ASTNODE_INTEGER) \
	$(DEP_ASTNODE_FLOAT) \
	$(DEP_ASTNODE_BOOLEAN) \
	$(DEP_ASTNODE_STRING) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/ast/astNodeUse.o: \
	src/ast/astNodeUse.c \
	$(DEP_ASTNODE_PARSEERROR) \
	$(DEP_ASTNODE_USE) \
	$(DEP_PROGRAM_VARIABLE)

$(OBJ_DIR)/ast/astNodeWhile.o: \
	src/ast/astNodeWhile.c \
	$(DEP_ASTNODE_WHILE) \
	$(DEP_ASTNODE_ASSIGN) \
	$(DEP_ASTNODE_BINARY) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_OPCODE) \
	$(DEP_PROGRAM_BINARY)

$(OBJ_DIR)/tangLanguage.o: \
	src/tangLanguage.c \
	include/tang/tangScanner.h \
	$(DEP_TANGLANGUAGE) \
	$(DEP_MACROS) \
	$(DEP_ASTNODE_IDENTIFIER) \
	$(DEP_ASTNODE_PARSEERROR)

$(OBJ_DIR)/computedValue/computedValue.o: \
	src/computedValue/computedValue.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_FLOAT) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_STRING) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_PROGRAM) \
	$(DEP_MACROS)

$(OBJ_DIR)/computedValue/computedValueArray.o: \
	src/computedValue/computedValueArray.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_ARRAY) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_MACROS)

$(OBJ_DIR)/computedValue/computedValueBoolean.o: \
	src/computedValue/computedValueBoolean.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_FLOAT) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_STRING) \
	$(DEP_COMPUTEDVALUE_ERROR)

$(OBJ_DIR)/computedValue/computedValueError.o: \
	src/computedValue/computedValueError.c \
	$(DEP_COMPUTEDVALUE_ERROR)

$(OBJ_DIR)/computedValue/computedValueFloat.o: \
	src/computedValue/computedValueFloat.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_FLOAT) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_STRING) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueFunction.o: \
	src/computedValue/computedValueFunction.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueFunctionNative.o: \
	src/computedValue/computedValueFunctionNative.c \
	$(DEP_COMPUTEDVALUE_FUNCTIONNATIVE) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueInteger.o: \
	src/computedValue/computedValueInteger.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_FLOAT) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_STRING) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueIterator.o: \
	src/computedValue/computedValueIterator.c \
	$(DEP_COMPUTEDVALUE_ITERATOR) \
	$(DEP_COMPUTEDVALUE_ITERATOREND) \
	$(DEP_COMPUTEDVALUE_ERROR)

$(OBJ_DIR)/computedValue/computedValueLibrary.o: \
	src/computedValue/computedValueLibrary.c \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_LIBRARY) \
	$(DEP_COMPUTEDVALUE_RNG) \
	$(DEP_COMPUTEDVALUE_FUNCTIONNATIVE) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueMap.o: \
	src/computedValue/computedValueMap.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_MAP) \
	$(DEP_COMPUTEDVALUE_STRING) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_UNICODESTRING)

$(OBJ_DIR)/computedValue/computedValueRNG.o: \
	src/computedValue/computedValueRNG.c \
	$(DEP_COMPUTEDVALUE_RNG) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_FLOAT) \
	$(DEP_COMPUTEDVALUE_FUNCTIONNATIVE) \
	$(DEP_EXECUTIONCONTEXT)

$(OBJ_DIR)/computedValue/computedValueString.o: \
	src/computedValue/computedValueString.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_BOOLEAN) \
	$(DEP_COMPUTEDVALUE_FLOAT) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_STRING) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_UNICODESTRING)

$(OBJ_DIR)/library/library.o: \
	src/library/library.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_LIBRARY) \
	$(DEP_PROGRAM)

$(OBJ_DIR)/library/libraryMath.o: \
	src/library/libraryMath.c \
	$(DEP_LIBRARY_MATH) \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_COMPUTEDVALUE_FLOAT) \
	$(DEP_COMPUTEDVALUE_INTEGER) \
	$(DEP_COMPUTEDVALUE_LIBRARY) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_PROGRAM)

$(OBJ_DIR)/library/libraryRandom.o: \
	src/library/libraryRandom.c \
	$(DEP_LIBRARY_RANDOM) \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_COMPUTEDVALUE_ERROR) \
	$(DEP_COMPUTEDVALUE_LIBRARY) \
	$(DEP_COMPUTEDVALUE_RNG) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_PROGRAM)

$(OBJ_DIR)/program/binary.o: \
	src/program/binary.c \
	$(DEP_PROGRAM_BINARY) \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_MACROS)

$(OBJ_DIR)/program/bytecode.o: \
	src/program/bytecode.c \
	$(DEP_BYTECODE) \
	$(DEP_COMPUTEDVALUE)

$(OBJ_DIR)/program/compilerContext.o: \
	src/program/compilerContext.c \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
	$(DEP_PROGRAM) \
	$(DEP_PROGRAM_VARIABLE)

$(OBJ_DIR)/program/executionContext.o: \
	src/program/executionContext.c \
	$(DEP_COMPUTEDVALUE) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_LIBRARY)

$(OBJ_DIR)/program/garbageCollector.o: \
	src/program/garbageCollector.c \
	$(DEP_GARBAGECOLLECTOR)

$(OBJ_DIR)/program/language.o: \
	src/program/language.c \
	$(DEP_COMPUTEDVALUE_LIBRARY) \
	$(DEP_LIBRARY) \
	$(DEP_LIBRARY_MATH) \
	$(DEP_LIBRARY_RANDOM) \
	$(DEP_PROGRAM_LANGUAGE)

$(OBJ_DIR)/program/program.o: \
	src/program/program.c \
	$(DEP_PROGRAM) \
	$(DEP_COMPUTEDVALUE_ALL) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_TANGLANGUAGE) \
	$(DEP_ASTNODEALL) \
	$(DEP_PROGRAM_COMPILERCONTEXT) \
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
	$(DEP_COMPUTEDVALUE_ALL) \
	$(DEP_LIBRARY)

$(OBJ_DIR)/tangParser.o: \
	$(GEN_DIR)/tangParser.c \
	$(DEP_ASTNODE)

$(OBJ_DIR)/tangScanner.o: \
		$(GEN_DIR)/tangScanner.c \
		$(DEP_UNICODESTRING) \
		include/tang/tangScanner.h \
		$(DEP_LOCATION)
	@printf "\n### Compiling $@ ###\n"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -MMD -o $@ $(OS_SPECIFIC_CXX_FLAGS) -Wno-unused-function

$(OBJ_DIR)/unicodeString.o: \
				src/unicodeString.c \
				$(DEP_UNICODESTRING)

####################################################################
# Shared Library
####################################################################

$(APP_DIR)/$(TARGET): \
		$(LIBOBJECTS)
	@printf "\n### Compiling Tang Shared Library ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LDFLAGS) $(OS_SPECIFIC_LIBRARY_NAME_FLAG)

ifeq ($(OS_NAME), Linux)
	@ln -f -s $(TARGET) $(APP_DIR)/$(SO_NAME)
	@ln -f -s $(SO_NAME) $(APP_DIR)/$(BASE_NAME)
endif

####################################################################
# Command Line Utility
####################################################################

$(APP_DIR)/tang$(EXE_EXTENSION): \
				src/tang.c \
				$(DEP_TANG) \
				$(APP_DIR)/$(TARGET)
	@printf "\n### Compiling Tang Command Line Utility ###\n"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $<  $(LDFLAGS) $(TANGLIBRARY)

####################################################################
# Unit Tests
####################################################################

$(APP_DIR)/libtestLibrary.so: \
				test/libtestLibrary.cpp \
				$(APP_DIR)/$(TARGET) \
				include/tang/tang.h
	@printf "\n### Compiling Test Library ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -shared -o $@ $< $(LDFLAGS) -fPIC

$(APP_DIR)/testUnicodeString$(EXE_EXTENSION): \
		test/test-unicodeString.cpp \
		$(OBJ_DIR)/unicodeString.o
#				$(OBJ_DIR)/htmlEscape.o \
#				$(OBJ_DIR)/htmlEscapeAscii.o \
#				$(OBJ_DIR)/percentEncode.o \
#				$(OBJ_DIR)/unescape.o
	@printf "\n### Compiling UnicodeString Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^ $(LDFLAGS) $(TESTFLAGS)

$(APP_DIR)/testTangLanguageParse$(EXE_EXTENSION): \
	test/test-tangLanguageParse.cpp \
	$(DEP_ASTNODE_ALL)
	@printf "\n### Compiling Tang Language Parse Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testTangLanguageExecuteSimple$(EXE_EXTENSION): \
	test/test-tangLanguageExecuteSimple.cpp \
	$(DEP_ASTNODE_ALL) \
	$(DEP_COMPUTEDVALUE_ALL) \
	$(DEP_PROGRAM) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_BYTECODE)
	@printf "\n### Compiling Tang Language Execution Simple Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testTangLanguageExecuteComplex$(EXE_EXTENSION): \
	test/test-tangLanguageExecuteComplex.cpp \
	$(DEP_ASTNODE_ALL) \
	$(DEP_COMPUTEDVALUE_ALL) \
	$(DEP_PROGRAM) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_BYTECODE)
	@printf "\n### Compiling Tang Language Execution Complex Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testTangLanguageLibrary$(EXE_EXTENSION): \
	test/test-tangLanguageLibrary.cpp \
	$(DEP_ASTNODE_ALL) \
	$(DEP_COMPUTEDVALUE_ALL) \
	$(DEP_PROGRAM) \
	$(DEP_EXECUTIONCONTEXT) \
	$(DEP_BYTECODE)
	@printf "\n### Compiling Tang Language Library Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testBinary$(EXE_EXTENSION): \
	test/test-binary.cpp \
	$(DEP_PROGRAM_BINARY)
	@printf "\n### Compiling Binary JIT functions Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/test$(EXE_EXTENSION): \
				test/test.cpp \
				$(DEP_TANG) \
				$(APP_DIR)/$(TARGET)
	@printf "\n### Compiling Tang Test ###\n"
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
					printf "\033[0;32m\n"; \
					printf "#########################\n"; \
					printf "# Waiting for changes.. #\n"; \
					printf "#########################\n"; \
					printf "\033[0m\n"; \
					inotifywait -qr -e modify -e create -e delete -e move src include bison flex test Makefile --exclude '/\.'; \
					done

test-watch: ## Watch the file directory for changes and run the unit tests
	@while true; do \
		make --no-print-directory all; \
		make --no-print-directory test; \
		printf "\033[0;32m\n"; \
		printf "#########################\n"; \
		printf "# Waiting for changes.. #\n"; \
		printf "#########################\n"; \
		printf "\033[0m\n"; \
		inotifywait -qr -e modify -e create -e delete -e move src include bison flex test Makefile --exclude '/\.'; \
		done

test: ## Make and run the Unit tests
test: \
				$(APP_DIR)/$(TARGET) \
				$(APP_DIR)/testUnicodeString$(EXE_EXTENSION) \
				$(APP_DIR)/testTangLanguageParse$(EXE_EXTENSION) \
				$(APP_DIR)/testTangLanguageExecuteSimple$(EXE_EXTENSION) \
				$(APP_DIR)/testTangLanguageExecuteComplex$(EXE_EXTENSION) \
				$(APP_DIR)/testTangLanguageLibrary$(EXE_EXTENSION) \
				$(APP_DIR)/testBinary$(EXE_EXTENSION) \
				$(APP_DIR)/tang$(EXE_EXTENSION)
#				$(APP_DIR)/libtestLibrary.so \
#				$(APP_DIR)/test$(EXE_EXTENSION) \
	@printf "\033[0;30;43m\n"
	@printf "############################\n"
	@printf "### Running string tests ###\n"
	@printf "############################\n"
	@printf "\033[0m\n\n"
	$(APP_DIR)/testUnicodeString --gtest_brief=1
	@printf "\033[0;30;43m\n"
	@printf "####################################\n"
	@printf "### Running Language Parse tests ###\n"
	@printf "####################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" $(APP_DIR)/testTangLanguageParse --gtest_brief=1
	@printf "\033[0;30;43m\n"
	@printf "################################\n"
	@printf "### Running Binary JIT tests ###\n"
	@printf "################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" TANG_DISABLE_BINARY= $(APP_DIR)/testBinary --gtest_brief=1

	@printf "\033[0;30;104m\n"
	@printf "########################################################\n"
	@printf "### Running Bytecode Language Execution Simple tests ###\n"
	@printf "########################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" TANG_DISABLE_BINARY= $(APP_DIR)/testTangLanguageExecuteSimple --gtest_brief=1
	@printf "\033[0;30;104m\n"
	@printf "#########################################################\n"
	@printf "### Running Bytecode Language Execution Complex tests ###\n"
	@printf "#########################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" TANG_DISABLE_BINARY= $(APP_DIR)/testTangLanguageExecuteComplex --gtest_brief=1
	@printf "\033[0;30;104m\n"
	@printf "################################################\n"
	@printf "### Running Bytecode Language Library tests  ###\n"
	@printf "################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" TANG_DISABLE_BINARY= $(APP_DIR)/testTangLanguageLibrary --gtest_brief=1

	@printf "\033[0;30;45m\n"
	@printf "########################################################\n"
	@printf "### Running Binary Language Execution Simple tests   ###\n"
	@printf "########################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" TANG_DISABLE_BYTECODE= $(APP_DIR)/testTangLanguageExecuteSimple --gtest_brief=1
	@printf "\033[0;30;45m\n"
	@printf "########################################################\n"
	@printf "### Running Binary Language Execution Complex tests  ###\n"
	@printf "########################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" TANG_DISABLE_BYTECODE= $(APP_DIR)/testTangLanguageExecuteComplex --gtest_brief=1
	@printf "\033[0;30;45m\n"
	@printf "##############################################\n"
	@printf "### Running Binary Language Library tests  ###\n"
	@printf "##############################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" TANG_DISABLE_BYTECODE= $(APP_DIR)/testTangLanguageLibrary --gtest_brief=1

	@printf "\033[0;30;47m\n"
	@printf "###################\n"
	@printf "### Running CLI ###\n"
	@printf "###################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" $(APP_DIR)/tang -s ./test/fib.tang
	@printf "\033[0;30;47m\n"
	@printf "###################\n"
	@printf "### Running CLI ###\n"
	@printf "###################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(APP_DIR)" $(APP_DIR)/tang ./test/fib.template.tang
#	@printf "\033[0;32m\n"
#	@printf "############################\n"
#	@printf "### Running normal tests ###\n"
#	@printf "############################\n"
#	@printf "\033[0m\n"
#	LD_LIBRARY_PATH="$(APP_DIR)" $(APP_DIR)/test --gtest_brief=1

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
	@mkdir -p $(LIB_INSTALL_PATH)/$(SUITE)
ifeq ($(OS_NAME), Linux)
# Install the .so file
	@cp $(APP_DIR)/$(TARGET) $(LIB_INSTALL_PATH)/$(SUITE)/
	@ln -f -s $(TARGET) $(LIB_INSTALL_PATH)/$(SUITE)/$(SO_NAME)
	@ln -f -s $(SO_NAME) $(LIB_INSTALL_PATH)/$(SUITE)/$(BASE_NAME)
	# Installing the ld configuration file.
	@echo "/usr/local/lib/$(SUITE)" > /etc/ld.so.conf.d/$(SUITE)-$(PROJECT)$(BRANCH).conf
endif
ifeq ($(OS_NAME), Windows)
# The .dll file and the .dll.a file
	@mkdir -p $(BIN_INSTALL_PATH)/$(SUITE)
	@cp $(APP_DIR)/$(TARGET).a $(LIB_INSTALL_PATH)
	@cp $(APP_DIR)/$(TARGET) $(BIN_INSTALL_PATH)
endif
	# Installing the headers.
	@mkdir -p $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)/$(PROJECT)
	@cp include/tang/*.h $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)/$(PROJECT)
	@cp build/generated/*.h $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)/$(PROJECT)
	# Installing the pkg-config files.
	@mkdir -p $(PKG_CONFIG_PATH)
	@cat pkgconfig/$(SUITE)-$(PROJECT).pc | sed 's/(SUITE)/$(SUITE)/g; s/(PROJECT)/$(PROJECT)/g; s/(BRANCH)/$(BRANCH)/g; s/(VERSION)/$(VERSION)/g; s|(LIB)|$(LIB_INSTALL_PATH)|g; s|(INCLUDE)|$(INCLUDE_INSTALL_PATH)|g' > $(PKG_CONFIG_PATH)/$(SUITE)-$(PROJECT)$(BRANCH).pc
ifeq ($(OS_NAME), Linux)
	# Running ldconfig.
	@ldconfig >> /dev/null 2>&1
endif
	@echo "Ghoti.io $(PROJECT)$(BRANCH) installed"

uninstall: ## Delete the globally-installed files.  Requires sudo.
	# Deleting the shared library.
ifeq ($(OS_NAME), Linux)
	@rm -f $(LIB_INSTALL_PATH)/$(SUITE)/$(BASE_NAME)*
	# Deleting the ld configuration file.
	@rm -f /etc/ld.so.conf.d/$(SUITE)-$(PROJECT)$(BRANCH).conf
endif
ifeq ($(OS_NAME), Windows)
	@rm -f $(LIB_INSTALL_PATH)/$(TARGET).a
	@rm -f $(BIN_INSTALL_PATH)/$(TARGET)
endif
	# Deleting the headers.
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)
	# Deleting the pkg-config files.
	@rm -f $(PKG_CONFIG_PATH)/$(SUITE)-$(PROJECT)$(BRANCH).pc
	# Cleaning up (potentially) no longer needed directories.
	@rmdir --ignore-fail-on-non-empty $(INCLUDE_INSTALL_PATH)/$(SUITE)
	@rmdir --ignore-fail-on-non-empty $(LIB_INSTALL_PATH)/$(SUITE)
ifeq ($(OS_NAME), Linux)
	# Running ldconfig.
	@ldconfig >> /dev/null 2>&1
endif
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

