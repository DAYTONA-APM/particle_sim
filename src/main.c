#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "simulator.h"

#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 700

static Color unpack_color(uint32_t c)
{
    Color col;
    col.r = (unsigned char)((c >> 24) & 0xFF);
    col.g = (unsigned char)((c >> 16) & 0xFF);
    col.b = (unsigned char)((c >> 8)  & 0xFF);
    col.a = (unsigned char)(c & 0xFF);
    return col;
}

int main(void)
{
    SimWorld world;
    float dt;
    size_t i;
    Vector2 mouse_pos;
    vector p_pos;
    vector p_vel;
    Color bg_color;
    Color border_color;
    Color text_white;
    Color text_gray;
    Color text_dark_gray;
    uint32_t color_palette[4];
	static int frame_count;

    bg_color.r = 20;  bg_color.g = 24;  bg_color.b = 30;  bg_color.a = 255;
    border_color.r = 60; border_color.g = 68; border_color.b = 80; border_color.a = 255;
    text_white.r = 245; text_white.g = 245; text_white.b = 245; text_white.a = 255;
    text_gray.r = 200; text_gray.g = 200; text_gray.b = 200; text_gray.a = 255;
    text_dark_gray.r = 130; text_dark_gray.g = 130; text_dark_gray.b = 130; text_dark_gray.a = 255;

    color_palette[0] = 0x38BDF8FF; /* Cyan */
    color_palette[1] = 0x818CF8FF; /* Indigo */
    color_palette[2] = 0xF472B6FF; /* Pink */
    color_palette[3] = 0xFBBF24FF; /* Amber */

    /* 1. Initialize Simulator World with exact 5 arguments */
    sim_world_init(&world, 1024, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 16.0f);
    world.gravity.x = 0.0f;
    world.gravity.y = 980.0f;
    world.substeps = 8;

    /* 2. Initialize Raylib Window */
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "C89 Particle Sandbox (Position-Based Dynamics)");
	SetExitKey(KEY_NULL);
    SetTargetFPS(60);

	PollInputEvents();

	printf("Starting render loop: \n");

    /* 3. Main Loop */
	printf("WindowShouldClose initial value: %d\n", WindowShouldClose());
    while (!WindowShouldClose()) {
		frame_count = 0;
        if (frame_count < 3) {
            printf("Frame: %d\n", frame_count++);
        }
        dt = GetFrameTime();
		if (dt <= 0.0f) {
    		dt = 1.0f / 60.0f;
		} else if (dt > 0.033f) {
    		dt = 0.033f;
		}
        

        /* Input: Left-click stream */
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            mouse_pos = GetMousePosition();
            p_pos.x = mouse_pos.x;
            p_pos.y = mouse_pos.y;

            p_vel.x = ((float)(rand() % 100) - 50.0f) * 2.0f;
            p_vel.y = ((float)(rand() % 50)) * 2.0f;

            sim_world_add_particle(
                &world,
                p_pos,
                p_vel,
                6.0f,
                1.0f,
                color_palette[rand() % 4]
            );
        }

        /* Input: Spacebar burst */
        if (IsKeyPressed(KEY_SPACE)) {
            int k;
            for (k = 0; k < 25; k++) {
                p_pos.x = (float)(SCREEN_WIDTH / 2) + ((float)(rand() % 120) - 60.0f);
                p_pos.y = 80.0f + ((float)(rand() % 60) - 30.0f);
                p_vel.x = ((float)(rand() % 200) - 100.0f);
                p_vel.y = ((float)(rand() % 100));

                sim_world_add_particle(
                    &world,
                    p_pos,
                    p_vel,
                    7.0f,
                    1.0f,
                    color_palette[rand() % 4]
                );
            }
        }

        /* Physics Step */
        sim_world_step(&world, dt);

        /* Render */
        BeginDrawing();
        ClearBackground(bg_color);

        DrawRectangleLines(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, border_color);

        for (i = 0; i < world.count; i++) {
            Vector2 center;
            center.x = world.particles[i].pos.x;
            center.y = world.particles[i].pos.y;
            DrawCircleV(center, world.particles[i].radius, unpack_color(world.particles[i].color));
        }

        DrawText(TextFormat("Particles: %u", (unsigned int)world.count), 20, 20, 20, text_white);
        DrawText(TextFormat("FPS: %i", GetFPS()), 20, 48, 18, text_gray);
        DrawText("Left Click + Drag: Spawn stream | Space: Burst spawn", 20, SCREEN_HEIGHT - 32, 16, text_dark_gray);

        EndDrawing();

		/* Diagnostic: Check why we are exiting after Frame 0 */
        if (WindowShouldClose()) { 
            printf("[DEBUG] Key pressed: %d\n", GetKeyPressed());
        }
    }

    CloseWindow();
    sim_world_destroy(&world);

    return 0;
}
