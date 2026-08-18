#include <SDL3/SDL.h>
#include <cstdio>

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window* win = SDL_CreateWindow("Hello SDL3", 800, 600, 0);
    if (!win) {
        printf("CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    printf("Window created OK!\n");
    
    SDL_Delay(2000);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}