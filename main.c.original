#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-server.h>
#include <wayland-util.h>
#include <xdg-shell-protocol.h>

#define wl_array_insert(array_p, elm) { \
	if (array_p->size + sizeof(elm) > alloc) \
		array_p->data = wl_array_add(array_p, array_p->alloc); \
	((typeof(elm) *)array_p->data)[array_p->size / sizeof(elm)] = elm; \
	array_p->size += sizeof(elm); \
}

/* Consise unsigned typedefs */
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

/*
 * Wayland interface implementations
 */
struct wl_state {
	struct wl_list outputs;
	struct wl_list compositors;
	struct wl_list shms;
};

struct shm {
	struct wl_list link;
	struct wl_resource *resource;
	struct wl_state *state;
	void *addr;
	int size;
};

struct output {
	struct wl_list link;
	struct wl_resource *resource;
	struct wl_state *state;
};

struct compositor {
	struct wl_list link;
	struct wl_resource *resource;
	struct wl_state *state;
};


/*
 * Global variables
 */
struct wl_state global_state = {
#define WL_LIST_HEAD(name) { .prev = &name, .next = &name, }
	.outputs = WL_LIST_HEAD(global_state.outputs),
	.compositors = WL_LIST_HEAD(global_state.compositors),
	.shms = WL_LIST_HEAD(global_state.shms),
};

static int log_fd = 1;
#define report(str) write(log_fd, str "\n", sizeof(str))
#define die(str) { report(str); exit(1); }


/*
 * Wayland shm implementation
 */
void
wl_shm_resource_destroy(struct wl_resource *resource)
{
	report("Shm resource destroyed");
	struct shm *shm = wl_resource_get_user_data(resource);
	munmap(shm->addr, shm->size);
	wl_list_remove(&resource->link);
}

void
wl_shm_create_pool(struct wl_client *client, struct wl_resource *resource,
		uint id, int fd, int size)
{
	uchar *mmap_addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

void
wl_shm_release(struct wl_client *client, struct wl_resource *resource)
{
	struct shm *client_shm = wl_resource_get_user_data(resource);
	munmap(client_shm->addr, client_shm->size);
}

struct wl_shm_interface wl_shm_implementation = {
	.create_pool = wl_shm_create_pool,
	.release = wl_shm_release,
};

void
wl_shm_bind_request(struct wl_client *client, void *data, uint version, uint id)
{
	report("Recieved request bind to shm");

	struct wl_state *state = data;
	struct shm *shm = malloc(sizeof(struct shm));

	struct wl_resource *rsrc = wl_resource_create(client,
			&wl_shm_interface, version, id);
	wl_resource_set_implementation(rsrc, &wl_shm_implementation,
			shm, wl_shm_resource_destroy);
	shm->resource = rsrc;
	shm->state = state;

	wl_list_insert(&state->shms, &rsrc->link);
}


/*
 * wl_output implementation functions
 */
void
wl_output_resource_destroy(struct wl_resource *resource)
{
	report("Output resource destroyed");
	//struct cwc_output *client_output = wl_resource_get_user_data(resource);
	/* TODO: Clean up resource */
	wl_list_remove(&resource->link); /* Don't free()!!! */
}

void
wl_output_release(struct wl_client *client, struct wl_resource *resource)
{
	report("Output release request");
	wl_resource_destroy(resource);
}

struct wl_output_interface wl_output_implementation = {
	.release = wl_output_release,
};

void
wl_output_bind_request(struct wl_client *client, void *data, uint version, uint id)
{
	report("Recieved request bind to output");

	struct wl_state *state = data;
	struct output *output = malloc(sizeof(struct output));

	struct wl_resource *rsrc = wl_resource_create(client,
			&wl_output_interface, wl_output_interface.version, id);
	wl_resource_set_implementation(rsrc, &wl_output_implementation,
			rsrc, wl_output_resource_destroy);
	output->resource = rsrc;
	output->state = state;

	wl_output_send_geometry(rsrc, 0, 0, 1920, 1080,
			WL_OUTPUT_SUBPIXEL_UNKNOWN, "Dell", "240hz Monitor",
			WL_OUTPUT_TRANSFORM_NORMAL);

	wl_list_insert(&state->outputs, &rsrc->link);
}


/*
 * wl_compositor implementation
 */
void
wl_compositor_resource_destroy(struct wl_resource *resource)
{
	report("Compositor resource destroyed");
	wl_list_remove(&resource->link);
}

void
wl_compositor_create_surface(struct wl_client *client,
		struct wl_resource *resource, uint id)
{
}

void
wl_compositor_create_region(struct wl_client *client,
		struct wl_resource *resource, uint id)
{
}

struct wl_compositor_interface wl_compositor_implementation = {
	.create_surface = wl_compositor_create_surface,
	.create_region = wl_compositor_create_region,
};

void
wl_compositor_bind_request(struct wl_client *client, void *data, uint version, uint id)
{
	report("Recieved request bind to compositor");

	struct wl_state *state = data;
	struct compositor *compositor = malloc(sizeof(struct compositor));

	struct wl_resource *rsrc = wl_resource_create(client,
			&wl_compositor_interface, wl_compositor_interface.version, id);
	wl_resource_set_implementation(rsrc, &wl_compositor_implementation,
			compositor, wl_compositor_resource_destroy);
	compositor->resource = rsrc;
	compositor->state = state;

	wl_list_insert(&state->compositors, &rsrc->link);
}


int
main(int argc, char *argv[])
{
	/* Open log file*/
	//log_fd = open("/home/basil/c/log", O_WRONLY | O_CREAT);
	//if (log_fd == -1)
	//	return 1;


	/* Initialise wayland display */
	struct wl_display *display = wl_display_create();
	if (display == NULL)
		die("Failed to create wayland display");

	/* Create UNIX socket */
	const char *socket = "wayland-1";
	int r = wl_display_add_socket(display, socket);
	if (r == -1)
		die("Fucking give up");

	/* TODO: Set up renderer and input devices */

	/* Get reference to event loop */
	struct wl_event_loop *event_loop = wl_display_get_event_loop(display);
	if (event_loop == NULL)
		die("Failed to obtain reference to the wl event loop");

	/* Create global objects */
	wl_display_init_shm(display);
	wl_display_add_shm_format(display, WL_SHM_FORMAT_ARGB8888);
	wl_display_add_shm_format(display, WL_SHM_FORMAT_XRGB8888);

	wl_global_create(display, &wl_compositor_interface, 6, &global_state, wl_compositor_bind_request);
	wl_global_create(display, &wl_output_interface, 1, &global_state, wl_output_bind_request);


	setenv("WAYLAND_DISPLAY", socket, 1);
	wl_display_run(display);


	/* Cleanup */
	wl_display_destroy(display);
	return 0;
}
