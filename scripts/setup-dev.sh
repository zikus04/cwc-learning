#!/bin/bash
# Development environment setup script for CWC

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt-get >/dev/null 2>&1; then
            OS="ubuntu"
        elif command -v dnf >/dev/null 2>&1; then
            OS="fedora"
        elif command -v pacman >/dev/null 2>&1; then
            OS="arch"
        else
            OS="linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    else
        OS="unknown"
    fi
    log_info "Detected OS: $OS"
}

# Install dependencies based on OS
install_dependencies() {
    log_info "Installing dependencies..."
    
    case $OS in
        ubuntu)
            sudo apt-get update
            sudo apt-get install -y \
                build-essential \
                pkg-config \
                libwayland-dev \
                wayland-protocols \
                libxkbcommon-dev \
                git \
                clang \
                clang-format \
                clang-tidy \
                cppcheck \
                valgrind \
                doxygen \
                graphviz \
                bear \
                lcov
            ;;
        fedora)
            sudo dnf install -y \
                gcc \
                make \
                pkgconfig \
                wayland-devel \
                wayland-protocols-devel \
                libxkbcommon-devel \
                git \
                clang \
                clang-tools-extra \
                cppcheck \
                valgrind \
                doxygen \
                graphviz \
                bear \
                lcov
            ;;
        arch)
            sudo pacman -S --needed \
                base-devel \
                pkgconfig \
                wayland \
                wayland-protocols \
                libxkbcommon \
                git \
                clang \
                cppcheck \
                valgrind \
                doxygen \
                graphviz \
                bear \
                lcov
            ;;
        macos)
            if ! command -v brew >/dev/null 2>&1; then
                log_error "Homebrew not found. Please install Homebrew first."
                exit 1
            fi
            brew install \
                pkg-config \
                wayland \
                wayland-protocols \
                libxkbcommon \
                clang-format \
                cppcheck \
                doxygen \
                graphviz \
                bear \
                lcov
            ;;
        *)
            log_error "Unsupported OS: $OS"
            log_info "Please install the following packages manually:"
            log_info "- build tools (gcc/clang, make)"
            log_info "- pkg-config"
            log_info "- wayland development libraries"
            log_info "- wayland-protocols"
            log_info "- libxkbcommon"
            exit 1
            ;;
    esac
    
    log_success "Dependencies installed successfully"
}

# Setup Git hooks
setup_git_hooks() {
    log_info "Setting up Git hooks..."
    
    # Pre-commit hook
    cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Pre-commit hook for CWC

set -e

echo "Running pre-commit checks..."

# Check code formatting
if ! make format >/dev/null 2>&1; then
    echo "Error: Code formatting failed"
    exit 1
fi

# Check if there are formatting changes
if ! git diff --exit-code >/dev/null 2>&1; then
    echo "Error: Code formatting changes detected. Please commit the formatting changes."
    exit 1
fi

# Run static analysis
if ! make check >/dev/null 2>&1; then
    echo "Error: Static analysis failed"
    exit 1
fi

echo "Pre-commit checks passed"
EOF

    chmod +x .git/hooks/pre-commit
    log_success "Git hooks setup complete"
}

# Setup IDE configuration
setup_ide_config() {
    log_info "Setting up IDE configuration..."
    
    # Generate compile_commands.json for IDE support
    if command -v bear >/dev/null 2>&1; then
        make compile_commands.json
        log_success "Generated compile_commands.json for IDE support"
    else
        log_warning "bear not found, skipping compile_commands.json generation"
    fi
    
    # Create .vscode directory with basic configuration
    mkdir -p .vscode
    
    # VS Code settings
    cat > .vscode/settings.json << 'EOF'
{
    "C_Cpp.default.compileCommands": "${workspaceFolder}/compile_commands.json",
    "C_Cpp.default.cStandard": "c11",
    "C_Cpp.default.intelliSenseMode": "gcc-x64",
    "files.associations": {
        "*.h": "c",
        "*.c": "c"
    },
    "editor.formatOnSave": true,
    "editor.tabSize": 4,
    "editor.insertSpaces": true,
    "C_Cpp.clang_format_style": "{BasedOnStyle: Linux, IndentWidth: 4, TabWidth: 4, UseTab: Never}",
    "files.exclude": {
        "build/": true,
        "*.o": true,
        "*.gcda": true,
        "*.gcno": true
    }
}
EOF

    # VS Code tasks
    cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "make",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "clean",
            "type": "shell",
            "command": "make",
            "args": ["clean"],
            "group": "build"
        },
        {
            "label": "test",
            "type": "shell",
            "command": "make",
            "args": ["test"],
            "group": "test"
        },
        {
            "label": "format",
            "type": "shell",
            "command": "make",
            "args": ["format"]
        }
    ]
}
EOF

    # VS Code launch configuration
    cat > .vscode/launch.json << 'EOF'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug CWC",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/cwc",
            "args": ["--debug"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build"
        }
    ]
}
EOF

    log_success "IDE configuration setup complete"
}

# Create development documentation
create_dev_docs() {
    log_info "Creating development documentation..."
    
    cat > CONTRIBUTING.md << 'EOF'
# Contributing to CWC

Thank you for your interest in contributing to CWC!

## Development Setup

Run the development setup script:
```bash
./scripts/setup-dev.sh
```

## Building

```bash
# Debug build (default)
make

# Release build
make release

# With custom flags
make EXTRA_CFLAGS="-DFEATURE=1"
```

## Code Style

- Use 4 spaces for indentation
- Follow Linux kernel coding style
- Run `make format` before committing
- All functions must have documentation comments

## Testing

```bash
# Run tests
make test

# Memory leak check
make valgrind

# Static analysis
make check
```

## Submitting Changes

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests and formatting
5. Submit a pull request

## Code Review Process

All submissions require review. We use GitHub pull requests for this purpose.
EOF

    log_success "Development documentation created"
}

# Main setup function
main() {
    log_info "Starting CWC development environment setup..."
    
    # Check if we're in the right directory
    if [[ ! -f "Makefile" ]] || [[ ! -d "src" ]]; then
        log_error "Please run this script from the CWC project root directory"
        exit 1
    fi
    
    detect_os
    install_dependencies
    setup_git_hooks
    setup_ide_config
    create_dev_docs
    
    log_success "Development environment setup complete!"
    log_info "You can now build the project with: make"
    log_info "For help, run: make help"
}

# Run main function
main "$@"
