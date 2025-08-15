#ifndef CWC_SHM_H
#define CWC_SHM_H

#include "cwc.h"

/* SHM pool state */
struct cwc_shm_pool {
    struct wl_list link;
    struct wl_resource *resource;
    struct cwc_server *server;
    
    /* Memory mapping */
    void *data;
    size_t size;
    int fd;
    
    /* Reference counting */
    int ref_count;
    
    /* Security limits */
    size_t max_size;
    time_t create_time;
};

/* SHM buffer */
struct cwc_shm_buffer {
    struct wl_list link;
    struct wl_resource *resource;
    struct cwc_shm_pool *pool;
    
    /* Buffer properties */
    int32_t offset;
    int32_t width, height;
    int32_t stride;
    uint32_t format;
    
    /* State */
    bool busy;
    time_t create_time;
};

/* SHM resource */
struct cwc_shm {
    struct wl_list link;
    struct wl_resource *resource;
    struct cwc_server *server;
    struct wl_list pools;  /* cwc_shm_pool::link */
};

/* Configuration constants */
#define CWC_SHM_MAX_POOL_SIZE (64 * 1024 * 1024)  /* 64MB */
#define CWC_SHM_MAX_POOLS_PER_CLIENT 10

/* Function declarations */

/* SHM interface */
void cwc_shm_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id);
void cwc_shm_create_pool(struct wl_client *client, struct wl_resource *resource,
                        uint32_t id, int32_t fd, int32_t size);

/* Pool management */
struct cwc_shm_pool *cwc_shm_pool_create(struct wl_client *client, struct cwc_shm *shm,
                                         uint32_t id, int fd, int32_t size);
void cwc_shm_pool_destroy(struct cwc_shm_pool *pool);
void cwc_shm_pool_resize(struct wl_client *client, struct wl_resource *resource, int32_t size);
void cwc_shm_pool_create_buffer(struct wl_client *client, struct wl_resource *resource,
                               uint32_t id, int32_t offset, int32_t width, int32_t height,
                               int32_t stride, uint32_t format);

/* Buffer management */
struct cwc_shm_buffer *cwc_shm_buffer_create(struct wl_client *client, struct cwc_shm_pool *pool,
                                            uint32_t id, int32_t offset, int32_t width, int32_t height,
                                            int32_t stride, uint32_t format);
void cwc_shm_buffer_destroy(struct cwc_shm_buffer *buffer);
void *cwc_shm_buffer_get_data(struct cwc_shm_buffer *buffer);

/* Validation functions */
bool cwc_shm_format_supported(uint32_t format);
bool cwc_shm_pool_validate_size(int32_t size);
bool cwc_shm_buffer_validate(int32_t offset, int32_t width, int32_t height,
                            int32_t stride, uint32_t format, size_t pool_size);

/* Resource cleanup */
void cwc_shm_resource_destroy(struct wl_resource *resource);
void cwc_shm_pool_resource_destroy(struct wl_resource *resource);
void cwc_shm_buffer_resource_destroy(struct wl_resource *resource);

/* Security functions */
bool cwc_shm_check_client_limits(struct wl_client *client, struct cwc_server *server);
void cwc_shm_cleanup_client_pools(struct wl_client *client, struct cwc_server *server);

#endif /* CWC_SHM_H */
