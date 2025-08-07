#include "raylib.h"
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <iostream>

#define RECT_SIZE 1
#define GRID_WIDTH 1000
#define GRID_HEIGHT 1000


auto start_timer() {
    return std::chrono::high_resolution_clock::now();
}

double elapsed_ms(decltype(start_timer()) start) {
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

struct Pos {
    int x,y;
    bool operator==(const Pos& other) const{
        return x == other.x  && y == other.y;
    }
};

struct PosHash{
    size_t operator()(const Pos& pos) const{
        return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.y) << 1);
    }
};

void start_random_grid(std::unordered_set<Pos, PosHash>& grid) {
    grid.clear();
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            if (GetRandomValue(0, 15) == 1) {
                grid.insert({i, j});
            }
        }
    }
}

void game_tick(std::unordered_set<Pos, PosHash>& grid) {
    auto t1 = start_timer();
    std::unordered_set<Pos, PosHash> new_grid;
    new_grid.reserve(grid.size());
    std::unordered_map<Pos, int, PosHash> neighbor_count;
    neighbor_count.reserve(grid.size() * 9); 

    for(const auto& pos : grid) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                Pos neighbor = {pos.x + i, pos.y + j};
                neighbor_count[neighbor]++;
            }
        }
    }

    for(const auto& [pos, count] : neighbor_count) {
        bool is_alive = grid.count(pos);

        int actual_neighbors = is_alive ? count - 1 : count;

        if (is_alive && (actual_neighbors == 2 || actual_neighbors == 3)) {
            new_grid.insert(pos);
        } else if (!is_alive && actual_neighbors == 3) {
            new_grid.insert(pos);
        }
    }

    std::cout << "Tick time: " << elapsed_ms(t1) << " ms\n";

    grid = new_grid;
}


int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(GRID_WIDTH * RECT_SIZE, GRID_HEIGHT * RECT_SIZE,
               "Conway's Game of Life");

    std::unordered_set<Pos, PosHash> grid;

    bool running = true;

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);
        for (const auto& pos: grid){
            DrawRectangle(pos.x * RECT_SIZE, pos.y * RECT_SIZE, RECT_SIZE, RECT_SIZE, WHITE);
        }

        static float target_ups = 60.0f;
        if (IsKeyPressed(KEY_UP)) {
            target_ups += 10.0f;
            if (target_ups > 120.0f)
                target_ups = 120.0f;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            target_ups -= 10.0f;
            if (target_ups < 10.0f)
                target_ups = 10.0f;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            running = !running;
        }

        static float timer = 0.0f;
        timer += GetFrameTime();

        if (running && timer >= 1.0f / target_ups) {
            game_tick(grid);
            timer = 0.0f;
        }

        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, WHITE);
        DrawText(TextFormat("UPS: %.1f", target_ups), 10, 40, 20, WHITE);
        DrawText("Press SPACE to randomize grid", 10, 70, 20, WHITE);
        DrawText("Press R to reset grid", 10, 100, 20, WHITE);
        DrawText("Use UP/DOWN arrows to adjust speed", 10, 130, 20, WHITE);
        DrawText("Press ENTER to pause/resume", 10, 160, 20, WHITE);
        DrawText(running ? "Running" : "Paused", 10, 190, 20, WHITE);

        if( IsKeyPressed(KEY_SPACE)) {
            start_random_grid(grid);
        }
        if( IsKeyPressed(KEY_R)) {
            grid.clear();
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse_pos = GetMousePosition();
            int x = mouse_pos.x / RECT_SIZE;
            int y = mouse_pos.y / RECT_SIZE;
            if(grid.find({x, y}) != grid.end()) {
                grid.erase({x, y});
            } else {
                if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                    grid.insert({x, y});
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
