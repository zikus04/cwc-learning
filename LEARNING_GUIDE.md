# CWC Learning Guide

This guide will help you understand the CWC (Custom Wayland Compositor) project and learn modern C programming practices.

## 🎯 What You'll Learn

### 1. **Modern C Programming**
- **Memory Management**: Safe allocation/deallocation patterns
- **Error Handling**: Comprehensive error checking and reporting
- **Code Organization**: Modular design with headers and source files
- **Security**: Buffer overflow protection, input validation
- **Standards Compliance**: C11 standard features

### 2. **Build Systems & DevOps**
- **Modern Makefiles**: Advanced build configurations
- **Compiler Flags**: Security hardening, optimization levels
- **Static Analysis**: Code quality tools (cppcheck, clang-tidy)
- **CI/CD**: Automated testing and deployment
- **Package Management**: Creating distributable packages

### 3. **System Programming**
- **Wayland Protocol**: Modern Linux graphics system
- **Event-Driven Programming**: Handling asynchronous events
- **IPC**: Inter-process communication
- **Resource Management**: File descriptors, memory mapping

### 4. **Software Engineering Practices**
- **Version Control**: Git workflows and hooks
- **Documentation**: README, API docs, man pages
- **Testing**: Unit tests, integration tests, memory leak detection
- **Code Quality**: Formatting, linting, static analysis

## 🚀 Getting Started

### Step 1: Environment Setup
```bash
# Run the development setup script
./scripts/setup-dev.sh

# This will install dependencies and configure your development environment
```

### Step 2: Build the Project
```bash
# Debug build (recommended for learning)
make debug

# Release build (optimized)
make release

# Get build information
make info
```

### Step 3: Explore the Code
Start with these files in order:
1. `include/cwc.h` - Main data structures and function declarations
2. `src/main.c` - Program entry point and command-line handling
3. `Makefile` - Build system configuration
4. `README.md` - Project overview and usage

## 📚 Key Concepts Explained

### Memory Safety
The original code had several memory safety issues:
- **Memory leaks**: Missing `free()` calls
- **Buffer overflows**: No bounds checking
- **Use after free**: Accessing freed memory

**Our improvements:**
```c
// Safe memory allocation with error checking
void *cwc_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr && size > 0) {
        fprintf(stderr, "Fatal: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Always check for NULL before freeing
void cwc_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}
```

### Error Handling
Instead of ignoring errors or using generic error messages:
```c
// Define specific error types
typedef enum {
    CWC_SUCCESS = 0,
    CWC_ERROR_MEMORY = -1,
    CWC_ERROR_DISPLAY = -2,
    CWC_ERROR_SOCKET = -3
} cwc_error_t;

// Provide meaningful error messages
const char *cwc_error_string(cwc_error_t error) {
    switch (error) {
        case CWC_SUCCESS: return "Success";
        case CWC_ERROR_MEMORY: return "Memory allocation failed";
        // ... more cases
    }
}
```

### Logging System
Professional logging with levels and timestamps:
```c
void cwc_log(struct cwc_server *server, cwc_log_level_t level, 
             const char *format, ...) {
    // Timestamp, log level, formatted message
    // Configurable output (stdout, file)
    // Immediate flush for errors
}
```

### Security Hardening
Compiler flags that protect against common vulnerabilities:
```makefile
CFLAGS_SECURITY = -fstack-protector-strong \
                  -fPIE \
                  -D_FORTIFY_SOURCE=2 \
                  -Wformat-security
```

## 🔧 Development Workflow

### 1. Making Changes
```bash
# Create a feature branch
git checkout -b feature/my-improvement

# Make your changes
# Edit files...

# Format code
make format

# Check for issues
make check
make lint

# Build and test
make debug
make test
```

### 2. Code Quality Checks
```bash
# Static analysis
make check

# Memory leak detection
make valgrind

# Code formatting
make format

# Generate documentation
make docs
```

### 3. Debugging
```bash
# Build with debug symbols
make debug

# Run with debugger
gdb ./cwc

# Or use your IDE's debugger with the provided .vscode/launch.json
```

## 🎓 Learning Exercises

### Beginner Level
1. **Add a new command-line option**
   - Add `--version-info` that shows detailed version information
   - Modify `main.c` and update the help text

2. **Improve logging**
   - Add a log rotation feature
   - Add colored output for different log levels

3. **Add configuration**
   - Create a simple config file parser
   - Allow setting default socket name via config

### Intermediate Level
1. **Add metrics collection**
   - Track client connections over time
   - Add memory usage monitoring
   - Create a status command

2. **Implement signal handling**
   - Add SIGUSR1 handler for log level changes
   - Add SIGHUP handler for config reload

3. **Add unit tests**
   - Create a test framework
   - Test error handling functions
   - Test memory allocation functions

### Advanced Level
1. **Add surface management**
   - Implement the missing surface creation functions
   - Add surface damage tracking
   - Implement buffer management

2. **Add input handling**
   - Keyboard input processing
   - Mouse input processing
   - Touch input support

3. **Performance optimization**
   - Profile the application
   - Optimize memory usage
   - Add multi-threading support

## 🐛 Common Issues & Solutions

### Build Issues
```bash
# Missing dependencies
./scripts/setup-dev.sh

# Clean build
make clean && make

# Check build configuration
make info
```

### Runtime Issues
```bash
# Enable debug logging
./cwc --debug

# Check for memory leaks
make valgrind

# Run with strace to see system calls
strace ./cwc
```

### Code Quality Issues
```bash
# Fix formatting
make format

# Check for common issues
make check

# Run linter
make lint
```

## 📖 Additional Resources

### Wayland Development
- [Wayland Book](https://wayland-book.com/) - Comprehensive Wayland guide
- [Wayland Protocol](https://wayland.freedesktop.org/docs/html/) - Official protocol documentation
- [wlroots](https://github.com/swaywm/wlroots) - Wayland compositor library

### C Programming
- [Modern C](https://modernc.gforge.inria.fr/) - Modern C programming practices
- [C11 Standard](https://www.iso.org/standard/57853.html) - Official C11 specification
- [Secure Coding in C](https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard) - Security guidelines

### Build Systems
- [GNU Make Manual](https://www.gnu.org/software/make/manual/) - Complete Make documentation
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/) - Alternative build system
- [Autotools](https://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html) - Traditional Unix build system

### Development Tools
- [GDB Tutorial](https://www.gnu.org/software/gdb/documentation/) - Debugging with GDB
- [Valgrind Manual](https://valgrind.org/docs/manual/) - Memory error detection
- [Git Best Practices](https://git-scm.com/book) - Version control workflows

## 🤝 Getting Help

1. **Read the documentation** - Start with README.md and this guide
2. **Check the issues** - Look for similar problems in GitHub issues
3. **Ask questions** - Create a GitHub issue with the "question" label
4. **Join the community** - Participate in discussions

## 🎉 Next Steps

Once you're comfortable with this project:

1. **Contribute back** - Submit improvements via pull requests
2. **Create your own project** - Apply these patterns to new projects
3. **Learn more advanced topics** - Graphics programming, kernel development
4. **Share your knowledge** - Help others learn these concepts

Remember: The goal isn't just to make the code work, but to understand *why* these practices matter and how they make software more reliable, secure, and maintainable.
