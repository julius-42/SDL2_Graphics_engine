/*
Graphics engine in C++
Author: Julius Kundrat
Date: 6.5.2026
Compiled: g++ -std=c++20 engine.cpp -o engine -lSDL2
*/

#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

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

struct Obj{
    vector<Position> vs;
    vector<pair<int, int>> es;
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

Position rotateYZ(Position pos, Position center, float angle){
    // move to origin
    float y = pos.Y - center.Y;
    float z = pos.Z - center.Z;

    float c = cos(angle);
    float s = sin(angle);

    // rotate, then move back
    return {
        pos.X,
        (y*c - z*s) + center.Y,
        (y*s + z*c) + center.Z
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

    // apply XZ,YZ rotation to all vertices
    for(auto& v : vs){
        v = rotateXZ(v, pos, rot.Y);
        v = rotateYZ(v, pos, rot.X);
    }


    for(auto& edge : es){
        Position& a = vs[edge[0]];
        Position& b = vs[edge[1]];
        Point p1 = project_point(rndr, a, clr);
        Point p2 = project_point(rndr, b, clr);
        SDL_RenderDrawLine(rndr, p1.X, p1.Y, p2.X, p2.Y);
    }
}

void project_obj(SDL_Renderer* rndr, Obj obj, Position pos, Rotation rot, Color clr){

    vector<Position> transformed_vs = obj.vs;

    // apply XZ,YZ rotation to all vertices
    for(auto& v : transformed_vs){
        v = {v.X+pos.X, v.Y+pos.Y, v.Z+pos.Z};
        v = rotateXZ(v, pos, rot.Y);
        v = rotateYZ(v, pos, rot.X);
    }

    for(auto& edge : obj.es){
        Position& a = transformed_vs[edge.first];
        Position& b = transformed_vs[edge.second];

        Point p1 = project_point(rndr, a, clr);
        Point p2 = project_point(rndr, b, clr);

        SDL_RenderDrawLine(rndr, p1.X, p1.Y, p2.X, p2.Y);
    }
}

Obj parse_obj_file(char* file_name){
    Obj obj;

    ifstream file(file_name);
    string line;
    
    while (getline(file, line)){
        stringstream ss(line);
        string prefix;

        if(ss >> prefix){

            if(prefix == "v"){  
                Position vertex;

                if(ss >> vertex.X >> vertex.Y >> vertex.Z){
                    obj.vs.push_back(vertex);
                    cout << "Pushed vertex: " << vertex.X << " " << vertex.Y << " " << vertex.Z << "\n";
                }
            }

            else if(prefix == "l"){
                pair<int, int> edge;

                if(ss >> edge.first >> edge.second){
                    edge.first -= 1;
                    edge.second -= 1;
                    obj.es.push_back(edge);
                    cout << "Pushed edge: " << edge.first << " " << edge.second << "\n";
                }
            }
        }
    }

    return obj;
}


int main(int argc, char** argv){

    Color black = {0,0,0};
    Color green = {0,255,0};
    bool running = true;
    SDL_Event e;
    Position obj_pos = {0,0,5};
    Rotation obj_rot = {0,0};

    Obj pyramid = parse_obj_file(argv[1]);

    SDL_Window* win = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &win, &renderer);

    while(running){

        // closes the window when 'X' pressed
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT)
                running = false;
            // Controls for movement and rotation of the object
            if(e.type == SDL_KEYDOWN){
                switch (e.key.keysym.sym){
                    case SDLK_s: obj_pos.Y += 0.3f; break;
                    case SDLK_w: obj_pos.Y -= 0.3f; break;
                    case SDLK_d: obj_pos.X += 0.3f; break;
                    case SDLK_a: obj_pos.X -= 0.3f; break;
                    case SDLK_e: obj_pos.Z += 0.3f; break;
                    case SDLK_q: obj_pos.Z -= 0.3f; break;
                    case SDLK_DOWN: obj_rot.X += 0.1f; break;
                    case SDLK_UP: obj_rot.X -= 0.1f; break;
                    case SDLK_RIGHT: obj_rot.Y += 0.1f; break;
                    case SDLK_LEFT: obj_rot.Y -= 0.1f; break;
                }
            }
        }

        // fills window with color
        SDL_SetRenderDrawColor(renderer, black.R, black.G, black.B, 255);
        SDL_RenderClear(renderer);

        // center point for reference
        SDL_SetRenderDrawColor(renderer, green.R, green.G, green.B, 255);
        SDL_RenderDrawPoint(renderer, WIDTH/2, HEIGHT/2);

        //project_cube(renderer, obj_pos, obj_rot, 8, green);
        project_obj(renderer, pyramid, obj_pos, obj_rot, green);
        

        SDL_RenderPresent(renderer);

        SDL_Delay(10);

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}