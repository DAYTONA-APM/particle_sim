#include "simulator.h"
#include <stdlib.h>
#include <stdio.h>

int sim_world_init(SimWorld *world, size_t intial_capacity,
		float width, float height, float cell_size)
{
	if (world == NULL || iniial_capacity == 0 || cell_size <= 0.0f) {
		fprintf(stderr, "sim_world_init: invalid params\n");
		return FALSE
	}
		
	world->capacity = initial_capacity;
	world->count = 0;
	world->bounds_width = width;
	world->bounds_height = height;
	world->substeps = 8; /* good for preventing jitter */
	world->gravity = (vector){0.0f, 980.0f}; /* grav * 100 to stop slomo*/

	/* allocate the particle buffer */
	world->particles = malloc(sizeof(Particle) * initial_capacity);
	if (!world->particles) {
		fprintf(stderr, "particle capacity could not be allocated\n");
		return FALSE;
	}

	/* config the spatial grid metadata */
	world->grid.cell_size = cell_size;
	world->grid.cols = (int)(width / cell_size) + 1;
	world->grid.rows = (int)(height / cell_size) + 1;
	world->grid.total_cells = (size_t)(world->grid.rows * world->grid.cols);

	world->grid.cell_heads = malloc(sizeof(int) * world->grid.total_cells);
	if (!world->grid.cell_heads) {
		fprintf(stderr, "unable to allocate cell heads\n");
		free(world->particles);
		world->particles = NULL;
		return FALSE;
	}

	/* allocation for the linked list arr */
	world->grid.particle_next = malloc(sizeof(int) * initial_capacity);
	if (!world->grid.particle_next) {
		fprintf(stderr, "unable to allocate for adjacent particles\n");
		free(world->grid.particle_next);
		world->grid.particle_next = NULL;
		free(world->particles);
		world->particles = NULL;
		return FALSE;
	}

					

	return TRUE;
}


void sim_world_destroy(SimWorld *world)
{
	if (!world)
		return;

	free(world->particles);
	free(world->grid.cell_heads);
	free(world->grid.particle_next);

	world->count = 0;
	world->capacity = 0;
	world->grid.total_cells = 0;

	world->particles = NULL;
    world->grid.cell_heads = NULL;
    world->grid.particle_next = NULL;

}
