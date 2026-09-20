#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef struct {
	float x;
	float y;
} vector;

typedef struct {
	vector pos;
	vector prev_pos;
	vector accel;
	float radius;
	float mass;
	uint32_t color;
} Particle;

/*
 * cell size = width and height of all square cells in pixels
 *
 * cols & rows = number of grid divisions horizontally and vertically
 *
 * total_cells = cols * rows;
 *
 * cell_head = a dynamic arrr of int of length "total_cells"
 * cell_head[cell_index]:
 * 		--> the array index of the first particle in that cell
 * 		--> if a cell has no particles, it contains -1.
 *
 * particle_next = a dynamic array of ints sized to match particle capacity
 * particle_next[particle_idx]:
 * 		--> the index of the next particle in that same cell
 * 		--> if particle p_idx is the last one in the cell, this equals -1
 *
 */
typedef struct {
	float cell_size;
	int cols;
	int rows;
	size_t total_cells;

	int *cell_heads;
	int *particle_next;
} SpatialGrid;

/* Particles = the flat array storring all active particles
 * 		--> # of particles = capacity
 *
 * count = how many particles currently exist in the simulation
 * 		--> new particles go to particles[count++]
 *
 * capacity = how many particles the particle buffer can hold before
 * reallocation must occur.
 *
 * grid = embedded SpatialGrid struct;
 *
 * gravity = 2D vector applied to all particles with each substep
 *
 * bounds_width and bounds_height = the bounding box dimensions
 * 		--> particles can't leave these boundaries
 *
 * substeps = the number of timies the physicss update per frame
 * 		--> smaller time steps prevent high-speed particles from clipping
 */
typedef struct {
	Particle *particles;
	size_t count;
	size_t capacity;

	SpatialGrid grid;

	vector gravity;
	float bounds_width;
	float bounds_height;
	int substeps;
} SimWorld;

/** the frame lifecycle
 *
 * Eaacah frame calls sim_world_step(world, dt).
 * 		dt = delta time. this is split into multiple smaller sub-steps
 * 		sub_dt = dt / world->substeps
 *
 * With each substep for each substep:
 *
 * 1. Apply global forces
 * 		--> for each active particle, apply gravity:
 * 		--> particle.accel += world->gravity
 *
 * 2. Numerical Integration:
 * 		--> compute velocity from displacement (cel = pos-prev_pos)
 * 		--> save the curr position as the new previous position
 * 		--> update ois using Verlet integration
 * 			pos += vel + accel * (sub_dt * sub_dt)
 * 		--> clear accumulated acceleration for the next pass (reset accel to 0)
 *
 *
 * 3. Rebuild the spatial grid (sim_grid_rebuild)
 * 		--> reset all cell heads
 * 		--> forr each particle
 * 				- clamp the particle to valid grid coords
 * 				- calculate the cell idx = col + row * cols
 * 				- prepend particle to cell linked list
 *
 * 4. Solve particle collisions (sim_solve_colisions)
 * 		--> iterate through each cell in the grid
 * 		--> for each cell, check partricles against:
 * 				other particles in the SAME cell
 * 				particles in neighbouring cells
 * 		--> for each pair (p1, p2)
 * 			- if distance < (radius1 + radius2)
 * 				push the particles apart
 *
 * 5. Enforce world boundaries (sim_solve_boundaries)
 *    - For every active particle:
 *        - Clamp pos.x between [radius, bounds_width - radius]
 *        - Clamp pos.y between [radius, bounds_height - radius]
 *
 * 6. render pass (done after all other work)
 *
 */


/* --- Lifecycle & Memory Management --- */

/**
 * Allocates particle buffer, grid cell arrays, and sets boundary limits.
 * Returns 1 on success, 0 if malloc failed.
 */
int sim_world_init(SimWorld *world, size_t initial_capacity, float width, float height, float cell_size);

/**
 * Frees all heap memory (particles, cell_heads, particle_next).
 */
void sim_world_destroy(SimWorld *world);

/* --- Particle Management --- */

/**
 * Spawns a particle. Reallocates particle buffer and particle_next if count == capacity.
 * Returns particle index, or -1 on allocation failure.
 */
int sim_world_add_particle(SimWorld *world, vector pos, vector initial_velocity, float radius, float mass, uint32_t color);

/**
 * Resets particle count to 0 without freeing memory buffers.
 */
void sim_world_clear(SimWorld *world);

/* --- Physics Pipeline Execution --- */

/**
 * Runs one full frame: divides dt into substeps and executes steps 1-5 per substep.
 */
void sim_world_step(SimWorld *world, float dt);

/**
 * Step 1: Adds gravity to particle accel.
 */
void sim_world_apply_gravity(SimWorld *world);

/**
 * Step 3: Clears grid and repopulates cell_heads & particle_next.
 */
void sim_grid_rebuild(SimWorld *world);

/**
 * Step 4: Checks neighboring cells and resolves particle overlaps.
 */
void sim_solve_collisions(SimWorld *world);

/**
 * Step 5: Clamps particle coordinates within bounds.
 */
void sim_solve_boundaries(SimWorld *world);

#endif 
