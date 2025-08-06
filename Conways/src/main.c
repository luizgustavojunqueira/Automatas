#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

#define RECT_SIZE 10
#define GRID_WIDTH 100
#define GRID_HEIGHT 100

void start_grid(int width, int height, bool grid[width][height]) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            grid[i][j] = false;
        }
    }
}

void start_grid_random(int width, int height, bool grid[width][height]) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            grid[i][j] = (GetRandomValue(0, 1) == 1);
        }
    }
}

int count_neighbors(int x, int y, int width, int height,
                    bool grid[width][height]) {
    int count = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0)
                continue;
            int nx = x + i;
            int ny = y + j;
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                count += grid[nx][ny];
            }
        }
    }

    return count;
}

void game_tick(int width, int height, bool grid[width][height]) {
    bool new_grid[width][height];
    start_grid(width, height, new_grid);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int neighbor_count = count_neighbors(i, j, width, height, grid);
            if (grid[i][j]) {
                if (neighbor_count < 2) {
                    new_grid[i][j] = 0;
                } else if (neighbor_count < 4) {
                    new_grid[i][j] = 1;
                } else {
                    new_grid[i][j] = 0;
                }
            } else if (neighbor_count == 3) {
                new_grid[i][j] = 1;
            }
        }
    }
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            grid[i][j] = new_grid[i][j];
        }
    }
    printf("Game tick completed.\n");
}

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(GRID_WIDTH * RECT_SIZE, GRID_HEIGHT * RECT_SIZE,
               "Conway's Game of Life");

    bool grid[GRID_WIDTH][GRID_HEIGHT] = {0};
    start_grid(GRID_WIDTH, GRID_HEIGHT, grid);

    bool running = true;

    // game loop
    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);
        for (int i = 0; i < GRID_WIDTH; i++) {
            for (int j = 0; j < GRID_HEIGHT; j++) {
                if (grid[i][j]) {
                    DrawRectangle(i * RECT_SIZE, j * RECT_SIZE, RECT_SIZE,
                                  RECT_SIZE, WHITE);
                } else {
                    DrawRectangle(i * RECT_SIZE, j * RECT_SIZE, RECT_SIZE,
                                  RECT_SIZE, DARKGRAY);
                }
            }
        }

        if (IsKeyPressed(KEY_SPACE)) {
            start_grid_random(GRID_WIDTH, GRID_HEIGHT, grid);
        }
        if (IsKeyPressed(KEY_R)) {
            start_grid(GRID_WIDTH, GRID_HEIGHT, grid);
        }

        static float target_fps = 60.0f;
        SetTargetFPS((int)target_fps);
        if (IsKeyPressed(KEY_UP)) {
            target_fps += 10.0f;
            if (target_fps > 120.0f)
                target_fps = 120.0f;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            target_fps -= 10.0f;
            if (target_fps < 10.0f)
                target_fps = 10.0f;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            running = !running;
        }

        static float timer = 0.0f;
        timer += GetFrameTime();

        if (running && timer >= 1.0f / target_fps) {
            game_tick(GRID_WIDTH, GRID_HEIGHT, grid);
            timer = 0.0f;
        }

        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, WHITE);
        DrawText(TextFormat("Target FPS: %.1f", target_fps), 10, 40, 20, WHITE);
        DrawText("Press SPACE to randomize grid", 10, 70, 20, WHITE);
        DrawText("Press R to reset grid", 10, 100, 20, WHITE);
        DrawText("Use UP/DOWN arrows to adjust speed", 10, 130, 20, WHITE);
        DrawText("Press ENTER to pause/resume", 10, 160, 20, WHITE);
        DrawText(running ? "Running" : "Paused", 10, 190, 20, WHITE);

        // mouse interaction
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse_pos = GetMousePosition();
            int x = mouse_pos.x / RECT_SIZE;
            int y = mouse_pos.y / RECT_SIZE;
            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                grid[x][y] = !grid[x][y];
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
