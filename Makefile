SUITE := ghoti.io
PROJECT := tang

BUILD ?= release
BRANCH := -cdev
# If BUILD is debug, append -debug
ifeq ($(BUILD),debug)
    BRANCH := $(BRANCH)-debug
endif

BASE_NAME := lib$(SUITE)-$(PROJECT)$(BRANCH).so
# The symbol namespace token, from BRANCH. See CONVENTIONS.md section 4.
LIBVER_SYMBOL := $(shell echo "ghotiio_$(PROJECT)$(BRANCH)" | sed 's/[.-]/_/g')

BASE_NAME_PREFIX := lib$(SUITE)-$(PROJECT)$(BRANCH)
STATIC_TARGET := $(BASE_NAME_PREFIX).a
MAJOR_VERSION := 0
MINOR_VERSION := 0.0
# Substituted into the .pc file; an empty Version: field makes every
# pkg-config version constraint fail.
VERSION := $(MAJOR_VERSION).$(MINOR_VERSION)
SO_NAME := $(BASE_NAME).$(MAJOR_VERSION)

BUILD ?= release

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
	BUILD := linux/$(BUILD)

else ifeq ($(UNAME_S), Darwin)
	OS_NAME := Mac
	LIB_EXTENSION := dylib
	OS_SPECIFIC_CXX_FLAGS := -shared
	OS_SPECIFIC_LIBRARY_NAME_FLAG := -Wl,-install_name,$(BASE_NAME_PREFIX).dylib
	TARGET := $(BASE_NAME_PREFIX).dylib
	EXE_EXTENSION :=
	# Additional macOS-specific variables
	BUILD := mac/$(BUILD)

else ifeq ($(findstring MINGW32_NT,$(UNAME_S)),MINGW32_NT)  # 32-bit Windows
	OS_NAME := Windows
	LIB_EXTENSION := dll
	OS_SPECIFIC_CXX_FLAGS := -shared
	OS_SPECIFIC_LIBRARY_NAME_FLAG = -Wl,--out-implib,$(APP_DIR)/$(BASE_NAME_PREFIX).dll.a
	TARGET := $(BASE_NAME_PREFIX).dll
	EXE_EXTENSION := .exe
	# Additional Windows-specific variables
	# This is the path to the pkg-config files on MSYS2
	PKG_CONFIG_PATH := /mingw32/lib/pkgconfig
	INCLUDE_INSTALL_PATH := /mingw32/include
	LIB_INSTALL_PATH := /mingw32/lib
	BIN_INSTALL_PATH := /mingw32/bin
	BUILD := win32/$(BUILD)

else ifeq ($(findstring MINGW64_NT,$(UNAME_S)),MINGW64_NT)  # 64-bit Windows
	OS_NAME := Windows
	LIB_EXTENSION := dll
	OS_SPECIFIC_CXX_FLAGS := -shared
	OS_SPECIFIC_LIBRARY_NAME_FLAG = -Wl,--out-implib,$(APP_DIR)/$(BASE_NAME_PREFIX).dll.a
	TARGET := $(BASE_NAME_PREFIX).dll
	EXE_EXTENSION := .exe
	# Additional Windows-specific variables
	# This is the path to the pkg-config files on MSYS2
	PKG_CONFIG_PATH := /mingw64/lib/pkgconfig
	INCLUDE_INSTALL_PATH := /mingw64/include
	LIB_INSTALL_PATH := /mingw64/lib
	BIN_INSTALL_PATH := /mingw64/bin
	BUILD := win64/$(BUILD)

else
    $(error Unsupported OS: $(UNAME_S))

endif

# ---------------------------------------------------------------------------
# Installation prefix
#
# Defaults to the system location chosen above. Override it to install
# somewhere else - the suite's bootstrap installs every library into a local
# prefix so that each build resolves its dependencies through pkg-config,
# exactly as a consumer would, rather than through a second code path that
# only in-tree builds exercise. See CONVENTIONS.md section 1.
#
#     make install PREFIX=/path/to/prefix
# ---------------------------------------------------------------------------
ifdef PREFIX
INCLUDE_INSTALL_PATH := $(PREFIX)/include
LIB_INSTALL_PATH := $(PREFIX)/lib
BIN_INSTALL_PATH := $(PREFIX)/bin
PKG_CONFIG_PATH := $(PREFIX)/share/pkgconfig
ifeq ($(OS_NAME), Windows)
PC_INCLUDE_DIR = $(shell cygpath -m $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH))
PC_LIB_DIR = $(shell cygpath -m $(LIB_INSTALL_PATH)/$(SUITE))
else
PC_INCLUDE_DIR := $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)
PC_LIB_DIR := $(LIB_INSTALL_PATH)/$(SUITE)
endif
# A non-system prefix has no /etc/ld.so.conf.d, and writing to it would need
# root anyway. Everything built here carries an rpath to the prefix instead.
LDCONF_INSTALL_PATH :=
endif



CXX := g++
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -Wno-error=unused-function -Wfatal-errors -std=c++20 -O1 -g $(EXTRA_CXXFLAGS)
CC := cc
# cutil comes from pkg-config when installed; otherwise fall back to a sibling
# checkout so the suite builds from a fresh clone without installing it first.
CUTIL_CFLAGS := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags ghoti.io-cutil-dev 2>/dev/null)
CUTIL_LIBS := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs ghoti.io-cutil-dev 2>/dev/null)
ifeq ($(strip $(CUTIL_CFLAGS)),)
$(error ghoti.io-cutil was not found by pkg-config. Run ./bootstrap.sh in the parent folder to build and install the suite into a local prefix, then pass the same PREFIX here - or point PKG_CONFIG_PATH at the directory holding its .pc file. There is deliberately no sibling-checkout fallback: a second resolution path that only in-tree builds exercise is one that silently rots.)
endif
ICU_CFLAGS := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags icu-io icu-i18n icu-uc)
ICU_LIBS := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs icu-io icu-i18n icu-uc)
CFLAGS := -pedantic-errors -Wall -Wextra -Werror -Wno-error=unused-function -Wfatal-errors -std=c17 -O0 -g $(ICU_CFLAGS) $(CUTIL_CFLAGS) $(EXTRA_CFLAGS)

# The shipped library exports its public API and nothing else. Tests reach the
# internals by linking the static archive, which a static link can do even for
# hidden symbols.
LIB_CFLAGS := $(CFLAGS) -fvisibility=hidden -DGHOTIIO_TANG_BUILD
# -DGHOTIIO_CUTIL_ENABLE_MEMORY_DEBUG
LDFLAGS := -L /usr/lib -lstdc++ -lm $(ICU_LIBS) $(CUTIL_LIBS) $(EXTRA_LDFLAGS)
ifdef PREFIX
# So that a library, a test or an example finds its Ghoti.io dependencies in the
# prefix at run time without LD_LIBRARY_PATH.
LDFLAGS += -Wl,-rpath,$(LIB_INSTALL_PATH)/$(SUITE)
endif

# The C++ test translation units include cutil and ICU headers too.
CXXFLAGS += $(ICU_CFLAGS) $(CUTIL_CFLAGS)
BUILD_DIR := ./build/$(BUILD)
OBJ_DIR := $(BUILD_DIR)/objects
GEN_DIR := $(BUILD_DIR)/generated
APP_DIR := $(BUILD_DIR)/apps


INCLUDE := -I include/ -I $(GEN_DIR)/
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


# The static archive, not -l: a static link resolves hidden symbols, so the
# tests can exercise internals the shared library does not export. ICU and
# cutil follow it, because an archive carries no DT_NEEDED of its own.
TANGLIBRARY := -Wl,--whole-archive $(APP_DIR)/$(STATIC_TARGET) -Wl,--no-whole-archive $(ICU_LIBS) $(CUTIL_LIBS)


all: $(APP_DIR)/$(TARGET) $(APP_DIR)/$(STATIC_TARGET) $(APP_DIR)/tang$(EXE_EXTENSION) ## Build the shared and static libraries

####################################################################
# Dependency Inclusion
####################################################################
# Compiler-generated .d files (see -MMD -MP -MF in compile commands).
TEST_EXE_NAMES := testUnicodeString testTangLanguageParse testTangLanguageExecuteSimple testTangLanguageExecuteComplex testTangLanguageLibrary testBinary test
TEST_DEPFILES := $(addprefix $(APP_DIR)/,$(TEST_EXE_NAMES:%=%.d))
TANG_CLI_DEP := $(APP_DIR)/tang.d
DEPFILES := $(LIBOBJECTS:.o=.d) $(TEST_DEPFILES) $(TANG_CLI_DEP)
# Sanitize existing .d files so Windows paths (C:/) are escaped before -include (fixes "multiple target patterns" on Linux when .d files came from a Windows build).
$(shell for f in $(DEPFILES); do test -f "$$f" && sed -i 's|\([A-Za-z]\):\([\\/]\)|\1\\:\2|g' "$$f"; done 2>/dev/null)
-include $(DEPFILES)

####################################################################
# Bison-Generated Files
####################################################################
$(GEN_DIR)/tangParser.h: \
				bison/tangParser.y \
				include/ghoti.io/tang/ast/astNode.h \
				include/ghoti.io/tang/location.h
	@echo "\n### Generating Bison TangParser ###"
	@mkdir -p $(@D)
	bison -v -o $(GEN_DIR)/tangParser.c -d $<

# Almost every translation unit reaches tangParser.h through
# include/ghoti.io/tang/ast/astNode.h, but nothing ordered them after bison. A serial
# build happened to generate the parser first; with -j the AST objects raced
# ahead and failed with "tangParser.h: No such file or directory". Order-only
# so a regenerated header does not force a full rebuild.
$(LIBOBJECTS): | $(GEN_DIR)/tangParser.h

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
		include/ghoti.io/tang/unicodeString.h \
		include/ghoti.io/tang/macros.h \
		include/ghoti.io/tang/libver.h
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
		include/ghoti.io/tang/unicodeString.h \
		include/ghoti.io/tang/macros.h \
		include/ghoti.io/tang/libver.h
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
# Pattern rule: src/%.c -> objects/%.o; compiler generates .d (headers tracked automatically).
# Sed escapes Windows C: in .d so make does not treat "C" as a target.
####################################################################
# Generated version header
####################################################################

LIBVER_GEN := $(GEN_DIR)/ghoti.io/tang/libver_gen.h

$(LIBVER_GEN): Makefile
	@if [ -z "$(LIBVER_SYMBOL)" ]; then \
		printf "### LIBVER_SYMBOL is empty ###\n" >&2; exit 1; \
	fi
	@mkdir -p $(@D)
	@printf '%s\n' \
		'// Generated by the Makefile. Do not edit; see CONVENTIONS.md section 4.' \
		'#ifndef GHOTI_IO_GTA_LIBVER_GEN_H' \
		'#define GHOTI_IO_GTA_LIBVER_GEN_H' \
		'' \
		'/** The symbol namespace for this build, from the Makefile'"'"'s BRANCH. */' \
		'#define GHOTIIO_TANG_NAME $(LIBVER_SYMBOL)' \
		'' \
		'/** Human-readable version of this build. */' \
		'#define GHOTIIO_TANG_VERSION "$(MAJOR_VERSION).$(MINOR_VERSION)$(BRANCH)"' \
		'' \
		'#endif // GHOTI_IO_GTA_LIBVER_GEN_H' > $@

$(OBJ_DIR)/%.o: src/%.c | $(LIBVER_GEN)
	@printf "\n### Compiling $@ ###\n"
	@mkdir -p $(@D)
	$(CC) $(LIB_CFLAGS) $(INCLUDE) -c $< -MMD -MP -MF $(@:.o=.d) -o $@ $(OS_SPECIFIC_CXX_FLAGS)
	@sed -i 's|\([A-Za-z]\):\([\\/]\)|\1\\:\2|g' $(@:.o=.d) 2>/dev/null || true

# Generated sources (bison/flex): same flags, output .d next to .o.
$(OBJ_DIR)/tangParser.o: $(GEN_DIR)/tangParser.c
	@printf "\n### Compiling $@ ###\n"
	@mkdir -p $(@D)
	$(CC) $(LIB_CFLAGS) $(INCLUDE) -c $< -MMD -MP -MF $(@:.o=.d) -o $@ $(OS_SPECIFIC_CXX_FLAGS)
	@sed -i 's|\([A-Za-z]\):\([\\/]\)|\1\\:\2|g' $(@:.o=.d) 2>/dev/null || true

$(OBJ_DIR)/tangScanner.o: $(GEN_DIR)/tangScanner.c
	@printf "\n### Compiling $@ ###\n"
	@mkdir -p $(@D)
	$(CC) $(LIB_CFLAGS) $(INCLUDE) -c $< -MMD -MP -MF $(@:.o=.d) -o $@ $(OS_SPECIFIC_CXX_FLAGS) -Wno-unused-function
	@sed -i 's|\([A-Za-z]\):\([\\/]\)|\1\\:\2|g' $(@:.o=.d) 2>/dev/null || true

####################################################################
# Shared Library
####################################################################

$(APP_DIR)/$(STATIC_TARGET): $(LIBOBJECTS)
	@printf "\n### Archiving Tang Library ###\n"
	@mkdir -p $(@D)
	@rm -f $@
	ar rcs $@ $^

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

$(APP_DIR)/tang$(EXE_EXTENSION): src/tang.c | $(APP_DIR)/$(TARGET) $(APP_DIR)/$(STATIC_TARGET)
	@printf "\n### Compiling Tang Command Line Utility ###\n"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -MMD -MP -MF $(APP_DIR)/tang.d -o $@ $< $(LDFLAGS) $(TANGLIBRARY)

####################################################################
# Unit Tests
####################################################################

$(APP_DIR)/libtestLibrary.so: \
				test/libtestLibrary.cpp \
				$(APP_DIR)/$(TARGET) \
				include/ghoti.io/tang/tang.h
	@printf "\n### Compiling Test Library ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -shared -o $@ $< $(LDFLAGS) -fPIC

$(APP_DIR)/testUnicodeString$(EXE_EXTENSION): test/test-unicodeString.cpp $(OBJ_DIR)/unicodeString.o
	@printf "\n### Compiling UnicodeString Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(APP_DIR)/testUnicodeString.d -o $@ $^ $(LDFLAGS) $(TESTFLAGS)

$(APP_DIR)/testTangLanguageParse$(EXE_EXTENSION): test/test-tangLanguageParse.cpp | $(APP_DIR)/$(TARGET) $(APP_DIR)/$(STATIC_TARGET)
	@printf "\n### Compiling Tang Language Parse Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(APP_DIR)/testTangLanguageParse.d -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testTangLanguageExecuteSimple$(EXE_EXTENSION): test/test-tangLanguageExecuteSimple.cpp | $(APP_DIR)/$(TARGET) $(APP_DIR)/$(STATIC_TARGET)
	@printf "\n### Compiling Tang Language Execution Simple Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(APP_DIR)/testTangLanguageExecuteSimple.d -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testTangLanguageExecuteComplex$(EXE_EXTENSION): test/test-tangLanguageExecuteComplex.cpp | $(APP_DIR)/$(TARGET) $(APP_DIR)/$(STATIC_TARGET)
	@printf "\n### Compiling Tang Language Execution Complex Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(APP_DIR)/testTangLanguageExecuteComplex.d -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testTangLanguageLibrary$(EXE_EXTENSION): test/test-tangLanguageLibrary.cpp | $(APP_DIR)/$(TARGET) $(APP_DIR)/$(STATIC_TARGET)
	@printf "\n### Compiling Tang Language Library Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(APP_DIR)/testTangLanguageLibrary.d -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/testBinary$(EXE_EXTENSION): test/test-binary.cpp | $(APP_DIR)/$(TARGET) $(APP_DIR)/$(STATIC_TARGET)
	@printf "\n### Compiling Binary JIT functions Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(APP_DIR)/testBinary.d -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

$(APP_DIR)/test$(EXE_EXTENSION): test/test.cpp | $(APP_DIR)/$(TARGET) $(APP_DIR)/$(STATIC_TARGET)
	@printf "\n### Compiling Tang Test ###\n"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(APP_DIR)/test.d -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(TANGLIBRARY)

####################################################################
# Commands
####################################################################

# General commands
.PHONY: clean cloc docs docs-pdf coverage
# Release build commands
.PHONY: all install test test-watch uninstall watch jit-alignment-check check-symbols
# Debug build commands
.PHONY: all-debug install-debug test-debug test-watch-debug uninstall-debug watch-debug


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

# So tests can load the tang library and its cutil dependency.
TEST_LD_PATH := $(APP_DIR):$(CUTIL_SIBLING_DIR)/apps

jit-alignment-check: ## Rebuild with the JIT stack-alignment check and run the tests
# The System V AMD64 ABI requires rsp to be 16-byte aligned at every call. The
# JIT prologue establishes that, so alignment holds in the body only while an
# even number of 8-byte slots are live. Getting it wrong is silent until some
# callee happens to use an alignment-sensitive instruction (movdqa, say) on a
# stack local, at which point it faults deep inside that callee with nothing
# pointing back at the offending call.
#
# This target rebuilds with -DGTA_JIT_CHECK_STACK_ALIGNMENT, which makes every
# emitted call test rsp first and execute ud2 (SIGILL) when it is misaligned.
# A violation then traps AT the offending call. Run this after changing any
# code that emits pushes or pops around a call.
	$(MAKE) clean
	$(MAKE) test \
		CFLAGS="$(CFLAGS) -DGTA_JIT_CHECK_STACK_ALIGNMENT" \
		CXXFLAGS="$(CXXFLAGS) -DGTA_JIT_CHECK_STACK_ALIGNMENT"

####################################################################
# Symbol namespace check
####################################################################

check-symbols: ## Fail if any exported symbol lacks the version namespace
check-symbols: $(APP_DIR)/$(TARGET)
ifeq ($(OS_NAME), Linux)
	@leaked=$$(nm -D --defined-only $(APP_DIR)/$(TARGET) \
		| awk '$$2 ~ /^[TDBR]$$/ {print $$3}' \
		| grep -v '^$(LIBVER_SYMBOL)_' | grep -v '^_' || true); \
	if [ -n "$$leaked" ]; then \
		printf "\033[0;31m\n### Exported symbols missing the $(LIBVER_SYMBOL)_ namespace ###\033[0m\n" >&2; \
		printf "%s\n" "$$leaked" >&2; \
		printf "\nEach needs a '#define <name> GHOTIIO_TANG(<name>)' line in namespace.h, or\n" >&2; \
		printf "should not be exported at all. See CONVENTIONS.md section 4.\n" >&2; \
		exit 1; \
	fi
	@split=$$(nm -D --undefined-only $(APP_DIR)/$(TARGET) \
		| awk '{print $$2}' | grep '^$(LIBVER_SYMBOL)_' || true); \
	if [ -n "$$split" ]; then \
		printf "\033[0;31m\n### Renamed but undefined - a split symbol ###\033[0m\n" >&2; \
		printf "%s\n" "$$split" >&2; \
		printf "\nA translation unit referenced the namespaced name while the one that\n" >&2; \
		printf "defines it did not see the rename.\n" >&2; \
		exit 1; \
	fi
	@unexported=$$(find include -name '*.h' -exec awk '/^#if DOXYGEN/{d=1} d==0 && /^[a-z_][A-Za-z0-9_ ]*\**[[:space:]]*gta_[a-z0-9_]+[[:space:]]*\(/{print FILENAME": "$$0} /^#endif/{d=0}' {} + \
		| grep -vE 'typedef|static inline' || true); \
	if [ -n "$$unexported" ]; then \
		printf "\033[0;31m\n### Public declarations without GTA_API ###\033[0m\n" >&2; \
		printf "%s\n" "$$unexported" >&2; \
		printf "\nThe library builds with -fvisibility=hidden, so these are not exported\n" >&2; \
		printf "and a consumer linking the .so gets an undefined reference. The tests\n" >&2; \
		printf "link the archive and would not notice.\n" >&2; \
		exit 1; \
	fi
	@nomacros=$$(find include src -name '*.h' \
		! -name 'libver.h' ! -name 'libver_gen.h' ! -name 'namespace.h' ! -name 'macros.h' \
		-exec grep -L '#include <ghoti.io/tang/macros.h>' {} + || true); \
	if [ -n "$$nomacros" ]; then \
		printf "\033[0;31m\n### Headers that do not include macros.h ###\033[0m\n" >&2; \
		printf "%s\n" "$$nomacros" >&2; \
		printf "\nEvery header must include <ghoti.io/tang/macros.h> before it declares\n" >&2; \
		printf "anything, so that the renames in namespace.h are already in effect. A\n" >&2; \
		printf "header that skips it can name a type before that type has been renamed,\n" >&2; \
		printf "producing two different types under one spelling.\n" >&2; \
		printf "See CONVENTIONS.md section 4.\n" >&2; \
		exit 1; \
	fi
	@badguards=$$(find include src -name '*.h' -exec awk 'FNR==1{d=0} !d && /^#ifndef/{print $$2; d=1}' {} + \
		| awk '$$1 !~ /^GHOTI_IO_GTA_/ {print $$1}' || true); \
	if [ -n "$$badguards" ]; then \
		printf "\033[0;31m\n### Include guards with the wrong prefix ###\033[0m\n" >&2; \
		printf "%s\n" "$$badguards" >&2; \
		printf "\nGuards mirror the path: GHOTI_IO_GTA_<PATH>_H. A guard without the\n" >&2; \
		printf "library token is one rename away from colliding with another library's.\n" >&2; \
		exit 1; \
	fi
	@dupguards=$$(find include src -name '*.h' -exec awk 'FNR==1{d=0} !d && /^#ifndef/{print $$2; d=1}' {} + \
		| sort | uniq -d || true); \
	if [ -n "$$dupguards" ]; then \
		printf "\033[0;31m\n### Headers sharing an include guard ###\033[0m\n" >&2; \
		printf "%s\n" "$$dupguards" >&2; \
		printf "\nTwo headers with one guard means whichever is included second is\n" >&2; \
		printf "silently empty. Guards mirror the path: GHOTI_IO_GTA_<PATH>_H.\n" >&2; \
		exit 1; \
	fi
	@printf "\033[0;32mEvery exported symbol carries the $(LIBVER_SYMBOL)_ namespace.\033[0m\n"
	@printf "\033[0;32mEvery public declaration carries GTA_API.\033[0m\n"
	@printf "\033[0;32mEvery header includes macros.h.\033[0m\n"
	@printf "\033[0;32mEvery include guard is unique and correctly prefixed.\033[0m\n"
else
	@printf "check-symbols: skipped (Linux only)\n"
endif

test: ## Make and run the Unit tests
test: \
				$(APP_DIR)/$(TARGET) \
				$(APP_DIR)/testUnicodeString$(EXE_EXTENSION) \
				$(APP_DIR)/testTangLanguageParse$(EXE_EXTENSION) \
				$(APP_DIR)/testTangLanguageExecuteSimple$(EXE_EXTENSION) \
				$(APP_DIR)/testTangLanguageExecuteComplex$(EXE_EXTENSION) \
				$(APP_DIR)/testTangLanguageLibrary$(EXE_EXTENSION) \
				$(APP_DIR)/testBinary$(EXE_EXTENSION) \
				$(APP_DIR)/tang$(EXE_EXTENSION) \
				check-symbols
#				$(APP_DIR)/libtestLibrary.so \
#				$(APP_DIR)/test$(EXE_EXTENSION) \

	@printf "\033[0;30;103m\n"
	@printf "############################\n"
	@printf "### Running string tests ###\n"
	@printf "############################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" $(APP_DIR)/testUnicodeString --gtest_brief=1
	@printf "\033[0;30;43m\n"
	@printf "####################################\n"
	@printf "### Running Language Parse tests ###\n"
	@printf "####################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" $(APP_DIR)/testTangLanguageParse --gtest_brief=1
	@printf "\033[0;30;43m\n"
	@printf "################################\n"
	@printf "### Running Binary JIT tests ###\n"
	@printf "################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" TANG_DISABLE_BINARY= $(APP_DIR)/testBinary --gtest_brief=1

	@printf "\033[0;30;104m\n"
	@printf "########################################################\n"
	@printf "### Running Bytecode Language Execution Simple tests ###\n"
	@printf "########################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" TANG_DISABLE_BINARY= $(APP_DIR)/testTangLanguageExecuteSimple --gtest_brief=1
	@printf "\033[0;30;104m\n"
	@printf "#########################################################\n"
	@printf "### Running Bytecode Language Execution Complex tests ###\n"
	@printf "#########################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" TANG_DISABLE_BINARY= $(APP_DIR)/testTangLanguageExecuteComplex --gtest_brief=1
	@printf "\033[0;30;104m\n"
	@printf "################################################\n"
	@printf "### Running Bytecode Language Library tests  ###\n"
	@printf "################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" TANG_DISABLE_BINARY= $(APP_DIR)/testTangLanguageLibrary --gtest_brief=1

	@printf "\033[0;30;45m\n"
	@printf "########################################################\n"
	@printf "### Running Binary Language Execution Simple tests   ###\n"
	@printf "########################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" TANG_DISABLE_BYTECODE= $(APP_DIR)/testTangLanguageExecuteSimple --gtest_brief=1
	@printf "\033[0;30;45m\n"
	@printf "########################################################\n"
	@printf "### Running Binary Language Execution Complex tests  ###\n"
	@printf "########################################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" TANG_DISABLE_BYTECODE= $(APP_DIR)/testTangLanguageExecuteComplex --gtest_brief=1
	@printf "\033[0;30;45m\n"
	@printf "##############################################\n"
	@printf "### Running Binary Language Library tests  ###\n"
	@printf "##############################################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" TANG_DISABLE_BYTECODE= $(APP_DIR)/testTangLanguageLibrary --gtest_brief=1

	@printf "\033[0;30;47m\n"
	@printf "###################\n"
	@printf "### Running CLI ###\n"
	@printf "###################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" $(APP_DIR)/tang -s ./test/fib.tang
	@printf "\033[0;30;47m\n"
	@printf "###################\n"
	@printf "### Running CLI ###\n"
	@printf "###################\n"
	@printf "\033[0m\n\n"
	LD_LIBRARY_PATH="$(TEST_LD_PATH)" $(APP_DIR)/tang ./test/fib.template.tang
#	@printf "\033[0;32m\n"
#	@printf "############################\n"
#	@printf "### Running normal tests ###\n"
#	@printf "############################\n"
#	@printf "\033[0m\n"
#	LD_LIBRARY_PATH="$(TEST_LD_PATH)" $(APP_DIR)/test --gtest_brief=1

clean: ## Remove all contents of the build directory for this OS/build.
	-@rm -rvf $(BUILD_DIR)

# Files will be as follows:
# /usr/local/lib/(SUITE)/
#   lib(SUITE)-(PROJECT)(BRANCH).so.(MAJOR).(MINOR)
#   lib(SUITE)-(PROJECT)(BRANCH).so.(MAJOR) link to previous
#   lib(SUITE)-(PROJECT)(BRANCH).so link to previous
# Where the dynamic loader configuration fragment goes. Overridable so a
# staged or user-prefix install has somewhere to write it; the default is the
# system location, which is what an ordinary `sudo make install` uses.
LDCONF_INSTALL_PATH ?= /etc/ld.so.conf.d

# Where this project's own .pc file is installed. Defaults to the directory
# pkg-config is already being told to search, but separate from it so a
# staged install can write somewhere else without also redirecting lookups.
PKGCONFIG_INSTALL_PATH ?= $(PKG_CONFIG_PATH)

# $(LDCONF_INSTALL_PATH)/(SUITE)-(PROJECT)(BRANCH).conf will point to $(LIB_INSTALL_PATH)/(SUITE)
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
	@if [ -n "$(LDCONF_INSTALL_PATH)" ]; then mkdir -p $(LDCONF_INSTALL_PATH); fi
	@if [ -n "$(LDCONF_INSTALL_PATH)" ]; then echo "$(LIB_INSTALL_PATH)/$(SUITE)" > $(LDCONF_INSTALL_PATH)/$(SUITE)-$(PROJECT)$(BRANCH).conf; fi
endif
ifeq ($(OS_NAME), Windows)
# The .dll file and the .dll.a file
	@mkdir -p $(BIN_INSTALL_PATH)/$(SUITE)
	@cp $(APP_DIR)/$(TARGET).a $(LIB_INSTALL_PATH)
	@cp $(APP_DIR)/$(TARGET) $(BIN_INSTALL_PATH)
endif
	# Installing the headers.
	# Removed first: this directory is owned entirely by this project and
	# branch, and copying over the top of it would leave headers behind that
	# have since been renamed or deleted.
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)
	@mkdir -p $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)
	# Copied with --parents so the subdirectories survive. This used to be a
	# flat `cp include/ghoti.io/tang/*.h`, which installed 10 of the 68 headers
	# and none of the generated ones: tang.h includes program/program.h, so the
	# installed library could not actually be included by a consumer.
	@cd include && find . -name "*.h" -exec cp --parents '{}' $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)/ \;
	@cd $(GEN_DIR) && find . -name "*.h" -exec cp --parents '{}' $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)/ \;
	# Installing the pkg-config files.
	@mkdir -p $(PKGCONFIG_INSTALL_PATH)
	@cat pkgconfig/$(SUITE)-$(PROJECT).pc | sed 's/(SUITE)/$(SUITE)/g; s/(PROJECT)/$(PROJECT)/g; s/(BRANCH)/$(BRANCH)/g; s/(VERSION)/$(VERSION)/g; s|(LIB)|$(LIB_INSTALL_PATH)|g; s|(INCLUDE)|$(INCLUDE_INSTALL_PATH)|g' > $(PKGCONFIG_INSTALL_PATH)/$(SUITE)-$(PROJECT)$(BRANCH).pc
ifeq ($(OS_NAME), Linux)
	# Running ldconfig.
	@if [ -n "$(LDCONF_INSTALL_PATH)" ]; then ldconfig >> /dev/null 2>&1; fi
endif
	@echo "Ghoti.io $(PROJECT)$(BRANCH) installed"

uninstall: ## Delete the globally-installed files.  Requires sudo.
	# Deleting the shared library.
ifeq ($(OS_NAME), Linux)
	@rm -f $(LIB_INSTALL_PATH)/$(SUITE)/$(BASE_NAME)*
	# Deleting the ld configuration file.
	@rm -f $(LDCONF_INSTALL_PATH)/$(SUITE)-$(PROJECT)$(BRANCH).conf
endif
ifeq ($(OS_NAME), Windows)
	@rm -f $(LIB_INSTALL_PATH)/$(TARGET).a
	@rm -f $(BIN_INSTALL_PATH)/$(TARGET)
endif
	# Deleting the headers.
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(SUITE)/$(PROJECT)$(BRANCH)
	# Deleting the pkg-config files.
	@rm -f $(PKGCONFIG_INSTALL_PATH)/$(SUITE)-$(PROJECT)$(BRANCH).pc
	# Cleaning up (potentially) no longer needed directories.
	@rmdir --ignore-fail-on-non-empty $(INCLUDE_INSTALL_PATH)/$(SUITE)
	@rmdir --ignore-fail-on-non-empty $(LIB_INSTALL_PATH)/$(SUITE)
ifeq ($(OS_NAME), Linux)
	# Running ldconfig.
	@if [ -n "$(LDCONF_INSTALL_PATH)" ]; then ldconfig >> /dev/null 2>&1; fi
endif
	@echo "Ghoti.io $(PROJECT)$(BRANCH) has been uninstalled"

debug: ## Build the project in DEBUG mode
	make all BUILD=debug

install-debug: ## Install the DEBUG library globally, requires sudo
	make install BUILD=debug

uninstall-debug: ## Delete the DEBUG globally-installed files.  Requires sudo.
	make uninstall BUILD=debug

test-debug: ## Make and run the Unit tests in DEBUG mode
	make test BUILD=debug

watch-debug: ## Watch the file directory for changes and compile the target in DEBUG mode
	make watch BUILD=debug

test-watch-debug: ## Watch the file directory for changes and run the unit tests in DEBUG mode
	make test-watch BUILD=debug

docs: ## Generate the documentation in the ./docs subdirectory
	doxygen

docs-pdf: docs ## Generate the documentation as a pdf, at ./docs/(SUITE)-(PROJECT)(BRANCH).pdf
	cd ./docs/latex/ && make
	mv -f ./docs/latex/refman.pdf ./docs/$(SUITE)-$(PROJECT)$(BRANCH)-docs.pdf

cloc: ## Count the lines of code used in the project
	cloc src include flex bison test Makefile

coverage: ## Build instrumented, run the tests, and report line coverage
# Cleans first because the object files would otherwise be reused without the
# instrumentation, then cleans and rebuilds at the end: leaving the
# instrumented objects behind would have a later `make` silently link them,
# and leaving the tree cleaned would break any sibling project that links
# this one. The cost is one extra build; coverage is not run often.
	@$(MAKE) --no-print-directory clean > /dev/null
	@$(MAKE) --no-print-directory test \
		EXTRA_CFLAGS="--coverage -O0" \
		EXTRA_LDFLAGS="--coverage" > /dev/null
	@tools/coverage.sh $(OBJ_DIR)
	@$(MAKE) --no-print-directory clean > /dev/null
	@$(MAKE) --no-print-directory all > /dev/null

help: ## Display this help
# Scan only this makefile. $(MAKEFILE_LIST) grows to include every generated
# .d file once the project has been built, and grep prefixes each match with
# a filename when given more than one file - so every target name in the
# output became "Makefile".
	@grep -E '^[ a-zA-Z_-]+:.*?## .*$$' $(firstword $(MAKEFILE_LIST)) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "%-15s %s\n", $$1, $$2}' | sed "s/(SUITE)/$(SUITE)/g; s/(PROJECT)/$(PROJECT)/g; s/(BRANCH)/$(BRANCH)/g"

