/*
Graphics engine in C++
Author: Julius Kundrat
Date: 6.5.2026
Compiled: g++ -std=c++20 engine.cpp -o engine -lSDL2
*/

#include <SDL2/SDL.h>
#include <iostream>

using namespace std;

#define HEIGHT 720
#define WIDTH 720


struct Color{
    unsigned R, G, B;
};

struct Point{
    int X,Y;
};

struct Position{
    float X,Y,Z;
};

struct Rotation{
    float X,Y;
};

void draw_sphere(SDL_Renderer* rndr, int x, int y, int r, Color clr){

    SDL_SetRenderDrawColor(rndr, clr.R, clr.G, clr.B, 255);

    for(int X = -r; X <= r; X++){
        for(int Y = -r; Y <= r; Y++){
            if((X*X + Y*Y) <= r*r) SDL_RenderDrawPoint(rndr, X + x, Y + y);
        }    
    }
}

// initial cords are 0..x 0..y -> projection uses -x/2..x/2  -y/2..y/2
// -x/2..x/2 +x/2 0..x
Point project_point(SDL_Renderer* rndr, Position pos, Color clr){

    const float fov = 200.0f;

    float pX = fov*(pos.X/pos.Z)+WIDTH/2;
    float pY = fov*(pos.Y/pos.Z)+HEIGHT/2;

    draw_sphere(rndr, (int)(pX), (int)(pY), 3, clr);

    return {(int)pX,(int)pY};
}

Position rotateXZ(Position pos, Position center, float angle){
    // move to origin
    float x = pos.X - center.X;
    float z = pos.Z - center.Z;

    float c = cos(angle);
    float s = sin(angle);

    // rotate, then move back
    return {
        (x*c - z*s) + center.X,
        pos.Y,
        (x*s + z*c) + center.Z
    };
}  


void project_cube(SDL_Renderer* rndr, Position pos, Rotation rot,float size, Color clr){
    float r = size/2;

    // 8 corners
    Position vs[8] = {
        {pos.X-r, pos.Y-r, pos.Z-r}, {pos.X+r, pos.Y-r, pos.Z-r},
        {pos.X+r, pos.Y+r, pos.Z-r}, {pos.X-r, pos.Y+r, pos.Z-r},  // front face

        {pos.X-r, pos.Y-r, pos.Z+r}, {pos.X+r, pos.Y-r, pos.Z+r},
        {pos.X+r, pos.Y+r, pos.Z+r}, {pos.X-r, pos.Y+r, pos.Z+r}, // back face
    };

    // 12 edges: front, back, connecting sides
    int es[12][2] = {
        {0,1},{1,2},{2,3},{3,0},  // front face
        {4,5},{5,6},{6,7},{7,4},  // back face
        {0,4},{1,5},{2,6},{3,7},  // connecting edges
    };

    for(auto& v : vs){
        v = rotateXZ(v, pos, rot.Y);
    }


    for(auto& edge : es){
        Position& a = vs[edge[0]];
        Position& b = vs[edge[1]];
        Point p1 = project_point(rndr, a, clr);
        Point p2 = project_point(rndr, b, clr);
        SDL_RenderDrawLine(rndr, p1.X, p1.Y, p2.X, p2.Y);
    }
}

int main(){

    Color black = {0,0,0};
    Color green = {0,255,0};
    bool running = true;
    SDL_Event e;
    Position cube_pos = {0,0,15};
    Rotation cube_rot = {0,0};

    SDL_Window* win = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &win, &renderer);

    while(running){

        // closes the window when 'X' pressed
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT)
                running = false;
            if(e.type == SDL_KEYDOWN){
                switch (e.key.keysym.sym){
                    case SDLK_s: cube_pos.Y += 0.3f; break;
                    case SDLK_w: cube_pos.Y -= 0.3f; break;
                    case SDLK_d: cube_pos.X += 0.3f; break;
                    case SDLK_a: cube_pos.X -= 0.3f; break;
                    case SDLK_e: cube_pos.Z += 0.3f; break;
                    case SDLK_q: cube_pos.Z -= 0.3f; break;
                    case SDLK_UP: cube_rot.X += 0.1f; break;
                    case SDLK_DOWN: cube_rot.X -= 0.1f; break;
                    case SDLK_RIGHT: cube_rot.Y += 0.1f; break;
                    case SDLK_LEFT: cube_rot.Y -= 0.1f; break;
                }
            }
        }

        // fills window with color
        SDL_SetRenderDrawColor(renderer, black.R, black.G, black.B, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, green.R, green.G, green.B, 255);
        SDL_RenderDrawPoint(renderer, WIDTH/2, HEIGHT/2);

        project_cube(renderer, cube_pos, cube_rot, 8, green);
        

        SDL_RenderPresent(renderer);

        SDL_Delay(10);

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}