#define SDL_MAIN_HANDLED 
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int MAX_OBJECTS = 100*1000;

enum TipEntitate {
    
    JUCATOR,
    MONSTRU,
    PLATFORMA,
    COLECTABIL,
    PARTICULA_EXPLOZIE
};

class Entitate {
public:
    float x, y, w, h;
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
    int hp;
    int maxHp;
    float invulnerabilityTimer;
    float invulnerabilityDuration;

    Jucator(float _startX, float _startY)
        : Entitate(_startX, _startY, 40, 40, 0, 0, 255, JUCATOR) {
        viteza = 300.0f;
        intentieX = 0; intentieY = 0;
        startX = _startX;
        startY = _startY;
        hp = 3;
        maxHp = 3;
        invulnerabilityTimer = 0.0f;
        invulnerabilityDuration = 1.5f;
    }

    void Update(float deltaTime) override {
        if (invulnerabilityTimer > 0) {
            invulnerabilityTimer -= deltaTime;
        }
        intentieX = 0;
        intentieY = 0;
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
        hp = maxHp;
        invulnerabilityTimer = 0.0f;
    }

    bool TakeDamage(int amount) {
        if (invulnerabilityTimer > 0) return false;
        hp -= amount;
        if (hp < 0) hp = 0;
        invulnerabilityTimer = invulnerabilityDuration;
        return true;
    }

    bool IsAlive() const { return hp > 0; }
    bool IsInvulnerable() const { return invulnerabilityTimer > 0; }

    void Render(SDL_Renderer* renderer) override {
        if (IsInvulnerable()) {
            int flash = (int)(invulnerabilityTimer * 10) % 2;
            if (flash == 0) return;
        }
        Entitate::Render(renderer);
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

class Colectabil : public Entitate {
public:
    int valoare;
    bool colectat;

    Colectabil(float startX, float startY, int val)
        : Entitate(startX, startY, 20, 20, 255, 255, 0, COLECTABIL) {
        valoare = val;
        colectat = false;
    }

    void Update(float deltaTime) override {}

    void Render(SDL_Renderer* renderer) override {
        if (colectat) return;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_FRect rect = GetRect();
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
        SDL_FRect innerRect = { x + 5, y + 5, w - 10, h - 10 };
        SDL_RenderFillRect(renderer, &innerRect);
    }
};

class ParticulaExplozie : public Entitate {
public:
    float velX, velY;
    float viata;
    float viataMaxima;

    ParticulaExplozie(float startX, float startY)
        : Entitate(startX, startY, 5, 5, 255, 150, 0, PARTICULA_EXPLOZIE) {
        velX = (rand() % 200 - 100) * 2.0f;
        velY = (rand() % 200 - 100) * 2.0f;
        viata = 0.5f + (rand() % 100) / 200.0f;
        viataMaxima = viata;
    }

    void Update(float deltaTime) override {
        x += velX * deltaTime;
        y += velY * deltaTime;
        viata -= deltaTime;
        velX *= 0.98f;
        velY *= 0.98f;
    }

    bool EsteMoarta() const { return viata <= 0; }

    void Render(SDL_Renderer* renderer) override {
        float alpha = (viata / viataMaxima) * 255.0f;
        SDL_SetRenderDrawColor(renderer, r, g, b, (Uint8)alpha);
        SDL_FRect rect = GetRect();
        SDL_RenderFillRect(renderer, &rect);
    }
};

class ParticleOOP {
public:
    float x, y, vx, vy, size;
    Uint8 r, g, b;

    ParticleOOP() {
        x = rand() % SCREEN_WIDTH;
        y = rand() % SCREEN_HEIGHT;
        vx = (rand() % 200 - 100) / 1.0f;
        vy = (rand() % 200 - 100) / 1.0f;
        size = 5.0f;
        r = 255; g = 255; b = 0;
    }

    void Update(float dt) {
        x += vx * dt;
        y += vy * dt;
        if (x <= 0 || x >= SCREEN_WIDTH) vx *= -1;
        if (y <= 0 || y >= SCREEN_HEIGHT) vy *= -1;
    }

    void Render(SDL_Renderer* ren) {
        SDL_SetRenderDrawColor(ren, r, g, b, 150);
        SDL_FRect rect = { x, y, size, size };
        SDL_RenderFillRect(ren, &rect);
    }
};

struct ParticlesDOD {
    std::vector<float> posX, posY, velX, velY;
    int count = 0;

    void AddParticle() {
        if (count >= MAX_OBJECTS) return;
        posX.push_back(rand() % SCREEN_WIDTH);
        posY.push_back(rand() % SCREEN_HEIGHT);
        velX.push_back((rand() % 200 - 100) / 1.0f);
        velY.push_back((rand() % 200 - 100) / 1.0f);
        count++;
    }

    void RemoveParticle() {
        if (count > 0) {
            posX.pop_back(); posY.pop_back();
            velX.pop_back(); velY.pop_back();
            count--;
        }
    }

    void Update(float dt) {
        for (int i = 0; i < count; i++) {
            posX[i] += velX[i] * dt;
            posY[i] += velY[i] * dt;
            if (posX[i] <= 0 || posX[i] >= SCREEN_WIDTH) velX[i] *= -1;
            if (posY[i] <= 0 || posY[i] >= SCREEN_HEIGHT) velY[i] *= -1;
        }
    }

    void Render(SDL_Renderer* ren) {
        SDL_SetRenderDrawColor(ren, 0, 255, 255, 150);
        for (int i = 0; i < count; i++) {
            SDL_FRect r = { posX[i], posY[i], 5.0f, 5.0f };
            SDL_RenderFillRect(ren, &r);
        }
    }
};

struct MonstruDOD {
    std::vector<float> posX, posY, velX, velY, oldX, oldY;
    int count = 0;
    const float size = 30.0f;

    void AddMonstru() {
        posX.push_back(rand() % (SCREEN_WIDTH - 100) + 50);
        posY.push_back(rand() % (SCREEN_HEIGHT - 100) + 50);
        velX.push_back(150.0f);
        velY.push_back(150.0f);
        oldX.push_back(posX.back());
        oldY.push_back(posY.back());
        count++;
    }

    void RemoveMonstru() {
        if (count > 0) {
            posX.pop_back(); posY.pop_back();
            velX.pop_back(); velY.pop_back();
            oldX.pop_back(); oldY.pop_back();
            count--;
        }
    }

    void UpdateX(float dt) {
        for (int i = 0; i < count; i++) {
            oldX[i] = posX[i];
            posX[i] += velX[i] * dt;
            if (posX[i] <= 0) { posX[i] = 0; velX[i] *= -1; }
            if (posX[i] >= SCREEN_WIDTH - size) { posX[i] = SCREEN_WIDTH - size; velX[i] *= -1; }
        }
    }

    void UpdateY(float dt) {
        for (int i = 0; i < count; i++) {
            oldY[i] = posY[i];
            posY[i] += velY[i] * dt;
            if (posY[i] <= 0) { posY[i] = 0; velY[i] *= -1; }
            if (posY[i] >= SCREEN_HEIGHT - size) { posY[i] = SCREEN_HEIGHT - size; velY[i] *= -1; }
        }
    }

    void UndoX(int i) { posX[i] = oldX[i]; velX[i] *= -1; }
    void UndoY(int i) { posY[i] = oldY[i]; velY[i] *= -1; }

    SDL_FRect GetRect(int i) { return { posX[i], posY[i], size, size }; }

    void Render(SDL_Renderer* ren) {
        SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        for (int i = 0; i < count; i++) {
            SDL_FRect r = GetRect(i);
            SDL_RenderFillRect(ren, &r);
        }
    }
};

class GameEngine {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;
    std::vector<Entitate*> obiecteJoc;
    Jucator* jucatorRef = nullptr;

    std::vector<ParticleOOP*> particlesOOP;
    ParticlesDOD particlesDOD;
    bool useDOD = false;

    std::vector<Monstru*> monstruOOP;
    MonstruDOD monstruDOD;
    bool useDODMonstri = false;

    int scor;
    bool gameOver;
    bool levelComplete;

public:
    GameEngine() : window(NULL), renderer(NULL), isRunning(false), scor(0), gameOver(false), levelComplete(false), useDODMonstri(false) { srand((unsigned int)time(0)); }

    bool Init() {
        if (!SDL_Init(SDL_INIT_VIDEO)) return false;
        if (!SDL_CreateWindowAndRenderer("DOD Project Phase 1", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer)) return false;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        isRunning = true;
        return true;
    }

    void LoadLevel() {
        jucatorRef = new Jucator(50, 50);
        obiecteJoc.push_back(jucatorRef);

        monstruOOP.push_back(new Monstru(300, 200));
        monstruOOP.push_back(new Monstru(800, 400));
        monstruOOP.push_back(new Monstru(500, 500));

        obiecteJoc.push_back(new Platforma(900, 200, 20, 300));
        obiecteJoc.push_back(new Platforma(1100, 200, 20, 300));
        obiecteJoc.push_back(new Platforma(900, 480, 220, 20));

        obiecteJoc.push_back(new Colectabil(200, 150, 10));
        obiecteJoc.push_back(new Colectabil(600, 250, 10));
        obiecteJoc.push_back(new Colectabil(1000, 350, 20));
        obiecteJoc.push_back(new Colectabil(450, 450, 15));
        obiecteJoc.push_back(new Colectabil(750, 550, 10));
    }

    void ModifyObjectCount(int amount) {
        int current = useDOD ? particlesDOD.count : (int)particlesOOP.size();
        int target = current + amount;
        if (target > MAX_OBJECTS) target = MAX_OBJECTS;
        if (target < 0) target = 0;
        int diff = target - current;

        if (useDOD) {
            if (diff > 0) for (int i = 0; i < diff; i++) particlesDOD.AddParticle();
            else for (int i = 0; i < abs(diff); i++) particlesDOD.RemoveParticle();
        }
        else {
            if (diff > 0) for (int i = 0; i < diff; i++) particlesOOP.push_back(new ParticleOOP());
            else for (int i = 0; i < abs(diff); i++) {
                if (!particlesOOP.empty()) { delete particlesOOP.back(); particlesOOP.pop_back(); }
            }
        }
    }

    void ToggleMode() {
        int count = useDOD ? particlesDOD.count : (int)particlesOOP.size();
        for (auto p : particlesOOP) delete p;
        particlesOOP.clear();
        particlesDOD.posX.clear();
        particlesDOD.posY.clear();
        particlesDOD.velX.clear();
        particlesDOD.velY.clear();
        particlesDOD.count = 0;
        useDOD = !useDOD;
        if (useDOD) for (int i = 0; i < count; i++) particlesDOD.AddParticle();
        else for (int i = 0; i < count; i++) particlesOOP.push_back(new ParticleOOP());
    }

    void ModifyMonsterCount(int amount) {
        int current = useDODMonstri ? monstruDOD.count : (int)monstruOOP.size();
        int target = current + amount;
        if (target < 0) target = 0;
        int diff = target - current;

        if (useDODMonstri) {
            if (diff > 0) for (int i = 0; i < diff; i++) monstruDOD.AddMonstru();
            else for (int i = 0; i < abs(diff); i++) monstruDOD.RemoveMonstru();
        }
        else {
            if (diff > 0) {
                for (int i = 0; i < diff; i++) {
                    float x = rand() % (SCREEN_WIDTH - 100) + 50;
                    float y = rand() % (SCREEN_HEIGHT - 100) + 50;
                    monstruOOP.push_back(new Monstru(x, y));
                }
            }
            else {
                for (int i = 0; i < abs(diff); i++) {
                    if (!monstruOOP.empty()) { delete monstruOOP.back(); monstruOOP.pop_back(); }
                }
            }
        }
    }

    void ToggleMonsterMode() {
        int count = useDODMonstri ? monstruDOD.count : (int)monstruOOP.size();
        for (auto m : monstruOOP) delete m;
        monstruOOP.clear();
        monstruDOD.posX.clear();
        monstruDOD.posY.clear();
        monstruDOD.velX.clear();
        monstruDOD.velY.clear();
        monstruDOD.oldX.clear();
        monstruDOD.oldY.clear();
        monstruDOD.count = 0;
        useDODMonstri = !useDODMonstri;
        if (useDODMonstri) {
            for (int i = 0; i < count; i++) monstruDOD.AddMonstru();
        }
        else {
            for (int i = 0; i < count; i++) {
                float x = rand() % (SCREEN_WIDTH - 100) + 50;
                float y = rand() % (SCREEN_HEIGHT - 100) + 50;
                monstruOOP.push_back(new Monstru(x, y));
            }
        }
    }

    void CreateExplosion(float x, float y, int numParticles = 20) {
        for (int i = 0; i < numParticles; i++) {
            obiecteJoc.push_back(new ParticulaExplozie(x, y));
        }
    }

    bool CheckWallCollision(Entitate* cine) {
        SDL_FRect rectCine = cine->GetRect();
        for (auto* obj : obiecteJoc) {
            if (obj == cine) continue;
            if (obj->tip == PLATFORMA) {
                SDL_FRect rectPlatforma = obj->GetRect();
                if (SDL_HasRectIntersectionFloat(&rectCine, &rectPlatforma)) return true;
            }
        }
        return false;
    }

    bool CheckPlayerVsMonster() {
        SDL_FRect rectPlayer = jucatorRef->GetRect();

        if (useDODMonstri) {
            for (int i = 0; i < monstruDOD.count; i++) {
                SDL_FRect rectMonster = monstruDOD.GetRect(i);
                if (SDL_HasRectIntersectionFloat(&rectPlayer, &rectMonster)) return true;
            }
        }
        else {
            for (auto* m : monstruOOP) {
                SDL_FRect rectMonster = m->GetRect();
                if (SDL_HasRectIntersectionFloat(&rectPlayer, &rectMonster)) return true;
            }
        }
        return false;
    }

    bool CheckWallCollisionDOD(int monsterId) {
        SDL_FRect rectMonstru = monstruDOD.GetRect(monsterId);
        for (auto* obj : obiecteJoc) {
            if (obj->tip == PLATFORMA) {
                SDL_FRect rectPlatforma = obj->GetRect();
                if (SDL_HasRectIntersectionFloat(&rectMonstru, &rectPlatforma)) return true;
            }
        }
        return false;
    }

    void Update(float deltaTime) {
        if (gameOver || levelComplete) return;

        for (auto* obj : obiecteJoc) obj->Update(deltaTime);

        if (useDODMonstri) {
            monstruDOD.UpdateX(deltaTime);
            for (int i = 0; i < monstruDOD.count; i++) {
                if (CheckWallCollisionDOD(i)) monstruDOD.UndoX(i);
            }
            monstruDOD.UpdateY(deltaTime);
            for (int i = 0; i < monstruDOD.count; i++) {
                if (CheckWallCollisionDOD(i)) monstruDOD.UndoY(i);
            }
        }
        else {
            for (auto* m : monstruOOP) {
                m->Update(deltaTime);
                m->MiscaX(deltaTime);
                if (CheckWallCollision(m)) { m->UndoX(); m->velX *= -1; }
                m->MiscaY(deltaTime);
                if (CheckWallCollision(m)) { m->UndoY(); m->velY *= -1; }
            }
        }

        if (jucatorRef) {
            jucatorRef->MiscaX();
            if (CheckWallCollision(jucatorRef)) jucatorRef->UndoX();
            jucatorRef->MiscaY();
            if (CheckWallCollision(jucatorRef)) jucatorRef->UndoY();

            SDL_FRect rectPlayer = jucatorRef->GetRect();
            int colectabileRamase = 0;
            std::vector<std::pair<float, float>> explosionPositions;

            for (auto* obj : obiecteJoc) {
                if (obj->tip == COLECTABIL) {
                    Colectabil* col = (Colectabil*)obj;
                    if (!col->colectat) {
                        colectabileRamase++;
                        SDL_FRect rectCol = col->GetRect();
                        if (SDL_HasRectIntersectionFloat(&rectPlayer, &rectCol)) {
                            col->colectat = true;
                            scor += col->valoare;
                            explosionPositions.push_back({ col->x + col->w / 2, col->y + col->h / 2 });
                            colectabileRamase--;
                        }
                    }
                }
            }

            for (auto& pos : explosionPositions) {
                CreateExplosion(pos.first, pos.second, 15);
            }

            if (colectabileRamase == 0) {
                levelComplete = true;
            }

            if (CheckPlayerVsMonster()) {
                if (jucatorRef->TakeDamage(1)) {
                    CreateExplosion(jucatorRef->x + jucatorRef->w / 2, jucatorRef->y + jucatorRef->h / 2, 30);
                }
                if (!jucatorRef->IsAlive()) {
                    gameOver = true;
                }
            }
        }

        for (auto it = obiecteJoc.begin(); it != obiecteJoc.end();) {
            if ((*it)->tip == PARTICULA_EXPLOZIE) {
                ParticulaExplozie* p = (ParticulaExplozie*)(*it);
                if (p->EsteMoarta()) {
                    delete* it;
                    it = obiecteJoc.erase(it);
                    continue;
                }
            }
            ++it;
        }

        if (useDOD) particlesDOD.Update(deltaTime);
        else for (auto* p : particlesOOP) p->Update(deltaTime);
    }

    void Render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        for (auto* obj : obiecteJoc) obj->Render(renderer);

        if (useDODMonstri) monstruDOD.Render(renderer);
        else for (auto* m : monstruOOP) m->Render(renderer);

        if (useDOD) particlesDOD.Render(renderer);
        else for (auto* p : particlesOOP) p->Render(renderer);

        if (jucatorRef) {
            for (int i = 0; i < jucatorRef->maxHp; i++) {
                SDL_FRect hpRect = { 20.0f + i * 35.0f, 20.0f, 30.0f, 30.0f };
                if (i < jucatorRef->hp) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                }
                else {
                    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
                }
                SDL_RenderFillRect(renderer, &hpRect);
            }
        }

        if (levelComplete) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_FRect overlay = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
            SDL_RenderFillRect(renderer, &overlay);

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_FRect completeBox = { SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 - 150, 600, 300 };
            SDL_RenderFillRect(renderer, &completeBox);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect innerBox = { SCREEN_WIDTH / 2 - 280, SCREEN_HEIGHT / 2 - 130, 560, 260 };
            SDL_RenderFillRect(renderer, &innerBox);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            float textY = SCREEN_HEIGHT / 2 - 60;
            float textX = SCREEN_WIDTH / 2 - 180;

            SDL_FRect l1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &l1);
            SDL_FRect l2 = { textX, textY + 45, 40, 15 }; SDL_RenderFillRect(renderer, &l2);

            textX += 60;
            SDL_FRect e1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &e1);
            SDL_FRect e2 = { textX, textY, 50, 15 }; SDL_RenderFillRect(renderer, &e2);
            SDL_FRect e3 = { textX, textY + 22, 40, 15 }; SDL_RenderFillRect(renderer, &e3);
            SDL_FRect e4 = { textX, textY + 45, 50, 15 }; SDL_RenderFillRect(renderer, &e4);

            textX += 70;
            SDL_FRect v1 = { textX, textY, 15, 40 }; SDL_RenderFillRect(renderer, &v1);
            SDL_FRect v2 = { textX + 35, textY, 15, 40 }; SDL_RenderFillRect(renderer, &v2);
            SDL_FRect v3 = { textX + 10, textY + 40, 30, 20 }; SDL_RenderFillRect(renderer, &v3);

            textX += 70;
            SDL_FRect e5 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &e5);
            SDL_FRect e6 = { textX, textY, 50, 15 }; SDL_RenderFillRect(renderer, &e6);
            SDL_FRect e7 = { textX, textY + 22, 40, 15 }; SDL_RenderFillRect(renderer, &e7);
            SDL_FRect e8 = { textX, textY + 45, 50, 15 }; SDL_RenderFillRect(renderer, &e8);

            textX += 70;
            SDL_FRect l3 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &l3);
            SDL_FRect l4 = { textX, textY + 45, 40, 15 }; SDL_RenderFillRect(renderer, &l4);

            textY += 90;
            textX = SCREEN_WIDTH / 2 - 200;
            SDL_FRect c1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &c1);
            SDL_FRect c2 = { textX, textY, 50, 15 }; SDL_RenderFillRect(renderer, &c2);
            SDL_FRect c3 = { textX, textY + 45, 50, 15 }; SDL_RenderFillRect(renderer, &c3);

            textX += 70;
            SDL_FRect o1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &o1);
            SDL_FRect o2 = { textX + 35, textY, 15, 60 }; SDL_RenderFillRect(renderer, &o2);
            SDL_FRect o3 = { textX, textY, 50, 15 }; SDL_RenderFillRect(renderer, &o3);
            SDL_FRect o4 = { textX, textY + 45, 50, 15 }; SDL_RenderFillRect(renderer, &o4);

            textX += 70;
            SDL_FRect m1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &m1);
            SDL_FRect m2 = { textX + 35, textY, 15, 60 }; SDL_RenderFillRect(renderer, &m2);
            SDL_FRect m3 = { textX + 10, textY, 30, 20 }; SDL_RenderFillRect(renderer, &m3);

            textX += 70;
            SDL_FRect p1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &p1);
            SDL_FRect p2 = { textX, textY, 40, 15 }; SDL_RenderFillRect(renderer, &p2);
            SDL_FRect p3 = { textX + 25, textY, 15, 30 }; SDL_RenderFillRect(renderer, &p3);
            SDL_FRect p4 = { textX, textY + 22, 40, 15 }; SDL_RenderFillRect(renderer, &p4);
        }

        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_FRect overlay = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
            SDL_RenderFillRect(renderer, &overlay);

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_FRect gameOverBox = { SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 - 150, 600, 300 };
            SDL_RenderFillRect(renderer, &gameOverBox);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect innerBox = { SCREEN_WIDTH / 2 - 280, SCREEN_HEIGHT / 2 - 130, 560, 260 };
            SDL_RenderFillRect(renderer, &innerBox);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            float textY = SCREEN_HEIGHT / 2 - 80;
            float textX = SCREEN_WIDTH / 2 - 200;
            SDL_FRect g1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &g1);
            SDL_FRect g2 = { textX, textY, 50, 15 }; SDL_RenderFillRect(renderer, &g2);
            SDL_FRect g3 = { textX, textY + 45, 50, 15 }; SDL_RenderFillRect(renderer, &g3);
            SDL_FRect g4 = { textX + 35, textY + 30, 15, 30 }; SDL_RenderFillRect(renderer, &g4);
            SDL_FRect g5 = { textX + 25, textY + 30, 25, 15 }; SDL_RenderFillRect(renderer, &g5);

            textX += 70;
            SDL_FRect a1 = { textX + 15, textY, 15, 60 }; SDL_RenderFillRect(renderer, &a1);
            SDL_FRect a2 = { textX, textY + 15, 45, 15 }; SDL_RenderFillRect(renderer, &a2);
            SDL_FRect a3 = { textX, textY, 45, 15 }; SDL_RenderFillRect(renderer, &a3);

            textX += 70;
            SDL_FRect m1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &m1);
            SDL_FRect m2 = { textX + 35, textY, 15, 60 }; SDL_RenderFillRect(renderer, &m2);
            SDL_FRect m3 = { textX + 10, textY, 30, 20 }; SDL_RenderFillRect(renderer, &m3);

            textX += 70;
            SDL_FRect e1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &e1);
            SDL_FRect e2 = { textX, textY, 50, 15 }; SDL_RenderFillRect(renderer, &e2);
            SDL_FRect e3 = { textX, textY + 22, 40, 15 }; SDL_RenderFillRect(renderer, &e3);
            SDL_FRect e4 = { textX, textY + 45, 50, 15 }; SDL_RenderFillRect(renderer, &e4);

            textY += 80;
            textX = SCREEN_WIDTH / 2 - 200;
            SDL_FRect o1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &o1);
            SDL_FRect o2 = { textX + 35, textY, 15, 60 }; SDL_RenderFillRect(renderer, &o2);
            SDL_FRect o3 = { textX, textY, 50, 15 }; SDL_RenderFillRect(renderer, &o3);
            SDL_FRect o4 = { textX, textY + 45, 50, 15 }; SDL_RenderFillRect(renderer, &o4);

            textX += 70;
            SDL_FRect v1 = { textX, textY, 15, 40 }; SDL_RenderFillRect(renderer, &v1);
            SDL_FRect v2 = { textX + 35, textY, 15, 40 }; SDL_RenderFillRect(renderer, &v2);
            SDL_FRect v3 = { textX + 10, textY + 40, 30, 20 }; SDL_RenderFillRect(renderer, &v3);

            textX += 70;
            SDL_FRect e5 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &e5);
            SDL_FRect e6 = { textX, textY, 50, 15 }; SDL_RenderFillRect(renderer, &e6);
            SDL_FRect e7 = { textX, textY + 22, 40, 15 }; SDL_RenderFillRect(renderer, &e7);
            SDL_FRect e8 = { textX, textY + 45, 50, 15 }; SDL_RenderFillRect(renderer, &e8);

            textX += 70;
            SDL_FRect r1 = { textX, textY, 15, 60 }; SDL_RenderFillRect(renderer, &r1);
            SDL_FRect r2 = { textX, textY, 40, 15 }; SDL_RenderFillRect(renderer, &r2);
            SDL_FRect r3 = { textX + 25, textY, 15, 30 }; SDL_RenderFillRect(renderer, &r3);
            SDL_FRect r4 = { textX, textY + 22, 40, 15 }; SDL_RenderFillRect(renderer, &r4);
            SDL_FRect r5 = { textX + 25, textY + 30, 15, 30 }; SDL_RenderFillRect(renderer, &r5);
        }

        SDL_RenderPresent(renderer);
    }

    void HandleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) isRunning = false;
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_TAB) ToggleMode();
                if (event.key.key == SDLK_T) ToggleMonsterMode();
                if (event.key.key == SDLK_KP_PLUS || event.key.key == SDLK_EQUALS) ModifyObjectCount(1000);
                if (event.key.key == SDLK_KP_MINUS || event.key.key == SDLK_MINUS) ModifyObjectCount(-1000);
                if (event.key.key == SDLK_N) ModifyMonsterCount(5);
                if (event.key.key == SDLK_M) ModifyMonsterCount(-5);
                if (event.key.key == SDLK_R && (gameOver || levelComplete)) {
                    for (auto* obj : obiecteJoc) delete obj;
                    obiecteJoc.clear();
                    for (auto* m : monstruOOP) delete m;
                    monstruOOP.clear();
                    jucatorRef = nullptr;
                    scor = 0;
                    gameOver = false;
                    levelComplete = false;
                    LoadLevel();
                }
            }
        }
    }

    void UpdateTitle(float fps) {
        std::string title = "Phase 3 | FPS: " + std::to_string((int)fps) + " | HP: " + std::to_string(jucatorRef ? jucatorRef->hp : 0) + "/" + std::to_string(jucatorRef ? jucatorRef->maxHp : 0) + " | Score: " + std::to_string(scor);
        if (gameOver) title += " | GAME OVER - Press R";
        if (levelComplete) title += " | LEVEL COMPLETE! - Press R";
        title += " | Particles:" + std::string(useDOD ? "DOD" : "OOP") + "(" + std::to_string(useDOD ? particlesDOD.count : particlesOOP.size()) + ")";
        title += " | Monsters:" + std::string(useDODMonstri ? "DOD" : "OOP") + "(" + std::to_string(useDODMonstri ? monstruDOD.count : monstruOOP.size()) + ")";
        SDL_SetWindowTitle(window, title.c_str());
    }

    void Clean() {
        for (auto* obj : obiecteJoc) delete obj;
        for (auto* p : particlesOOP) delete p;
        for (auto* m : monstruOOP) delete m;
        obiecteJoc.clear(); particlesOOP.clear(); monstruOOP.clear();
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
    }
    bool Running() { return isRunning; }
};

int main(int argc, char* argv[]) {
    GameEngine engine;
    if (!engine.Init()) return -1;
    engine.LoadLevel();
    Uint64 lastPerf = SDL_GetPerformanceCounter();
    float fpsTimer = 0.0f, frameCount = 0.0f;

    while (engine.Running()) {
        Uint64 nowPerf = SDL_GetPerformanceCounter();
        float deltaTime = (float)(nowPerf - lastPerf) / (float)SDL_GetPerformanceFrequency();
        lastPerf = nowPerf;
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        fpsTimer += deltaTime;
        frameCount++;
        if (fpsTimer >= 1.0f) {
            engine.UpdateTitle(frameCount / fpsTimer);
            fpsTimer = 0; frameCount = 0;
        }

        engine.HandleEvents();
        engine.Update(deltaTime);
        engine.Render();
    }
    engine.Clean();
    return 0;
}