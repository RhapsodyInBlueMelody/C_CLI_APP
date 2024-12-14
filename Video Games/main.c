#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

typedef struct {
    int x, y, w, h;
    bool active;
} GameObject;

void movePlayer(GameObject *player, SDL_Event event, SDL_Window *window) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) {
            if (player->x > 0) {
                player->x -= 5;
            }
        }
        if (event.key.keysym.sym == SDLK_RIGHT) {
            if (player->x + player->w < w) {
                player->x += 5;
            }
        }
    }
}

void firing(SDL_Renderer *renderer,const GameObject *laser){
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect laserRect = {laser->x, laser->y, laser->w, laser->h};
    SDL_RenderFillRect(renderer, &laserRect);
}

void drawObjectAlly(SDL_Renderer *renderer, GameObject obj) {
    SDL_Point points[] = {
        {obj.x, obj.y},                      // Top point
        {obj.x - obj.w/2, obj.y + obj.h},    // Bottom left
        {obj.x, obj.y + obj.h*2/3},          // Bottom middle point
        {obj.x + obj.w/2, obj.y + obj.h},    // Bottom right
        {obj.x, obj.y}                       // Back to top to close shape
    };
    SDL_SetRenderDrawColor(renderer, 50, 255, 50, 255);
    SDL_RenderDrawLines(renderer, points, 5);
}

void drawObjectEnemy(SDL_Renderer *renderer, GameObject obj) {
    SDL_Rect rect = {obj.x, obj.y, obj.w, obj.h};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);
}

bool checkCollision(GameObject a, GameObject b) {
    // Get the sides of rectangle A
    int leftA = a.x;
    int rightA = a.x + a.w;
    int topA = a.y;
    int bottomA = a.y + a.h;

    // Get the sides of rectangle B
    int leftB = b.x;
    int rightB = b.x + b.w;
    int topB = b.y;
    int bottomB = b.y + b.h;

    // Check if any of the sides from A are outside of B
    if (bottomA <= topB) return false;
    if (topA >= bottomB) return false;
    if (rightA <= leftB) return false;
    if (leftA >= rightB) return false;

    // If none of the sides from A are outside B, they are colliding
    return true;
}

void handleCollisions(GameObject *lasers, GameObject *enemies, int laserCount, int enemyCount) {
    for (int i = 0; i < laserCount; i++) {
        if (!lasers[i].active) continue;

        for (int j = 0; j < enemyCount; j++) {
            if (!enemies[j].active) continue;

            if (checkCollision(lasers[i], enemies[j])) {
                // Collision detected!
                lasers[i].active = false;   // Deactivate laser
                enemies[j].active = false;  // Destroy enemy
                break;
            }
        }
    }
}

void renderScene(SDL_Renderer *renderer, const GameObject *player, const GameObject *lasers, GameObject *enemies, int laserCount, int enemyCount) {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 100);
    SDL_RenderClear(renderer);

    // render player
    drawObjectAlly(renderer, *player);

    // render musuh with spacing
    int spacing = 40; // Space between enemies
    int startX = 20; // Starting X position
    int startY = 30; // Starting Y position

    for(int i = 0; i < enemyCount; i++){
        if (enemies[i].active){
            enemies[i].x = startX + (i * spacing);
            enemies[i].y = startY;
            drawObjectEnemy(renderer, enemies[i]);
        }
    }

    // Render semua laser
    for (int i = 0; i < laserCount; i++) {
        if (lasers[i].active) {
            firing(renderer, &lasers[i]);
        }
    }
    SDL_RenderPresent(renderer);
}

void playerFire(GameObject *player, SDL_Event event, GameObject *lasers, int laserCount) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_x) {
        for (int i = 0; i < laserCount; i++) {
            if (!lasers[i].active) { // mencari laser yang tidak aktif
                lasers[i].x = player->x + (player->w / 2) - (lasers[i].w / 2);
                lasers[i].y = player->y;
                lasers[i].w = 4;
                lasers[i].h = 12;
                lasers[i].active = true;
                break;
            }
        }
    }

    // Update laser positions
    for (int i = 0; i < laserCount; i++) {
        if (lasers[i].active) {
            lasers[i].y -= 10;
            if (lasers[i].y < -lasers[i].h) {
                lasers[i].active = false; // menghilang jika lebih dari window
            }
        }
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Simple Game",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          256, 240, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    GameObject lasers[20]; // banyaknya laser
    GameObject enemies[6]; // banyaknya musuh
    int countofLaser = 20;
    int countofEnemies = 6;

    // menginisialisasi musuh dengan status aktif
    for(int i = 0; i < countofEnemies; i++) {
        enemies[i].w = 16;
        enemies[i].h = 16;
        enemies[i].x = 20 + (i * 40);
        enemies[i].y = 30;
        enemies[i].active = true;
    }

    // Initialize lasers
    for(int i = 0; i < countofLaser; i++) {
        lasers[i].active = false;
    }

    GameObject player = {120, 200, 16, 16, true}; // Added active field

    bool running = true;
    SDL_Event event = {0}; // Initialize event

    Uint32 previousTime = SDL_GetTicks();
    const int FPS = 60;
    const int framedelay= 1000/FPS;

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - previousTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            movePlayer(&player, event, window);
            playerFire(&player, event, lasers, 20);
        }

        if(deltaTime >= framedelay){
            previousTime = currentTime;

        for(int i = 0;i < 20; i++){
            if(lasers[i].active){
                lasers[i].y -= 5;
                if(lasers[i].y < -lasers[i].h) lasers[i].active = false;
            }
        }
        handleCollisions(lasers, enemies, countofLaser, countofEnemies);

        renderScene(renderer, &player, lasers, enemies, countofLaser, countofEnemies);
        }

        SDL_Delay(1);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
