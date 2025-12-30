#define SDL_MAIN_HANDLED 
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <string>
#include <iostream>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

enum TipEntitate {
    NECUNOSCUT,
    JUCATOR,
    MONSTRU,
    PLATFORMA
};

class Entitate {
public:
    float x, y;
    float w, h;
    Uint8 r, g, b;
    TipEntitate tip;
    float oldX, oldY;

    Entitate(float startX, float startY, float width, float height, Uint8 red, Uint8 green, Uint8 blue, TipEntitate tipulMeu)
        : x(startX), y(startY), w(width), h(height), r(red), g(green), b(blue), tip(tipulMeu) {
        oldX = startX;
        oldY = startY;
    }

    virtual ~Entitate() {}
    virtual void Update(float deltaTime) = 0;

    SDL_FRect GetRect() { return { x, y, w, h }; }

    virtual void Render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_FRect rect = GetRect();
        SDL_RenderFillRect(renderer, &rect);
    }

    void UndoX() { x = oldX; }
    void UndoY() { y = oldY; }
};

class Jucator : public Entitate {
public:
    float viteza;
    float intentieX, intentieY;
    float startX, startY;

    Jucator(float _startX, float _startY)
        : Entitate(_startX, _startY, 40, 40, 0, 0, 255, JUCATOR) {
        viteza = 300.0f;
        intentieX = 0; intentieY = 0;
        startX = _startX;
        startY = _startY;
    }

    void Update(float deltaTime) override {
        intentieX = 0; intentieY = 0;
        const bool* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])    intentieY = -viteza * deltaTime;
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])  intentieY = viteza * deltaTime;
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  intentieX = -viteza * deltaTime;
        if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) intentieX = viteza * deltaTime;
    }

    void MiscaX() {
        oldX = x;
        x += intentieX;
        if (x < 0) x = 0;
        if (x > SCREEN_WIDTH - w) x = SCREEN_WIDTH - w;
    }

    void MiscaY() {
        oldY = y;
        y += intentieY;
        if (y < 0) y = 0;
        if (y > SCREEN_HEIGHT - h) y = SCREEN_HEIGHT - h;
    }

    void Respawn() {
        x = startX;
        y = startY;
    }
};

class Monstru : public Entitate {
public:
    float velX, velY;

    Monstru(float startX, float startY)
        : Entitate(startX, startY, 30, 30, 255, 0, 0, MONSTRU) {
        velX = 150.0f;
        velY = 150.0f;
    }

    void Update(float deltaTime) override {}

    void MiscaX(float deltaTime) {
        oldX = x;
        x += velX * deltaTime;
        if (x <= 0) { x = 0; velX *= -1; }
        if (x >= SCREEN_WIDTH - w) { x = SCREEN_WIDTH - w; velX *= -1; }
    }

    void MiscaY(float deltaTime) {
        oldY = y;
        y += velY * deltaTime;
        if (y <= 0) { y = 0; velY *= -1; }
        if (y >= SCREEN_HEIGHT - h) { y = SCREEN_HEIGHT - h; velY *= -1; }
    }
};

class Platforma : public Entitate {
public:
    Platforma(float startX, float startY, float latime, float inaltime)
        : Entitate(startX, startY, latime, inaltime, 0, 255, 0, PLATFORMA) {
    }

    void Update(float deltaTime) override {}
};

class GameEngine {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;
    std::vector<Entitate*> obiecte;
    Jucator* jucatorRef = nullptr;

public:
    GameEngine() : window(NULL), renderer(NULL), isRunning(false) {}

    bool Init() {
        if (!SDL_Init(SDL_INIT_VIDEO)) return false;
        if (!SDL_CreateWindowAndRenderer("Project Phase 2 Final", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer)) return false;
        isRunning = true;
        return true;
    }

    void LoadLevel() {
        jucatorRef = new Jucator(50, 50);
        obiecte.push_back(jucatorRef);

        obiecte.push_back(new Monstru(300, 200));
        obiecte.push_back(new Monstru(800, 400));

        obiecte.push_back(new Platforma(400, 300, 20, 200));
        obiecte.push_back(new Platforma(600, 300, 20, 200));
        obiecte.push_back(new Platforma(400, 300, 220, 20));
        obiecte.push_back(new Platforma(400, 480, 220, 20));
    }

    bool CheckWallCollision(Entitate* cine) {
        SDL_FRect rectCine = cine->GetRect();
        for (auto* obj : obiecte) {
            if (obj == cine) continue;
            if (obj->tip == PLATFORMA) {
                SDL_FRect rectPlatforma = obj->GetRect();
                if (SDL_HasRectIntersectionFloat(&rectCine, &rectPlatforma)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool CheckPlayerVsMonster() {
        SDL_FRect rectPlayer = jucatorRef->GetRect();
        for (auto* obj : obiecte) {
            if (obj->tip == MONSTRU) {
                SDL_FRect rectMonster = obj->GetRect();
                if (SDL_HasRectIntersectionFloat(&rectPlayer, &rectMonster)) {
                    return true;
                }
            }
        }
        return false;
    }

    void Update(float deltaTime) {
        for (auto* obj : obiecte) obj->Update(deltaTime);

        for (auto* obj : obiecte) {
            if (obj->tip == MONSTRU) {
                Monstru* m = (Monstru*)obj;

                m->MiscaX(deltaTime);
                if (CheckWallCollision(m)) {
                    m->UndoX();
                    m->velX *= -1;
                }

                m->MiscaY(deltaTime);
                if (CheckWallCollision(m)) {
                    m->UndoY();
                    m->velY *= -1;
                }
            }
        }

        if (jucatorRef) {
            jucatorRef->MiscaX();
            if (CheckWallCollision(jucatorRef)) {
                jucatorRef->UndoX();
            }

            jucatorRef->MiscaY();
            if (CheckWallCollision(jucatorRef)) {
                jucatorRef->UndoY();
            }

            if (CheckPlayerVsMonster()) {
                jucatorRef->Respawn();
            }
        }
    }

    void Render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        for (auto* obj : obiecte) obj->Render(renderer);
        SDL_RenderPresent(renderer);
    }

    void HandleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) isRunning = false;
        }
    }

    void Clean() {
        for (auto* obj : obiecte) delete obj;
        obiecte.clear();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool Running() { return isRunning; }
};

int main(int argc, char* argv[]) {
    GameEngine engine;
    if (!engine.Init()) return -1;

    engine.LoadLevel();
    Uint64 lastPerf = SDL_GetPerformanceCounter();

    while (engine.Running()) {
        Uint64 nowPerf = SDL_GetPerformanceCounter();
        float deltaTime = (float)(nowPerf - lastPerf) / (float)SDL_GetPerformanceFrequency();
        lastPerf = nowPerf;
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        engine.HandleEvents();
        engine.Update(deltaTime);
        engine.Render();
    }
    engine.Clean();
    return 0;
}