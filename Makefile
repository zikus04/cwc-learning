# CWC - Custom Wayland Compositor Makefile
# Modern C build system with security and optimization flags

# Project configuration
PROJECT_NAME = cwc
VERSION = 1.0.0
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

# Compiler and tools
CC ?= gcc
PKG_CONFIG ?= pkg-config
INSTALL ?= install
STRIP ?= strip

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = build
TESTDIR = tests
DOCDIR = docs

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
HEADERS = $(wildcard $(INCDIR)/*.h)

# Dependencies
PKGS = wayland-server wayland-protocols
LIBS = $(shell $(PKG_CONFIG) --libs $(PKGS))
CFLAGS_PKG = $(shell $(PKG_CONFIG) --cflags $(PKGS))

# Compiler flags
CFLAGS_BASE = -std=c11 -I$(INCDIR) $(CFLAGS_PKG)

# Security flags (hardening)
CFLAGS_SECURITY = -fstack-protector-strong \
                  -fPIE \
                  -D_FORTIFY_SOURCE=2 \
                  -Wformat \
                  -Wformat-security \
                  -fno-common

# Warning flags
CFLAGS_WARNINGS = -Wall \
                  -Wextra \
                  -Wpedantic \
                  -Wshadow \
                  -Wpointer-arith \
                  -Wcast-align \
                  -Wwrite-strings \
                  -Wmissing-prototypes \
                  -Wmissing-declarations \
                  -Wredundant-decls \
                  -Wnested-externs \
                  -Winline \
                  -Wno-long-long \
                  -Wuninitialized \
                  -Wconversion \
                  -Wstrict-prototypes

# Linker flags
LDFLAGS_SECURITY = -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack

# Build configurations
CFLAGS_DEBUG = $(CFLAGS_BASE) $(CFLAGS_WARNINGS) $(CFLAGS_SECURITY) -g -O0 -DDEBUG -fsanitize=address,undefined
CFLAGS_RELEASE = $(CFLAGS_BASE) $(CFLAGS_SECURITY) -O2 -DNDEBUG -flto
CFLAGS_PROFILE = $(CFLAGS_RELEASE) -pg -fprofile-arcs -ftest-coverage

LDFLAGS_DEBUG = $(LDFLAGS_SECURITY) -fsanitize=address,undefined
LDFLAGS_RELEASE = $(LDFLAGS_SECURITY) -flto
LDFLAGS_PROFILE = $(LDFLAGS_RELEASE) -pg -fprofile-arcs -ftest-coverage

# Default build type
BUILD_TYPE ?= debug

ifeq ($(BUILD_TYPE),release)
    CFLAGS = $(CFLAGS_RELEASE)
    LDFLAGS = $(LDFLAGS_RELEASE)
else ifeq ($(BUILD_TYPE),profile)
    CFLAGS = $(CFLAGS_PROFILE)
    LDFLAGS = $(LDFLAGS_PROFILE)
else
    CFLAGS = $(CFLAGS_DEBUG)
    LDFLAGS = $(LDFLAGS_DEBUG)
endif

# Add user-defined flags
CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += $(EXTRA_LDFLAGS)

# Targets
.PHONY: all clean install uninstall test check format lint docs help debug release profile

# Default target
all: $(PROJECT_NAME)

# Create build directory
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(PROJECT_NAME): $(OBJECTS)
	@echo "LD $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@

# Build configurations
debug:
	@$(MAKE) BUILD_TYPE=debug

release:
	@$(MAKE) BUILD_TYPE=release

profile:
	@$(MAKE) BUILD_TYPE=profile

# Install
install: $(PROJECT_NAME)
	@echo "Installing $(PROJECT_NAME) to $(BINDIR)"
	@$(INSTALL) -d $(BINDIR)
	@$(INSTALL) -m 755 $(PROJECT_NAME) $(BINDIR)/
	@if [ -f $(DOCDIR)/$(PROJECT_NAME).1 ]; then \
		$(INSTALL) -d $(MANDIR); \
		$(INSTALL) -m 644 $(DOCDIR)/$(PROJECT_NAME).1 $(MANDIR)/; \
	fi

# Uninstall
uninstall:
	@echo "Removing $(PROJECT_NAME) from $(BINDIR)"
	@rm -f $(BINDIR)/$(PROJECT_NAME)
	@rm -f $(MANDIR)/$(PROJECT_NAME).1

# Clean
clean:
	@echo "Cleaning build files"
	@rm -rf $(OBJDIR)
	@rm -f $(PROJECT_NAME)
	@rm -f *.gcda *.gcno *.gcov
	@rm -f gmon.out
	@rm -f core core.*
	@rm -f vgcore.*

# Deep clean (including generated files)
distclean: clean
	@rm -rf $(DOCDIR)/html
	@rm -f $(DOCDIR)/*.pdf
	@rm -f tags TAGS
	@rm -f compile_commands.json

# Testing
test: $(PROJECT_NAME)
	@echo "Running tests..."
	@if [ -d $(TESTDIR) ]; then \
		$(MAKE) -C $(TESTDIR) test; \
	else \
		echo "No tests found"; \
	fi

# Static analysis
check: $(SOURCES)
	@echo "Running static analysis..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c11 --platform=unix64 \
		         --suppress=missingIncludeSystem \
		         --inline-suppr \
		         -I$(INCDIR) $(SOURCES); \
	else \
		echo "cppcheck not found, skipping static analysis"; \
	fi

# Code formatting
format: $(SOURCES) $(HEADERS)
	@echo "Formatting code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i -style="{BasedOnStyle: Linux, IndentWidth: 4, TabWidth: 4, UseTab: Never}" \
		             $(SOURCES) $(HEADERS); \
	else \
		echo "clang-format not found, skipping formatting"; \
	fi

# Linting
lint: $(SOURCES)
	@echo "Running linter..."
	@if command -v clang-tidy >/dev/null 2>&1; then \
		clang-tidy $(SOURCES) -- $(CFLAGS); \
	else \
		echo "clang-tidy not found, skipping linting"; \
	fi

# Memory leak detection
valgrind: $(PROJECT_NAME)
	@echo "Running Valgrind..."
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
		         --track-origins=yes --verbose ./$(PROJECT_NAME) --help; \
	else \
		echo "Valgrind not found"; \
	fi

# Generate documentation
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
	else \
		echo "Doxygen not found, skipping documentation generation"; \
	fi

# Generate compile commands for IDE support
compile_commands.json: $(SOURCES)
	@echo "Generating compile_commands.json..."
	@if command -v bear >/dev/null 2>&1; then \
		bear -- $(MAKE) clean all; \
	else \
		echo "bear not found, generating basic compile_commands.json"; \
		echo '[' > compile_commands.json; \
		for src in $(SOURCES); do \
			echo '  {' >> compile_commands.json; \
			echo '    "directory": "'$(PWD)'",' >> compile_commands.json; \
			echo '    "command": "$(CC) $(CFLAGS) -c '$$src'",' >> compile_commands.json; \
			echo '    "file": "'$$src'"' >> compile_commands.json; \
			echo '  },' >> compile_commands.json; \
		done; \
		sed -i '$$s/,$$//' compile_commands.json; \
		echo ']' >> compile_commands.json; \
	fi

# Package creation
package: clean
	@echo "Creating source package..."
	@tar -czf $(PROJECT_NAME)-$(VERSION).tar.gz \
		--exclude='.git*' \
		--exclude='*.tar.gz' \
		--exclude='build' \
		--transform 's,^,$(PROJECT_NAME)-$(VERSION)/,' \
		*

# Show build information
info:
	@echo "Build Information:"
	@echo "  Project: $(PROJECT_NAME) v$(VERSION)"
	@echo "  Build Type: $(BUILD_TYPE)"
	@echo "  CC: $(CC)"
	@echo "  CFLAGS: $(CFLAGS)"
	@echo "  LDFLAGS: $(LDFLAGS)"
	@echo "  LIBS: $(LIBS)"
	@echo "  Sources: $(SOURCES)"
	@echo "  Objects: $(OBJECTS)"

# Help
help:
	@echo "CWC Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all          Build the project (default)"
	@echo "  debug        Build with debug flags"
	@echo "  release      Build optimized release version"
	@echo "  profile      Build with profiling support"
	@echo "  clean        Remove build files"
	@echo "  distclean    Remove all generated files"
	@echo "  install      Install to system"
	@echo "  uninstall    Remove from system"
	@echo "  test         Run tests"
	@echo "  check        Run static analysis"
	@echo "  format       Format source code"
	@echo "  lint         Run linter"
	@echo "  valgrind     Run memory leak detection"
	@echo "  docs         Generate documentation"
	@echo "  package      Create source package"
	@echo "  info         Show build information"
	@echo "  help         Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_TYPE   Build type (debug, release, profile)"
	@echo "  PREFIX       Installation prefix (default: /usr/local)"
	@echo "  CC           C compiler (default: gcc)"
	@echo "  EXTRA_CFLAGS Additional compiler flags"
	@echo "  EXTRA_LDFLAGS Additional linker flags"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build debug version"
	@echo "  make release            # Build release version"
	@echo "  make BUILD_TYPE=release # Same as above"
	@echo "  make install PREFIX=/opt/cwc  # Install to /opt/cwc"
	@echo "  make EXTRA_CFLAGS=-DFEATURE=1  # Add custom flag"

# Dependencies
-include $(OBJECTS:.o=.d)

# Generate dependency files
$(OBJDIR)/%.d: $(SRCDIR)/%.c | $(OBJDIR)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< > $@
