#include "raylib.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>

float GenerateRandomSeed() { return (float)rand() / RAND_MAX; }

void GenerateRandomGridGPU(RenderTexture2D *texture, Shader generationShader,
                           int gridWidth, int gridHeight, float density,
                           int pattern) {
    int resolutionLoc = GetShaderLocation(generationShader, "resolution");
    int seedLoc = GetShaderLocation(generationShader, "seed");
    int densityLoc = GetShaderLocation(generationShader, "density");
    int patternLoc = GetShaderLocation(generationShader, "pattern");
    int timeLoc = GetShaderLocation(generationShader, "time");

    float resolution[2] = {(float)gridWidth, (float)gridHeight};
    float seed = GenerateRandomSeed();
    float currentTime = GetTime();

    SetShaderValue(generationShader, resolutionLoc, resolution,
                   SHADER_UNIFORM_VEC2);
    SetShaderValue(generationShader, seedLoc, &seed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(generationShader, densityLoc, &density,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(generationShader, patternLoc, &pattern, SHADER_UNIFORM_INT);
    SetShaderValue(generationShader, timeLoc, &currentTime,
                   SHADER_UNIFORM_FLOAT);

    BeginTextureMode(*texture);
    ClearBackground(BLACK);
    BeginShaderMode(generationShader);

    DrawRectangle(0, 0, gridWidth, gridHeight, WHITE);

    EndShaderMode();
    EndTextureMode();
}

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 1000;

    float gridMultiplier = 2.0f;
    int gridWidth = (int)(screenWidth * gridMultiplier);
    int gridHeight = (int)(screenHeight * gridMultiplier);

    InitWindow(screenWidth, screenHeight, "Game Of Life - GPU Generation");

    Camera2D camera = {0};
    camera.target = {gridWidth / 2.0f, gridHeight / 2.0f};
    camera.offset = {screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    RenderTexture2D textureA = LoadRenderTexture(gridWidth, gridHeight);
    RenderTexture2D textureB = LoadRenderTexture(gridWidth, gridHeight);

    RenderTexture2D *current = &textureA;
    RenderTexture2D *next = &textureB;

    Shader gameShader = LoadShader(0, "./game_of_life.fs");
    Shader generationShader = LoadShader(0, "./generation.fs");

    if (gameShader.id == 0) {
        printf("ERRO: Game shader não carregou!\n");
        return -1;
    }
    if (generationShader.id == 0) {
        printf("ERRO: Generation shader não carregou!\n");
        return -1;
    }

    printf("Shaders carregados com sucesso!\n");
    printf("Game Shader ID: %d, Generation Shader ID: %d\n", gameShader.id,
           generationShader.id);

    srand((unsigned int)time(NULL));

    GenerateRandomGridGPU(current, generationShader, gridWidth, gridHeight,
                          0.25f, 0);

    BeginTextureMode(*next);
    ClearBackground(BLACK);
    EndTextureMode();

    bool running = false;
    bool showGrid = true;
    bool showBounds = true;
    float gameTimer = 0.0f;
    float gameSpeed = 10.0f;
    float panSpeed = 200.0f;
    float randomDensity = 0.25f;
    int generationPattern = 0;

    Vector2 mousePos = {0};

    SetTargetFPS(60);

    printf("Grid Size: %dx%d (Screen: %dx%d, Multiplier: %.1f)\n", gridWidth,
           gridHeight, screenWidth, screenHeight, gridMultiplier);

    while (!WindowShouldClose()) {
        if (gameSpeed < 60.0f) {
            SetTargetFPS(60);
        } else {
            SetTargetFPS((int)gameSpeed);
        }
        float deltaTime = GetFrameTime();
        gameTimer += deltaTime * gameSpeed;
        mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

        float zoomIncrement = GetMouseWheelMove() * 0.2f;
        camera.zoom += zoomIncrement;

        if (camera.zoom < 0.01f)
            camera.zoom = 0.01f;
        if (camera.zoom > 50.0f)
            camera.zoom = 50.0f;

        if (IsKeyDown(KEY_EQUAL))
            camera.zoom += 1.0f * deltaTime;
        if (IsKeyDown(KEY_MINUS))
            camera.zoom -= 1.0f * deltaTime;

        float currentPanSpeed = panSpeed / camera.zoom * 2;

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
            camera.target.y -= currentPanSpeed * deltaTime;
        }
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
            camera.target.y += currentPanSpeed * deltaTime;
        }
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
            camera.target.x -= currentPanSpeed * deltaTime;
        }
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
            camera.target.x += currentPanSpeed * deltaTime;
        }

        float margin = 200.0f;
        if (camera.target.x < -margin)
            camera.target.x = -margin;
        if (camera.target.x > gridWidth + margin)
            camera.target.x = gridWidth + margin;
        if (camera.target.y < -margin)
            camera.target.y = -margin;
        if (camera.target.y > gridHeight + margin)
            camera.target.y = gridHeight + margin;

        if (IsKeyPressed(KEY_SPACE)) {
            running = !running;
        }
        if (IsKeyPressed(KEY_G)) {
            showGrid = !showGrid;
        }
        if (IsKeyPressed(KEY_B)) {
            showBounds = !showBounds;
        }
        if (IsKeyPressed(KEY_R)) {
            BeginTextureMode(*current);
            ClearBackground(BLACK);
            EndTextureMode();
        }

        if (IsKeyPressed(KEY_LEFT_BRACKET)) {
            gridMultiplier -= 0.5f;
            if (gridMultiplier < 0.5f)
                gridMultiplier = 0.5f;

            UnloadRenderTexture(textureA);
            UnloadRenderTexture(textureB);

            gridWidth = (int)(screenWidth * gridMultiplier);
            gridHeight = (int)(screenHeight * gridMultiplier);

            textureA = LoadRenderTexture(gridWidth, gridHeight);
            textureB = LoadRenderTexture(gridWidth, gridHeight);

            current = &textureA;
            next = &textureB;

            GenerateRandomGridGPU(current, generationShader, gridWidth,
                                  gridHeight, randomDensity, generationPattern);
            BeginTextureMode(*next);
            ClearBackground(BLACK);
            EndTextureMode();

            camera.target = {gridWidth / 2.0f, gridHeight / 2.0f};

            printf("Grid resized to: %dx%d (Multiplier: %.1f)\n", gridWidth,
                   gridHeight, gridMultiplier);
        }

        if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
            gridMultiplier += 1.0f;
            if (gridMultiplier > 15.0f)
                gridMultiplier = 15.0f;

            UnloadRenderTexture(textureA);
            UnloadRenderTexture(textureB);

            gridWidth = (int)(screenWidth * gridMultiplier);
            gridHeight = (int)(screenHeight * gridMultiplier);

            textureA = LoadRenderTexture(gridWidth, gridHeight);
            textureB = LoadRenderTexture(gridWidth, gridHeight);

            current = &textureA;
            next = &textureB;

            GenerateRandomGridGPU(current, generationShader, gridWidth,
                                  gridHeight, randomDensity, generationPattern);
            BeginTextureMode(*next);
            ClearBackground(BLACK);
            EndTextureMode();

            camera.target = {gridWidth / 2.0f, gridHeight / 2.0f};

            printf("Grid resized to: %dx%d (Multiplier: %.1f)\n", gridWidth,
                   gridHeight, gridMultiplier);
        }

        if (IsKeyPressed(KEY_ONE)) {
            generationPattern = 0;
            GenerateRandomGridGPU(current, generationShader, gridWidth,
                                  gridHeight, randomDensity, generationPattern);
        }
        if (IsKeyPressed(KEY_TWO)) {
            generationPattern = 1;
            GenerateRandomGridGPU(current, generationShader, gridWidth,
                                  gridHeight, randomDensity, generationPattern);
        }
        if (IsKeyPressed(KEY_THREE)) {
            generationPattern = 2;
            GenerateRandomGridGPU(current, generationShader, gridWidth,
                                  gridHeight, randomDensity, generationPattern);
        }
        if (IsKeyPressed(KEY_FOUR)) {
            generationPattern = 3;
            GenerateRandomGridGPU(current, generationShader, gridWidth,
                                  gridHeight, randomDensity, generationPattern);
        }

        if (IsKeyPressed(KEY_Q)) {
            randomDensity += 0.05f;
            if (randomDensity > 0.8f)
                randomDensity = 0.8f;
        }
        if (IsKeyPressed(KEY_E)) {
            randomDensity -= 0.05f;
            if (randomDensity < 0.05f)
                randomDensity = 0.05f;
        }

        if (IsKeyPressed(KEY_KP_ADD)) {
            gameSpeed += 5.0f;
            if (gameSpeed > 240.0f)
                gameSpeed = 240.0f;
        }
        if (IsKeyPressed(KEY_KP_SUBTRACT)) {
            gameSpeed -= 5.0f;
            if (gameSpeed < 1.0f)
                gameSpeed = 1.0f;
        }

        if (IsKeyPressed(KEY_C)) {
            camera.target = {gridWidth / 2.0f, gridHeight / 2.0f};
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            BeginTextureMode(*current);
            int cellSize = 1;
            int x = (int)(mousePos.x / cellSize) * cellSize;
            int y = (int)(mousePos.y / cellSize) * cellSize;

            if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
                DrawRectangle(x, y, cellSize, cellSize, WHITE);
            }
            EndTextureMode();
        }

        if (running && gameTimer >= 1.0f) {
            int resolutionLoc = GetShaderLocation(gameShader, "resolution");
            int timeLoc = GetShaderLocation(gameShader, "time");

            float resolution[2] = {(float)gridWidth, (float)gridHeight};
            float time = GetTime();

            SetShaderValue(gameShader, resolutionLoc, resolution,
                           SHADER_UNIFORM_VEC2);
            SetShaderValue(gameShader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

            BeginTextureMode(*next);
            ClearBackground(BLACK);
            BeginShaderMode(gameShader);
            DrawTextureRec(current->texture,
                           {0, 0, (float)current->texture.width,
                            -(float)current->texture.height},
                           {0, 0}, WHITE);
            EndShaderMode();
            EndTextureMode();

            RenderTexture2D *temp = current;
            current = next;
            next = temp;

            gameTimer = 0.0f;
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(camera);

        if (showBounds) {
            DrawRectangleLines(-2, -2, gridWidth + 4, gridHeight + 4, WHITE);
            DrawRectangleLines(-1, -1, gridWidth + 2, gridHeight + 2,
                               LIGHTGRAY);

            int screenStartX = (gridWidth - screenWidth) / 2;
            int screenStartY = (gridHeight - screenHeight) / 2;
            DrawRectangleLines(screenStartX, screenStartY, screenWidth,
                               screenHeight, RED);
        }

        DrawTextureRec(current->texture,
                       {0, 0, (float)current->texture.width,
                        -(float)current->texture.height},
                       {0, 0}, WHITE);

        if (showGrid && camera.zoom > 1.0f) {
            int cellSize = 1;
            Color gridColor = {100, 100, 100,
                               (unsigned char)(25 + (camera.zoom - 1.0f) * 15)};

            Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera);
            Vector2 bottomRight =
                GetScreenToWorld2D({screenWidth, screenHeight}, camera);

            int startX = (int)(topLeft.x / cellSize) * cellSize;
            int endX = (int)(bottomRight.x / cellSize + 1) * cellSize;
            int startY = (int)(topLeft.y / cellSize) * cellSize;
            int endY = (int)(bottomRight.y / cellSize + 1) * cellSize;

            if (startX < 0)
                startX = 0;
            if (endX > gridWidth)
                endX = gridWidth;
            if (startY < 0)
                startY = 0;
            if (endY > gridHeight)
                endY = gridHeight;

            int step = (camera.zoom < 5.0f) ? (int)(5.0f / camera.zoom) : 1;

            for (int x = startX; x <= endX; x += step) {
                if (x >= 0 && x <= gridWidth) {
                    DrawLine(x, startY, x, endY, gridColor);
                }
            }
            for (int y = startY; y <= endY; y += step) {
                if (y >= 0 && y <= gridHeight) {
                    DrawLine(startX, y, endX, y, gridColor);
                }
            }
        }

        EndMode2D();

        DrawText("Conway's Game of Life - GPU Generation", 10, 10, 20, WHITE);
        DrawText(TextFormat("Speed: %.0f UPS", gameSpeed), 10, 40, 16, WHITE);
        DrawText(TextFormat("Zoom: %.3fx", camera.zoom), 10, 60, 16, WHITE);
        DrawText(TextFormat("Grid: %dx%d (%.1fx). Num Cells: %d", gridWidth,
                            gridHeight, gridMultiplier, gridWidth * gridHeight),
                 10, 80, 16, WHITE);
        DrawText(TextFormat("Density: %.2f", randomDensity), 10, 100, 16,
                 WHITE);

        const char *patternNames[] = {"Random", "Patterns", "Noise",
                                      "Hotspots"};
        DrawText(TextFormat("Pattern: %s", patternNames[generationPattern]), 10,
                 120, 16, WHITE);

        DrawText(running ? "RUNNING" : "PAUSED", 10, 140, 16,
                 running ? GREEN : YELLOW);

        DrawText("Controls:", 10, 170, 16, WHITE);
        DrawText("SPACE - Play/Pause", 20, 190, 12, LIGHTGRAY);
        DrawText("R - Reset (Clear)", 20, 205, 12, LIGHTGRAY);
        DrawText("G - Toggle Grid", 20, 220, 12, LIGHTGRAY);
        DrawText("B - Toggle Bounds", 20, 235, 12, LIGHTGRAY);
        DrawText("C - Center Camera", 20, 250, 12, LIGHTGRAY);
        DrawText("1-4 - Generation Types", 20, 265, 12, LIGHTGRAY);
        DrawText("[ ] - Grid Size -/+", 20, 280, 12, LIGHTGRAY);
        DrawText("Q/E - Density -/+", 20, 295, 12, LIGHTGRAY);
        DrawText("Numpad +/- - Speed", 20, 310, 12, LIGHTGRAY);
        DrawText("WASD/Arrows - Pan", 20, 325, 12, LIGHTGRAY);
        DrawText("Mouse Wheel - Zoom", 20, 340, 12, LIGHTGRAY);
        DrawText("Left Mouse - Draw", 20, 355, 12, LIGHTGRAY);

        if (!running) {
            DrawText("Press 1-4 for GPU generation patterns!", 10, 385, 16,
                     YELLOW);
        }

        EndDrawing();
    }

    UnloadShader(gameShader);
    UnloadShader(generationShader);
    UnloadRenderTexture(textureA);
    UnloadRenderTexture(textureB);
    CloseWindow();

    return 0;
}
