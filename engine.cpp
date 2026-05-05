/*
Graphics engine in C++
Author: Julius Kundrat
Date: 6.5.2026
Compiled: g++ -std=c++20 engine.cpp -o engine -lSDL2
*/

#include <SDL2/SDL.h>

#define HEIGHT 720
#define WIDTH 720


class Color{
    public:
        unsigned R, G, B;
};

class Point{
    public:
        int X,Y;
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
Point project_point(SDL_Renderer* rndr, float x, float y, float z, Color clr){
    float fov = 100;
    float pX = (x/z * fov)+WIDTH/2;
    float pY = (y/z * fov)+HEIGHT/2;

    draw_sphere(rndr, (int)(pX), (int)(pY), 4, clr);

    Point p = {(int)pX,(int)pY};
    return p;
}


void project_cube(SDL_Renderer* rndr, float x, float y, float z, float size, Color clr){
    float r = size/2;

    // 8 corners
    float pts[8][3] = {
        {x-r, y-r, z-r}, {x+r, y-r, z-r},
        {x+r, y+r, z-r}, {x-r, y+r, z-r},  // front face
        {x-r, y-r, z+r}, {x+r, y-r, z+r},
        {x+r, y+r, z+r}, {x-r, y+r, z+r},  // back face
    };

    // 12 edges: front, back, connecting sides
    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},  // front face
        {4,5},{5,6},{6,7},{7,4},  // back face
        {0,4},{1,5},{2,6},{3,7},  // connecting edges
    };

    for(auto& edge : edges){
        auto* a = pts[edge[0]];
        auto* b = pts[edge[1]];
        Point p1 = project_point(rndr, a[0],a[1],a[2], clr);
        Point p2 = project_point(rndr, b[0],b[1],b[2], clr);
        SDL_RenderDrawLine(rndr, p1.X, p1.Y, p2.X, p2.Y);
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
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &win, &renderer);

    while(running){

        // closes the window when 'X' pressed
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT)
                running = false;
        }

        // fills window with color
        SDL_SetRenderDrawColor(renderer, black.R, black.G, black.B, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, green.R, green.G, green.B, 255);
        SDL_RenderDrawPoint(renderer, WIDTH/2, HEIGHT/2);

        project_cube(renderer, 0, 0, 8, 10, green);
        

        SDL_RenderPresent(renderer);

        SDL_Delay(10);

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}