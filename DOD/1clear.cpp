#define SDL_MAIN_HANDLED 
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <ctime>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int NUM_OBJECTS = 50 * 1000;
const float OBJECT_SIZE = 2.0f;

struct EntityOOP {
    float x, y;
    float vx, vy;
    Uint8 r, g, b;

    void update() {
        x += vx;
        y += vy;

        // Coliziune
        if (x <= 0 || x >= SCREEN_WIDTH  - OBJECT_SIZE) vx *= -1;
        if (y <= 0 || y >= SCREEN_HEIGHT - OBJECT_SIZE) vy *= -1;
    }
};

struct SystemDOD {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> vx;
    std::vector<float> vy;
    std::vector<Uint8> r;
    std::vector<Uint8> g;
    std::vector<Uint8> b;

    void reserve(int count) {
        x.reserve(count); y.reserve(count);
        vx.reserve(count); vy.reserve(count);
        r.reserve(count); g.reserve(count); b.reserve(count);
    }

    void add(float _x, float _y, float _vx, float _vy, Uint8 _r, Uint8 _g, Uint8 _b) {
        x.push_back(_x); y.push_back(_y);
        vx.push_back(_vx); vy.push_back(_vy);
        r.push_back(_r); g.push_back(_g); b.push_back(_b);
    }

    void update(int count) {
        for (int i = 0; i < count; i++) {
            x[i] += vx[i];
            y[i] += vy[i];

            if (x[i] <= 0 || x[i] >= SCREEN_WIDTH  - OBJECT_SIZE) vx[i] *= -1;
            if (y[i] <= 0 || y[i] >= SCREEN_HEIGHT - OBJECT_SIZE) vy[i] *= -1;
        }
    }
};


int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Eroare SDL: %s", SDL_GetError());
        return -1;
    }

    SDL_Window*   window   = NULL;
    SDL_Renderer* renderer = NULL;

    if (!SDL_CreateWindowAndRenderer("SDL3: OOP vs DOD", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer))
    {
        SDL_Log("Nu s-a putut crea fereastra/renderer: %s", SDL_GetError());
        return -1;
    }

    // Generare Date
    std::vector<EntityOOP> objectsOOP;
    SystemDOD systemDOD;
    systemDOD.reserve(NUM_OBJECTS);

    std::mt19937 rng((unsigned int)time(0));
    std::uniform_real_distribution<float> distX(0, (float)SCREEN_WIDTH);
    std::uniform_real_distribution<float> distY(0, (float)SCREEN_HEIGHT);
    std::uniform_real_distribution<float> distV(-2.0f, 2.0f);
    std::uniform_int_distribution<int> distC(50, 255);

    SDL_Log("Se genereaza %d obiecte...", NUM_OBJECTS);

    for (int i = 0; i < NUM_OBJECTS; i++)
    {
        float x  = distX(rng);
        float y  = distY(rng);
        float vx = distV(rng);
        float vy = distV(rng);
        Uint8 r  = (Uint8)distC(rng);
        Uint8 g  = (Uint8)distC(rng);
        Uint8 b  = (Uint8)distC(rng);

        objectsOOP.push_back({ x, y, vx, vy, r, g, b });
        systemDOD.add(x, y, vx, vy, r, g, b);
    }

    bool running = true;
    bool useDOD = false; // Pornim cu OOP
    SDL_Event e;

    Uint64 startPerf, endPerf;
    double deltaTime; // SDL3 preferã double uneori, dar float e ok

    while (running) {
        startPerf = SDL_GetPerformanceCounter();

        // --- INPUT ---
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_SPACE) {
                    useDOD = !useDOD;
                }
            }
        }

        // UPDATE 
        if (useDOD) {
            systemDOD.update(NUM_OBJECTS);
        }
        else {
            for (auto& obj : objectsOOP) {
                obj.update();
            }
        }

        // RENDER 
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (useDOD) {
            for (int i = 0; i < NUM_OBJECTS; i++) {
                SDL_SetRenderDrawColor(renderer, systemDOD.r[i], systemDOD.g[i], systemDOD.b[i], 255);
                SDL_FRect rect = { systemDOD.x[i], systemDOD.y[i], OBJECT_SIZE, OBJECT_SIZE };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        else {
            for (const auto& obj : objectsOOP) {
                SDL_SetRenderDrawColor(renderer, obj.r, obj.g, obj.b, 255);
                SDL_FRect rect = { obj.x, obj.y, OBJECT_SIZE, OBJECT_SIZE };
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);

        endPerf = SDL_GetPerformanceCounter();
        deltaTime = (double)(endPerf - startPerf) / (double)SDL_GetPerformanceFrequency();
        int fps = (int)(1.0 / deltaTime);

        std::string title = "SDL3 | " + std::string(useDOD ? "DOD" : "OOP") +
            " | FPS: " + std::to_string(fps);
        SDL_SetWindowTitle(window, title.c_str());
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}