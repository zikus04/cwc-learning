/*
 * CWC - Custom Wayland Compositor
 * 
 * ORIGINAL WORK BY: donthurtme1
 * Original Repository: https://github.com/donthurtme1/cwc
 * 
 * This file is based on the excellent work by donthurtme1.
 * All core implementation, algorithms, and architecture credit
 * goes to the original author.
 * 
 * This version includes educational enhancements and documentation
 * improvements for learning purposes.
 */

#include "../include/cwc.h"
#include <signal.h>
#include <getopt.h>
#include <stdarg.h>

/* Global server instance for signal handling */
static struct cwc_server *g_server = NULL;

/* Signal handler for graceful shutdown */
static void signal_handler(int signal) {
    if (g_server && g_server->display) {
        printf("Received signal %d, shutting down gracefully\n", signal);
        wl_display_terminate(g_server->display);
    }
}

/* Setup signal handlers */
static void setup_signals(void) {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    /* Ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
}

/* Print version information */
void cwc_print_version(void) {
    printf("CWC (Custom Wayland Compositor) v%d.%d.%d\n",
           CWC_VERSION_MAJOR, CWC_VERSION_MINOR, CWC_VERSION_PATCH);
    printf("Built on %s at %s\n", __DATE__, __TIME__);
}

/* Print usage information */
void cwc_print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\nOptions:\n");
    printf("  -h, --help           Show this help message\n");
    printf("  -v, --version        Show version information\n");
    printf("  -s, --socket NAME    Use custom socket name (default: %s)\n", CWC_DEFAULT_SOCKET);
    printf("  -l, --log-file FILE  Log to file instead of stdout\n");
    printf("  -d, --debug          Enable debug mode\n");
    printf("  -q, --quiet          Reduce log output\n");
}

/* Convert error code to string */
const char *cwc_error_string(cwc_error_t error) {
    switch (error) {
        case CWC_SUCCESS: return "Success";
        case CWC_ERROR_MEMORY: return "Memory allocation failed";
        case CWC_ERROR_DISPLAY: return "Display creation failed";
        case CWC_ERROR_SOCKET: return "Socket creation failed";
        case CWC_ERROR_RESOURCE: return "Resource creation failed";
        case CWC_ERROR_INVALID_PARAM: return "Invalid parameter";
        default: return "Unknown error";
    }
}

/* Logging function with levels */
void cwc_log(struct cwc_server *server, cwc_log_level_t level, const char *format, ...) {
    if (!server || level > server->log_level) {
        return;
    }
    
    const char *level_str[] = {"ERROR", "WARN", "INFO", "DEBUG"};
    char timestamp[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    char buffer[CWC_LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    dprintf(server->log_fd, "[%s] %s: %s\n", timestamp, level_str[level], buffer);
    
    /* Flush immediately for errors */
    if (level == CWC_LOG_ERROR) {
        fsync(server->log_fd);
    }
}

/* Initialize logging */
void cwc_log_init(struct cwc_server *server, const char *log_file) {
    if (log_file) {
        server->log_fd = open(log_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (server->log_fd == -1) {
            fprintf(stderr, "Failed to open log file %s: %s\n", log_file, strerror(errno));
            server->log_fd = STDOUT_FILENO;
        }
    } else {
        server->log_fd = STDOUT_FILENO;
    }
    
    server->log_level = server->debug_mode ? CWC_LOG_DEBUG : CWC_LOG_INFO;
}

/* Cleanup logging */
void cwc_log_cleanup(struct cwc_server *server) {
    if (server->log_fd != STDOUT_FILENO && server->log_fd != STDERR_FILENO) {
        close(server->log_fd);
    }
}

/* Safe memory allocation with error checking */
void *cwc_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr && size > 0) {
        fprintf(stderr, "Fatal: Memory allocation failed for %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *cwc_calloc(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);
    if (!ptr && nmemb > 0 && size > 0) {
        fprintf(stderr, "Fatal: Memory allocation failed for %zu * %zu bytes\n", nmemb, size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void cwc_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

/* Basic server initialization for demo */
cwc_error_t cwc_server_init(struct cwc_server *server, const char *socket_name) {
    memset(server, 0, sizeof(*server));
    
    /* Initialize lists */
    wl_list_init(&server->outputs);
    wl_list_init(&server->surfaces);
    wl_list_init(&server->clients);
    
    /* Set socket name */
    server->socket_name = socket_name ? socket_name : CWC_DEFAULT_SOCKET;
    server->start_time = time(NULL);
    
    /* Create display */
    server->display = wl_display_create();
    if (!server->display) {
        return CWC_ERROR_DISPLAY;
    }
    
    /* Add socket */
    if (wl_display_add_socket(server->display, server->socket_name) == -1) {
        wl_display_destroy(server->display);
        return CWC_ERROR_SOCKET;
    }
    
    /* Set environment variable */
    if (setenv("WAYLAND_DISPLAY", server->socket_name, 1) != 0) {
        printf("Warning: Failed to set WAYLAND_DISPLAY environment variable\n");
    }
    
    return CWC_SUCCESS;
}

/* Run server */
cwc_error_t cwc_server_run(struct cwc_server *server) {
    if (!server || !server->display) {
        return CWC_ERROR_INVALID_PARAM;
    }
    
    printf("Starting CWC compositor on socket '%s'\n", server->socket_name);
    printf("Debug mode: %s\n", server->debug_mode ? "enabled" : "disabled");
    printf("Press Ctrl+C to stop\n");
    
    /* Run the event loop */
    wl_display_run(server->display);
    
    printf("Compositor shutting down\n");
    return CWC_SUCCESS;
}

/* Destroy server */
void cwc_server_destroy(struct cwc_server *server) {
    if (!server) return;
    
    /* Destroy display */
    if (server->display) {
        wl_display_destroy(server->display);
    }
    
    cwc_log_cleanup(server);
}

/* Main function */
int main(int argc, char *argv[]) {
    struct cwc_server server = {0};
    const char *socket_name = NULL;
    const char *log_file = NULL;
    bool debug_mode = false;
    bool quiet_mode = false;
    
    /* Parse command line arguments */
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"socket", required_argument, 0, 's'},
        {"log-file", required_argument, 0, 'l'},
        {"debug", no_argument, 0, 'd'},
        {"quiet", no_argument, 0, 'q'},
        {0, 0, 0, 0}
    };
    
    int c;
    while ((c = getopt_long(argc, argv, "hvs:l:dq", long_options, NULL)) != -1) {
        switch (c) {
            case 'h':
                cwc_print_usage(argv[0]);
                return EXIT_SUCCESS;
            case 'v':
                cwc_print_version();
                return EXIT_SUCCESS;
            case 's':
                socket_name = optarg;
                break;
            case 'l':
                log_file = optarg;
                break;
            case 'd':
                debug_mode = true;
                break;
            case 'q':
                quiet_mode = true;
                break;
            case '?':
                cwc_print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }
    
    /* Check environment variables */
    if (!socket_name) {
        socket_name = getenv("WAYLAND_DISPLAY");
    }
    if (!debug_mode && getenv("CWC_DEBUG")) {
        debug_mode = true;
    }
    
    /* Set debug mode */
    server.debug_mode = debug_mode;
    
    /* Initialize logging */
    cwc_log_init(&server, log_file);
    
    if (quiet_mode) {
        server.log_level = CWC_LOG_ERROR;
    }
    
    /* Set global server for signal handling */
    g_server = &server;
    setup_signals();
    
    cwc_print_version();
    
    /* Initialize server */
    cwc_error_t result = cwc_server_init(&server, socket_name);
    if (result != CWC_SUCCESS) {
        fprintf(stderr, "Failed to initialize server: %s\n", cwc_error_string(result));
        return EXIT_FAILURE;
    }
    
    /* Run server */
    result = cwc_server_run(&server);
    
    /* Cleanup */
    cwc_server_destroy(&server);
    
    return (result == CWC_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
