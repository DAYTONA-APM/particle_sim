#include "simulator.h"
#include <stdlib.h>
#include <stdio.h>

int sim_world_init(SimWorld *world, size_t initial_capacity,
		float width, float height, float cell_size)
{
	if (world == NULL || initial_capacity == 0 || cell_size <= 0.0f) {
		fprintf(stderr, "sim_world_init: invalid params\n");
		return FALSE;
	}
		
	world->capacity = initial_capacity;
	world->count = 0;
	world->bounds_width = width;
	world->bounds_height = height;
	world->substeps = 8; /* good for preventing jitter */
	world->gravity.x = 0.0f; 
	world->gravity.y = 980.0f; /* grav * 100 to stop slomo*/

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

/*
 * TODO: Implement sim_world_step(SimWorld *world, float dt)
 *
 * 1. INPUT VALIDATION:
 *    - Check for NULL world, zero particles (world->count == 0), or non-positive dt.
 *    - Determine substep count (guard against world->substeps <= 0, fallback to 1).
 *
 * 2. SUB-STEP DELTA TIME:
 *    - Compute sub_dt = dt / (float)substeps.
 *    - Precompute sub_dt_squared = sub_dt * sub_dt.
 *
 * 3. SUB-STEP LOOP (repeat 'substeps' times):
 *    - Step A: Call sim_world_apply_gravity(world)
 *    - Step B: Verlet Integration across all particles (0 to world->count - 1):
 *        * Calculate implicit velocity: vel = pos - prev_pos
 *        * Stash current pos in a temp variable
 *        * Update pos = pos + vel + accel * sub_dt_squared
 *        * Update prev_pos = temp
 *        * Reset accel = {0.0f, 0.0f}
 *    - Step C: Call sim_grid_rebuild(world)
 *    - Step D: Call sim_solve_collisions(world)
 *    - Step E: Call sim_solve_boundaries(world)
 */
void sim_world_step(SimWorld *world, float dt)
{
	float sub_dt;
	float sub_dt_sq;
	int i; 
	size_t j;
	float vel_x;
	float vel_y;
	float curr_x;
	float curr_y;
	Particle *p;

    if (!world || world->count == 0 || dt <= 0.0f) {
		fprintf(stderr, "invalid input\n");
		return;
	}

	sub_dt = dt / (float)world->substeps;
	sub_dt_sq = sub_dt * sub_dt;

	for (i = 0; i < world->substeps; i++) {
		sim_world_apply_gravity(world); /* step A */

		for (j = 0; j < world->count-1; j++) {
			p = &world->particles[j];

			vel_x = p->pos.x - p->prev_pos.x;
			vel_y = p->pos.y - p->prev_pos.y;

			curr_x = p->pos.x;
            curr_y = p->pos.y;

			p->pos.x += vel_x + p->accel.x * sub_dt_sq;
            p->pos.y += vel_y + p->accel.y * sub_dt_sq;

            p->prev_pos.x = curr_x;
            p->prev_pos.y = curr_y;

            p->accel.x = 0.0f;
            p->accel.y = 0.0f;		
		}

		sim_grid_rebuild(world);
		sim_solve_collisions(world);
		sim_solve_boundaries(world);
	}

}

int sim_world_add_particle(SimWorld *world, vector pos, vector initial_velocity,
                           float radius, float mass, uint32_t color)
{
    /* TODO: Add particle to world->particles, reallocating if count == capacity */
    (void)world;
    (void)pos;
    (void)initial_velocity;
    (void)radius;
    (void)mass;
    (void)color;
    return -1;
}

void sim_world_apply_gravity(SimWorld *world)
{
    /* TODO: Accumulate world->gravity into each particle's accel */
    (void)world;
}

void sim_grid_rebuild(SimWorld *world)
{
    /* TODO: Re-index particles into grid buckets */
    (void)world;
}

void sim_solve_collisions(SimWorld *world)
{
    /* TODO: Detect and resolve overlaps */
    (void)world;
}

void sim_solve_boundaries(SimWorld *world)
{
    /* TODO: Clamp particles to container edges */
    (void)world;
}
