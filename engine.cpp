/*
Graphics engine in C++
Author: Julius Kundrat
Date: 6.5.2026
Compiled: g++ -std=c++20 engine.cpp -o engine -lSDL2
*/

#include <SDL2/SDL.h>

class Color{
    public:
        unsigned R, G, B;
};

void draw_sphere(SDL_Renderer* rndr, int x, int y, int r, Color clr){

    SDL_SetRenderDrawColor(rndr, clr.R, clr.G, clr.B, 255);

    for(int X = -r; X <= r; X++){
        for(int Y = -r; Y <= r; Y++){
            if((X*X + Y*Y) <= r*r) SDL_RenderDrawPoint(rndr, X + x, Y + y);
        }    
    }
}


int main(){

    Color black = {0,0,0};
    Color green = {0,255,0};
    bool running = true;
    SDL_Event e;

    SDL_Window* win = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(640, 480, 0, &win, &renderer);

    while(running){

        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT)
                running = false;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_sphere(renderer, 320, 240, 5, green);

        SDL_RenderPresent(renderer);

        SDL_Delay(10);

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}