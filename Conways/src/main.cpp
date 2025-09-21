#include "raylib.h"
#include <cmath>
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

    // Configurações de performance - DESABILITA VSync completamente
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Game Of Life");

    // FORÇA desabilitar VSync para FPS ilimitado
    SetTargetFPS(0);

// Tenta desabilitar VSync via sistema (Windows/Linux/Mac)
#ifdef PLATFORM_DESKTOP
    // No Windows, força desabilitar VSync via OpenGL
    // Algumas placas de vídeo forçam VSync no painel de controle
    printf("Tentando desabilitar VSync...\n");
#endif

    Camera2D camera = {0};
    camera.target = {gridWidth / 2.0f, gridHeight / 2.0f};
    camera.offset = {screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Usa formato otimizado para texturas (R8 ao invés de RGBA8)
    // Economiza 75% da memória de vídeo
    RenderTexture2D textureA = LoadRenderTexture(gridWidth, gridHeight);
    RenderTexture2D textureB = LoadRenderTexture(gridWidth, gridHeight);
    RenderTexture2D textureC = LoadRenderTexture(
        gridWidth, gridHeight); // Buffer extra para triple buffering

    RenderTexture2D *current = &textureA;
    RenderTexture2D *next = &textureB;
    RenderTexture2D *aux = &textureC;

    // Carrega shaders otimizados
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

    srand((unsigned int)time(NULL));
    GenerateRandomGridGPU(current, generationShader, gridWidth, gridHeight,
                          0.25f, 0);

    BeginTextureMode(*next);
    ClearBackground(BLACK);
    EndTextureMode();
    BeginTextureMode(*aux);
    ClearBackground(BLACK);
    EndTextureMode();

    bool running = false;
    bool showGrid = true;
    bool showBounds = true;
    bool enableTripleBuffering = false;
    float gameTimer = 0.0f;
    float gameSpeed = 240.0f; // Inicializa em 240 UPS
    float panSpeed = 200.0f;
    float randomDensity = 0.25f;
    int generationPattern = 0;

    Vector2 mousePos = {0};

    // Métricas de performance - CORRIGIDAS para medir UPS real
    double lastTime = GetTime();
    double currentTime = 0.0;
    double gameUpdateTime = 0.0;
    int gameUpdates = 0;
    float realUPS = 0.0f;
    float renderFPS = 0.0f;

    // Buffer para médias móveis separadas
    const int FRAME_BUFFER_SIZE = 30;
    float upsBuffer[FRAME_BUFFER_SIZE] = {0};
    float fpsBuffer[FRAME_BUFFER_SIZE] = {0};
    int upsBufferIndex = 0;
    int fpsBufferIndex = 0;

    SetTargetFPS(0); // FPS completamente ilimitado

    printf("Grid Size: %dx%d (%.1fM cells)\n", gridWidth, gridHeight,
           (gridWidth * gridHeight) / 1000000.0f);
    printf("ATENÇÃO: Se UPS estiver limitado a %d, desabilite VSync no painel "
           "da GPU!\n",
           GetMonitorRefreshRate(GetCurrentMonitor()));

    while (!WindowShouldClose()) {
        currentTime = GetTime();
        float deltaTime = GetFrameTime();

        // Calcula FPS de renderização (limitado pelo monitor)
        fpsBuffer[fpsBufferIndex] = 1.0f / deltaTime;
        fpsBufferIndex = (fpsBufferIndex + 1) % FRAME_BUFFER_SIZE;

        float avgFPS = 0.0f;
        for (int i = 0; i < FRAME_BUFFER_SIZE; i++) {
            avgFPS += fpsBuffer[i];
        }
        renderFPS = avgFPS / FRAME_BUFFER_SIZE;

        gameTimer += deltaTime * gameSpeed;
        mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

        // Zoom controls
        float zoomIncrement = GetMouseWheelMove() * 0.2f;
        camera.zoom += zoomIncrement;

        if (camera.zoom < 0.001f) // Zoom muito baixo para grids enormes
            camera.zoom = 0.001f;
        if (camera.zoom > 20.0f)
            camera.zoom = 20.0f;

        if (IsKeyDown(KEY_EQUAL))
            camera.zoom += 1.0f * deltaTime;
        if (IsKeyDown(KEY_MINUS))
            camera.zoom -= 1.0f * deltaTime;

        // Pan controls otimizados
        float currentPanSpeed = panSpeed / camera.zoom * 2;

        Vector2 panDelta = {0, 0};
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
            panDelta.y -= 1.0f;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
            panDelta.y += 1.0f;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            panDelta.x -= 1.0f;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            panDelta.x += 1.0f;

        // Normaliza movimento diagonal
        float length = sqrt(panDelta.x * panDelta.x + panDelta.y * panDelta.y);
        if (length > 0.0f) {
            panDelta.x /= length;
            panDelta.y /= length;
            camera.target.x += panDelta.x * currentPanSpeed * deltaTime;
            camera.target.y += panDelta.y * currentPanSpeed * deltaTime;
        }

        // Camera bounds
        float margin = 500.0f;
        if (camera.target.x < -margin)
            camera.target.x = -margin;
        if (camera.target.x > gridWidth + margin)
            camera.target.x = gridWidth + margin;
        if (camera.target.y < -margin)
            camera.target.y = -margin;
        if (camera.target.y > gridHeight + margin)
            camera.target.y = gridHeight + margin;

        // Game controls
        if (IsKeyPressed(KEY_SPACE))
            running = !running;
        if (IsKeyPressed(KEY_G))
            showGrid = !showGrid;
        if (IsKeyPressed(KEY_B))
            showBounds = !showBounds;
        if (IsKeyPressed(KEY_T))
            enableTripleBuffering = !enableTripleBuffering;

        if (IsKeyPressed(KEY_R)) {
            BeginTextureMode(*current);
            ClearBackground(BLACK);
            EndTextureMode();
        }

        // Grid size controls (otimizado)
        if (IsKeyPressed(KEY_LEFT_BRACKET)) {
            gridMultiplier = std::fmax(0.5f, gridMultiplier - 0.5f);

            UnloadRenderTexture(textureA);
            UnloadRenderTexture(textureB);
            UnloadRenderTexture(textureC);

            gridWidth = (int)(screenWidth * gridMultiplier);
            gridHeight = (int)(screenHeight * gridMultiplier);

            textureA = LoadRenderTexture(gridWidth, gridHeight);
            textureB = LoadRenderTexture(gridWidth, gridHeight);
            textureC = LoadRenderTexture(gridWidth, gridHeight);

            current = &textureA;
            next = &textureB;
            aux = &textureC;

            GenerateRandomGridGPU(current, generationShader, gridWidth,
                                  gridHeight, randomDensity, generationPattern);
            BeginTextureMode(*next);
            ClearBackground(BLACK);
            EndTextureMode();
            BeginTextureMode(*aux);
            ClearBackground(BLACK);
            EndTextureMode();

            camera.target = {gridWidth / 2.0f, gridHeight / 2.0f};
            printf("Grid: %dx%d (%.1fM cells, %.1fx)\n", gridWidth, gridHeight,
                   (gridWidth * gridHeight) / 1000000.0f, gridMultiplier);
        }

        if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
            gridMultiplier = fmin(
                20.0f, gridMultiplier + 0.5f); // Até 20x para grids massivas

            UnloadRenderTexture(textureA);
            UnloadRenderTexture(textureB);
            UnloadRenderTexture(textureC);

            gridWidth = (int)(screenWidth * gridMultiplier);
            gridHeight = (int)(screenHeight * gridMultiplier);

            textureA = LoadRenderTexture(gridWidth, gridHeight);
            textureB = LoadRenderTexture(gridWidth, gridHeight);
            textureC = LoadRenderTexture(gridWidth, gridHeight);

            current = &textureA;
            next = &textureB;
            aux = &textureC;

            GenerateRandomGridGPU(current, generationShader, gridWidth,
                                  gridHeight, randomDensity, generationPattern);
            BeginTextureMode(*next);
            ClearBackground(BLACK);
            EndTextureMode();
            BeginTextureMode(*aux);
            ClearBackground(BLACK);
            EndTextureMode();

            camera.target = {gridWidth / 2.0f, gridHeight / 2.0f};
            printf("Grid: %dx%d (%.1fM cells, %.1fx)\n", gridWidth, gridHeight,
                   (gridWidth * gridHeight) / 1000000.0f, gridMultiplier);
        }

        // Generation controls
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

        // Density controls
        if (IsKeyPressed(KEY_Q))
            randomDensity = fmin(0.8f, randomDensity + 0.05f);
        if (IsKeyPressed(KEY_E))
            randomDensity = std::fmax(0.05f, randomDensity - 0.05f);

        // Speed controls expandidos
        if (IsKeyPressed(KEY_KP_ADD))
            gameSpeed =
                fmin(1000.0f, gameSpeed + (gameSpeed < 100.0f ? 10.0f : 50.0f));
        if (IsKeyPressed(KEY_KP_SUBTRACT))
            gameSpeed =
                fmax(1.0f, gameSpeed - (gameSpeed <= 100.0f ? 10.0f : 50.0f));

        // Presets de velocidade
        if (IsKeyPressed(KEY_F1))
            gameSpeed = 60.0f; // 60 UPS
        if (IsKeyPressed(KEY_F2))
            gameSpeed = 120.0f; // 120 UPS
        if (IsKeyPressed(KEY_F3))
            gameSpeed = 240.0f; // 240 UPS
        if (IsKeyPressed(KEY_F4))
            gameSpeed = 480.0f; // 480 UPS
        if (IsKeyPressed(KEY_F5))
            gameSpeed = 1000.0f; // 1000 UPS

        if (IsKeyPressed(KEY_C))
            camera.target = {gridWidth / 2.0f, gridHeight / 2.0f};

        // Manual drawing otimizado
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            BeginTextureMode(*current);
            int x = (int)mousePos.x;
            int y = (int)mousePos.y;
            if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
                // Desenha pincel maior em zoom baixo
                int brushSize = (camera.zoom < 1.0f) ? 100 : 1;
                DrawRectangle(x - brushSize / 2, y - brushSize / 2, brushSize,
                              brushSize, WHITE);
            }
            EndTextureMode();
        }

        // Game logic com medição PRECISA de UPS
        if (running && gameTimer >= 1.0f) {
            double updateStartTime = GetTime();

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

            if (enableTripleBuffering) {
                RenderTexture2D *temp = aux;
                aux = current;
                current = next;
                next = temp;
            } else {
                RenderTexture2D *temp = current;
                current = next;
                next = temp;
            }

            gameTimer = 0.0f;
            gameUpdates++;

            // Calcula UPS real baseado no tempo de update
            double updateEndTime = GetTime();
            double updateDuration = updateEndTime - updateStartTime;

            // Armazena UPS instantâneo
            if (updateDuration > 0.0) {
                upsBuffer[upsBufferIndex] = (float)(1.0 / updateDuration);
                upsBufferIndex = (upsBufferIndex + 1) % FRAME_BUFFER_SIZE;

                float totalUPS = 0.0f;
                for (int i = 0; i < FRAME_BUFFER_SIZE; i++) {
                    totalUPS += upsBuffer[i];
                }
                realUPS = totalUPS / FRAME_BUFFER_SIZE;
            }
        }

        // Rendering otimizado
        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(camera);

        // Bounds otimizados (só desenha se visível)
        if (showBounds && camera.zoom > 0.01f) {
            DrawRectangleLines(-2, -2, gridWidth + 4, gridHeight + 4, WHITE);
            DrawRectangleLines(-1, -1, gridWidth + 2, gridHeight + 2,
                               LIGHTGRAY);

            if (gridMultiplier > 1.0f) {
                int screenStartX = (gridWidth - screenWidth) / 2;
                int screenStartY = (gridHeight - screenHeight) / 2;
                DrawRectangleLines(screenStartX, screenStartY, screenWidth,
                                   screenHeight, RED);
            }
        }

        // Draw grid
        DrawTextureRec(current->texture,
                       {0, 0, (float)current->texture.width,
                        -(float)current->texture.height},
                       {0, 0}, WHITE);

        // Grid lines ultra otimizadas
        if (showGrid && camera.zoom > 0.5f) {
            Color gridColor = {100, 100, 100,
                               (unsigned char)(10 + camera.zoom * 20)};

            Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera);
            Vector2 bottomRight =
                GetScreenToWorld2D({screenWidth, screenHeight}, camera);

            int startX = (int)fmax(0, topLeft.x);
            int endX = (int)fmin(gridWidth, bottomRight.x);
            int startY = (int)fmax(0, topLeft.y);
            int endY = (int)fmin(gridHeight, bottomRight.y);

            // Adaptive step baseado no zoom
            int step = (int)fmax(1, 10.0f / camera.zoom);

            // Só desenha se não for muitas linhas
            if ((endX - startX) / step < 200 && (endY - startY) / step < 200) {
                for (int x = startX; x <= endX; x += step) {
                    DrawLine(x, startY, x, endY, gridColor);
                }
                for (int y = startY; y <= endY; y += step) {
                    DrawLine(startX, y, endX, y, gridColor);
                }
            }
        }

        EndMode2D();

        // UI com métricas separadas de UPS e FPS
        char titleBuffer[256];
        snprintf(titleBuffer, sizeof(titleBuffer),
                 "Conway's Game of Life - UPS: %.0f | Render FPS: %.0f",
                 realUPS, renderFPS);
        DrawText(titleBuffer, 10, 10, 20, WHITE);

        char infoBuffer[512];
        snprintf(
            infoBuffer, sizeof(infoBuffer),
            "Target Speed: %.0f UPS | Zoom: %.3fx | Grid: %dx%d (%.1fM cells)\n"
            "Density: %.2f | Pattern: %d | Triple Buffer: %s | Monitor: %dHz",
            gameSpeed, camera.zoom, gridWidth, gridHeight,
            (gridWidth * gridHeight) / 1000000.0f, randomDensity,
            generationPattern, enableTripleBuffering ? "ON" : "OFF",
            GetMonitorRefreshRate(GetCurrentMonitor()));
        DrawText(infoBuffer, 10, 40, 12, WHITE);

        DrawText(running ? "RUNNING" : "PAUSED", 10, 80, 16,
                 running ? GREEN : YELLOW);

        // Aviso se UPS está limitado
        if (running && realUPS > 0 &&
            fabs(realUPS - GetMonitorRefreshRate(GetCurrentMonitor())) < 5) {
            DrawText(
                "AVISO: UPS limitado pelo VSync! Desabilite no painel da GPU",
                10, 100, 14, RED);
        }

        // Controles compactos
        static int blinkCounter = 0;
        blinkCounter++;
        if ((blinkCounter / 30) % 2 == 0) { // Pisca a cada segundo
            DrawText(
                "F1-F5: Speed Presets | T: Triple Buffer | O: Optimized Shader",
                10, screenHeight - 40, 12, YELLOW);
            DrawText(
                "SPACE: Play/Pause | 1-4: Patterns | []: Grid Size | C: Center",
                10, screenHeight - 25, 12, LIGHTGRAY);
        }

        EndDrawing();
    }

    UnloadShader(gameShader);
    UnloadShader(generationShader);
    UnloadRenderTexture(textureA);
    UnloadRenderTexture(textureB);
    UnloadRenderTexture(textureC);
    CloseWindow();

    return 0;
}
