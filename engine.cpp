/*
Graphics engine in C++
Author: Julius Kundrat
Date: 6.5.2026
Compiled: g++ -std=c++20 engine.cpp -o engine -lSDL2 -lSDL2_ttf
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <unordered_set>

using namespace std;

#define HEIGHT 720
#define WIDTH 720


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

struct Text{
    SDL_Texture *texture;
    SDL_Rect rect;
};

void draw_sphere(SDL_Renderer* rndr, int x, int y, int r, SDL_Color clr){

    SDL_SetRenderDrawColor(rndr, clr.r, clr.g, clr.b, 255);

    for(int X = -r; X <= r; X++){
        for(int Y = -r; Y <= r; Y++){
            if((X*X + Y*Y) <= r*r) SDL_RenderDrawPoint(rndr, X + x, Y + y);
        }    
    }
}


// initial cords are 0..x 0..y -> projection uses -x/2..x/2  -y/2..y/2
// -x/2..x/2 +x/2 0..x
Point position_to_point(Position pos){
    const float fov = 200.0f;

    float pX = fov*(pos.X/pos.Z)+WIDTH/2;
    float pY = fov*(pos.Y/pos.Z)+HEIGHT/2;
    
    return {(int)pX,(int)pY};
}

void project_point(SDL_Renderer* rndr, Point p, SDL_Color clr){
    draw_sphere(rndr, (p.X), (p.Y), 3, clr);
}

Position rotateXZYZ(Position pos, float sin_X, float cos_X, float sin_Y, float cos_Y){

    float x1 = pos.X*cos_Y - pos.Z*sin_Y;
    float z1 = pos.X*sin_Y + pos.Z*cos_Y;

    float y1 = pos.Y*cos_X - z1*sin_X;
    float z2 = pos.Y*sin_X + z1*cos_X;

    return {x1, y1, z2};
}  

int is_in_bounds(Point p){
    if(p.X < 0 || p.X > WIDTH || p.Y < 0 || p.Y > HEIGHT){
        return 0;
    }
    return 1;
}

int project_obj(SDL_Renderer* rndr, const Obj& obj, Position pos, Rotation rot, SDL_Color clr){

    size_t vertices_num = obj.vs.size();
    vector<Position> transformed_vs(vertices_num);
    vector<Point> ps(vertices_num);
    vector<bool> is_p_valid(vertices_num, false);
    int projected = 0;

    SDL_SetRenderDrawColor(rndr, clr.r, clr.g, clr.b, 255);

    for(size_t i = 0; i < vertices_num; i++){

        Position v = obj.vs[i];

        // transform the original vertex (rotation, position)
        v = rotateXZYZ(v, sin(rot.X), cos(rot.X), sin(rot.Y), cos(rot.Y));
        v = {v.X+pos.X, v.Y+pos.Y, v.Z+pos.Z};
        transformed_vs[i] = v;
        
        // convert position to point only if it's on the visible side of the screen
        if(v.Z > 0){
            Point p = position_to_point(v);
            ps[i] = p;

            // project a point only if it's in screen bounds
            if(is_in_bounds(p)){
                project_point(rndr, p, clr);
                projected += 1;
            }
        }
    }

    for(const auto& edge : obj.es){
        // draw line only if both vertices are on the visible side of the screen
        if(transformed_vs[edge.first].Z > 0 && transformed_vs[edge.second].Z > 0){
            SDL_RenderDrawLine(rndr, ps[edge.first].X, ps[edge.first].Y, 
                                     ps[edge.second].X, ps[edge.second].Y);
        }
    }

    return projected;
}


Obj parse_obj_file(char* file_name){
    Obj obj;

    ifstream file(file_name);
    string line;
    int pushed_vs = 0;
    int pushed_es = 0;
    
    while (getline(file, line)){
        stringstream ss(line);
        string prefix;

        if(ss >> prefix){

            if(prefix == "v"){  
                Position vertex;

                if(ss >> vertex.X >> vertex.Y >> vertex.Z){
                    obj.vs.push_back(vertex);
                    pushed_vs += 1;
                }
            }

            else if(prefix == "l"){
                pair<int, int> edge;

                if(ss >> edge.first >> edge.second){
                    edge.first -= 1;
                    edge.second -= 1;
                    obj.es.push_back(edge);
                    pushed_es += 1;
                }
            }

            else if(prefix == "f"){
                int f1, f2, f3;

                if(ss >> f1 >> f2 >> f3){
                    obj.es.push_back({f1-1,f2-1});
                    obj.es.push_back({f2-1,f3-1});
                    obj.es.push_back({f3-1,f1-1});

                    pushed_es += 3;
                }
            }
        }
    }
    cout << "Pushed " << pushed_vs << " vertices\n";
    cout << "Pushed " << pushed_es << " edges\n";

    return obj;
}

Text create_text(SDL_Renderer *rndr, TTF_Font *font, string str, SDL_Color clr, int w = 50, int h = 70){
    SDL_Surface *surface = TTF_RenderText_Solid(font, str.c_str(), clr);
    SDL_Texture *text = SDL_CreateTextureFromSurface(rndr, surface);
    SDL_FreeSurface(surface);
    SDL_Rect dst = {0,0, (int)(w*str.size()), h};

    return Text{text, dst};
}


int main(int argc, char** argv){

    SDL_Color black = {0,0,0};
    SDL_Color green = {0,255,0};
    bool running = true;
    bool print_status = false;
    SDL_Event e;
    Position obj_pos = {0,0,5};
    Rotation obj_rot = {0,0};
    int projected_points = 0;

    Obj pyramid = parse_obj_file(argv[1]);

    SDL_Window* win = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDL_Init(SDL_INIT_VIDEO);
    if (TTF_Init() < 0){
        cout << "Error initializing  SDL_ttf: " << TTF_GetError();
    }
    
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &win, &renderer);

    TTF_Font *font = TTF_OpenFont("DroidSansMono.ttf", 30);
    Text mode_text = create_text(renderer, font, "wireframe", green);

    while(running){

        // closes the window when 'X' pressed
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT)
                running = false;
            // Controls for movement and rotation of the object
            if(e.type == SDL_KEYDOWN){
                print_status = true;
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
        SDL_SetRenderDrawColor(renderer, black.r, black.g, black.b, 255);
        SDL_RenderClear(renderer);
        
        // center point for reference
        SDL_SetRenderDrawColor(renderer, green.r, green.b, green.b, 255);
        SDL_RenderDrawPoint(renderer, WIDTH/2, HEIGHT/2);

        //rendering mode text
        mode_text.rect.x = WIDTH/2 - (mode_text.rect.w/2);
        mode_text.rect.y = HEIGHT*0.9;
        SDL_RenderCopy(renderer, mode_text.texture, NULL, &mode_text.rect);
        
        projected_points = project_obj(renderer, pyramid, obj_pos, obj_rot, green);
        
        if(print_status){
            cout << "Projected: " << projected_points << " points\n";
            print_status = false;
        }
        
        SDL_RenderPresent(renderer);

        SDL_Delay(10);

    }

    SDL_DestroyTexture(mode_text.texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}