#ifndef CWC_COMPOSITOR_H
#define CWC_COMPOSITOR_H

#include "cwc.h"

/* Surface state */
struct cwc_surface {
    struct wl_list link;
    struct wl_resource *resource;
    struct cwc_server *server;
    struct cwc_client_state *client_state;
    
    /* Surface properties */
    int32_t x, y;
    int32_t width, height;
    bool mapped;
    
    /* Buffer management */
    struct wl_resource *buffer;
    void *buffer_data;
    size_t buffer_size;
    
    /* Damage tracking */
    struct wl_list damage_list;
    
    /* Creation time for debugging */
    time_t create_time;
};

/* Compositor resource */
struct cwc_compositor {
    struct wl_list link;
    struct wl_resource *resource;
    struct cwc_server *server;
};

/* Function declarations */

/* Compositor interface */
void cwc_compositor_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id);
void cwc_compositor_create_surface(struct wl_client *client, struct wl_resource *resource, uint32_t id);
void cwc_compositor_create_region(struct wl_client *client, struct wl_resource *resource, uint32_t id);

/* Surface management */
struct cwc_surface *cwc_surface_create(struct wl_client *client, struct cwc_server *server, uint32_t id);
void cwc_surface_destroy(struct cwc_surface *surface);
void cwc_surface_commit(struct wl_client *client, struct wl_resource *resource);
void cwc_surface_attach(struct wl_client *client, struct wl_resource *resource, 
                       struct wl_resource *buffer, int32_t x, int32_t y);

/* Resource cleanup */
void cwc_compositor_resource_destroy(struct wl_resource *resource);
void cwc_surface_resource_destroy(struct wl_resource *resource);

/* Validation functions */
bool cwc_surface_validate(struct cwc_surface *surface);
bool cwc_buffer_validate(struct wl_resource *buffer);

#endif /* CWC_COMPOSITOR_H */
