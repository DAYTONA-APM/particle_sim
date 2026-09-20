#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "simulator.h"

/* Helper to initialize vector structs without C99 compound literals */
static vector vec2(float x, float y)
{
    vector v;
    v.x = x;
    v.y = y;
    return v;
}

static void test_gravity_and_boundaries(void)
{
    SimWorld world;
    int idx;
    int i;

    printf("[TEST] Running gravity and floor collision test...\n");

    /* 800x600 box, cell size 20.0f, initial cap 10 */
    assert(sim_world_init(&world, 10, 800.0f, 600.0f, 20.0f) == 1);
    world.gravity = vec2(0.0f, 980.0f);
    world.substeps = 8;

    /* Drop a particle at x=400, y=100 with radius 10.0f */
    idx = sim_world_add_particle(&world, vec2(400.0f, 100.0f), vec2(0.0f, 0.0f), 10.0f, 1.0f, 0xFFFFFFFF);
    assert(idx == 0);

    /* Step forward 0.1 seconds (about 6 frames) */
    for (i = 0; i < 6; i++) {
        sim_world_step(&world, 1.0f / 60.0f);
    }
    assert(world.particles[0].pos.y > 100.0f);
    printf("  -> Position after freefall: y = %.2f\n", world.particles[0].pos.y);

    /* Step forward 2 full seconds; settles at bottom floor (600 - 10 = 590) */
    for (i = 0; i < 120; i++) {
        sim_world_step(&world, 1.0f / 60.0f);
    }
    assert(fabsf(world.particles[0].pos.y - 590.0f) < 0.01f);
    printf("  -> Position settled at floor: y = %.2f (Expected 590.00)\n", world.particles[0].pos.y);

    sim_world_destroy(&world);
    printf("[PASS] Gravity & Boundary OK\n\n");
}

static void test_two_particle_collision(void)
{
    SimWorld world;
    int i;
    float dx;
    float dist;

    printf("[TEST] Running two-particle overlap resolution test...\n");

    assert(sim_world_init(&world, 10, 800.0f, 600.0f, 20.0f) == 1);
    world.gravity = vec2(0.0f, 0.0f); /* zero gravity to isolate collision */
    world.substeps = 8;

    /* Two particles with radius 10 (target distance = 20), spawned only 10px apart */
    sim_world_add_particle(&world, vec2(100.0f, 300.0f), vec2(0.0f, 0.0f), 10.0f, 1.0f, 0xFF);
    sim_world_add_particle(&world, vec2(110.0f, 300.0f), vec2(0.0f, 0.0f), 10.0f, 1.0f, 0xFF);

    for (i = 0; i < 8; i++) {
        sim_world_step(&world, 1.0f / 60.0f);
    }

    dx = world.particles[1].pos.x - world.particles[0].pos.x;
    dist = fabsf(dx);
    printf("  -> Distance after relaxation: %.2f (Target >= 20.00)\n", dist);
    assert(dist >= 19.99f);

    sim_world_destroy(&world);
    printf("[PASS] Collision resolution OK\n\n");
}

int main(void)
{
    test_gravity_and_boundaries();
    test_two_particle_collision();
    printf("All headless physics checks passed!\n");
    return 0;
}
