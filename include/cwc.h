#ifndef CWC_H
#define CWC_H

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-server.h>
#include <wayland-util.h>
#include <xdg-shell-protocol.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* Version information */
#define CWC_VERSION_MAJOR 1
#define CWC_VERSION_MINOR 0
#define CWC_VERSION_PATCH 0

/* Configuration constants */
#define CWC_MAX_CLIENTS 100
#define CWC_MAX_SURFACES 1000
#define CWC_DEFAULT_SOCKET "wayland-1"
#define CWC_LOG_BUFFER_SIZE 1024

/* Concise unsigned typedefs */
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

/* Forward declarations */
struct cwc_server;
struct cwc_output;
struct cwc_surface;
struct cwc_client_state;

/* Error codes */
typedef enum {
    CWC_SUCCESS = 0,
    CWC_ERROR_MEMORY = -1,
    CWC_ERROR_DISPLAY = -2,
    CWC_ERROR_SOCKET = -3,
    CWC_ERROR_RESOURCE = -4,
    CWC_ERROR_INVALID_PARAM = -5
} cwc_error_t;

/* Logging levels */
typedef enum {
    CWC_LOG_ERROR = 0,
    CWC_LOG_WARN = 1,
    CWC_LOG_INFO = 2,
    CWC_LOG_DEBUG = 3
} cwc_log_level_t;

/* Main server state */
struct cwc_server {
    struct wl_display *display;
    struct wl_event_loop *event_loop;
    const char *socket_name;
    
    /* Global objects */
    struct wl_global *compositor_global;
    struct wl_global *output_global;
    
    /* Resource lists */
    struct wl_list outputs;      /* cwc_output::link */
    struct wl_list surfaces;     /* cwc_surface::link */
    struct wl_list clients;      /* cwc_client_state::link */
    
    /* Configuration */
    bool debug_mode;
    int log_fd;
    cwc_log_level_t log_level;
    
    /* Statistics */
    uint32_t client_count;
    uint32_t surface_count;
    time_t start_time;
};

/* Client state tracking */
struct cwc_client_state {
    struct wl_list link;
    struct wl_client *client;
    struct cwc_server *server;
    uint32_t surface_count;
    time_t connect_time;
};

/* Function declarations */

/* Core server functions */
cwc_error_t cwc_server_init(struct cwc_server *server, const char *socket_name);
void cwc_server_destroy(struct cwc_server *server);
cwc_error_t cwc_server_run(struct cwc_server *server);

/* Logging functions */
void cwc_log(struct cwc_server *server, cwc_log_level_t level, const char *format, ...);
void cwc_log_init(struct cwc_server *server, const char *log_file);
void cwc_log_cleanup(struct cwc_server *server);

/* Utility functions */
const char *cwc_error_string(cwc_error_t error);
void cwc_print_version(void);
void cwc_print_usage(const char *program_name);

/* Memory management helpers */
void *cwc_malloc(size_t size);
void *cwc_calloc(size_t nmemb, size_t size);
void cwc_free(void *ptr);

/* Client management */
void cwc_client_state_create(struct wl_client *client, struct cwc_server *server);
void cwc_client_state_destroy(struct cwc_client_state *client_state);

#endif /* CWC_H */
