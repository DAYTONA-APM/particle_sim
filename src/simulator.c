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

/* goal: store particles in memroy, for the world to simulate
 *
 * check if world->count == world->capacity;
 * 		if full, realloc world->particles and world->grid.particle_next
 *
 * calc prev_pos using initial velocity and a nominal timestep so Verlet
 * integration starts with that velocity
 *
 * initialise particle properties & increment world count
 *
 * return the assigned particle index */

int sim_world_add_particle(SimWorld *world, vector pos, vector initial_velocity,
                           float radius, float mass, uint32_t color)
{    
	Particle *temp_part;
	int *temp_next;
	Particle new_part;
	float nominal_dt;
	float sub_dt;
	int substeps;
	size_t new_capacity;


	if (!world)  {
		fprintf(stderr, "invalid input values\n");
		return -1;
	}

	if (world->count == world->capacity) {
		new_capacity = world->capacity * 2;
		temp_part = realloc(world->particles, sizeof(Particle) * new_capacity);
		if (!temp_part) {
			fprintf(stderr, "unable to resize particle array\n");
			return FALSE;
		}
		world->particles = temp_part;

		temp_next = realloc(world->grid.particle_next,
			   	sizeof(int) * new_capacity);
		if (!temp_next) {
			fprintf(stderr, "unable to resize adjacent particle memory \n");
			return FALSE;
		}
		world->grid.particle_next = temp_next;
		world->capacity = new_capacity;
	}	

	/* calculation of prev_pos */
	nominal_dt = (1.0f/ 60.0f);
	substeps = (world->substeps > 0) ? world->substeps : 1;
	sub_dt = nominal_dt / (float)substeps;

	new_part.prev_pos.x = pos.x - initial_velocity.x * sub_dt;
	new_part.prev_pos.y = pos.y - initial_velocity.y * sub_dt;

	/* filling out the rest of the attributes */
	new_part.pos.x = pos.x;
	new_part.pos.y = pos.y;

	new_part.radius = radius;
	new_part.mass = mass;
	new_part.color = color;

	new_part.accel.x = 0.0f;
	new_part.accel.y = 0.0f;

	/* integrate into particle array */
	world->particles[world->count] = new_part;
	world->count++;

    return world->count-1; /* index of new particle */
}

/* goal: accumulate downward force for each substep
 *
 * loop over all particles --> 0 < i < world->count
 * add world->gravity to each particle's acceleration
 */
void sim_world_apply_gravity(SimWorld *world)
{
	size_t j; /* account for count of all particles */
	
    /* TODO: Accumulate world->gravity into each particle's accel */
    if (!world || world->count == 0) {
		fprintf(stderr, "unable to apply gravity\n");
		return;
	}

	/* only one loop needed, the substeps applies the data world->substeps
	 * times. aapplying it in a loop is a double step */
	for (j = 0; j < world->count; j++) {
		world->particles[j].accel.x += world->gravity.x;
		world->particles[j].accel.y += world->gravity.y;
	}
		
}

/* hash particles to the spatial grid
 *
 * reset world->grid.cell_heads to -1 (use memset)
 *
 * for each particle, calc the int cell coord(col, row) based on the cell size
 *
 * clamp (col, row) so particles on the edge don't cause an out of bounds cell
 * idx
 *
 * cell_idx = col + row * grid_cols
 */
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
