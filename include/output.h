#ifndef CWC_OUTPUT_H
#define CWC_OUTPUT_H

#include "cwc.h"

/* Output configuration */
struct cwc_output_config {
    int32_t x, y;
    int32_t width, height;
    int32_t physical_width, physical_height;
    int32_t refresh_rate;
    enum wl_output_subpixel subpixel;
    enum wl_output_transform transform;
    const char *make;
    const char *model;
};

/* Output state */
struct cwc_output {
    struct wl_list link;
    struct wl_resource *resource;
    struct cwc_server *server;
    
    /* Output configuration */
    struct cwc_output_config config;
    
    /* State tracking */
    bool enabled;
    time_t create_time;
};

/* Default output configurations */
extern const struct cwc_output_config cwc_default_output_config;

/* Function declarations */

/* Output interface */
void cwc_output_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id);
void cwc_output_release(struct wl_client *client, struct wl_resource *resource);

/* Output management */
struct cwc_output *cwc_output_create(struct wl_client *client, struct cwc_server *server, uint32_t id);
void cwc_output_destroy(struct cwc_output *output);
void cwc_output_send_geometry(struct cwc_output *output);
void cwc_output_send_mode(struct cwc_output *output);
void cwc_output_send_done(struct cwc_output *output);

/* Configuration */
void cwc_output_configure(struct cwc_output *output, const struct cwc_output_config *config);
bool cwc_output_config_validate(const struct cwc_output_config *config);

/* Resource cleanup */
void cwc_output_resource_destroy(struct wl_resource *resource);

#endif /* CWC_OUTPUT_H */
