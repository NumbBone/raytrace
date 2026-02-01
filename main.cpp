#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <tuple>

const int GRID_COLS =11;
const int GRID_ROWS =11;

const float FOV           =PI / 2;
const float DIST_FROM_CAM =1;
const float EPSI = 1e-3;

const int SCREEN_WIDTH = 100;
const float FAR_PLANE = 10.0;

int map[GRID_ROWS][GRID_COLS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

Color backround{
    .r = 25,
    .g = 25,
    .b = 25,
    .a = 255,
};


Vector2 fromDirection(float angle) {
    return Vector2Normalize(Vector2{cosf(angle), sinf(angle)});
};

bool inMap(Vector2 c)
{
    return (c.x >= GRID_COLS || c.x < 0 || c.y >= GRID_ROWS || c.y < 0);
};

float snap(float x, float dx) 
{

    if (dx > 0) return ceil(x + EPSI);
    if (dx < 0) return floor(x - EPSI);
    return x;
};

Vector2 Cell_snap(Vector2 p1 , Vector2 p2) 
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;

    Vector2 Cell = {
        floor(p2.x + copysign(p2.x, dx) * EPSI),
        floor(p2.y + copysign(p2.y, dy) * EPSI),
    };


    return Cell;
};

Vector2 world_to_screen(Vector2 world_pos, Vector2 offset, float scale) {
    float cell_width = ((float)GetScreenWidth() * scale) / GRID_COLS;
    float cell_height = ((float)GetScreenHeight() * scale) / GRID_ROWS;
    return { offset.x + world_pos.x * cell_width, offset.y + world_pos.y * cell_height };
}

Vector2 screen_to_world(Vector2 screen_pos, Vector2 offset, float scale) {
    float cell_width = ((float)GetScreenWidth() * scale) / GRID_COLS;
    float cell_height = ((float)GetScreenHeight() * scale) / GRID_ROWS;
    return { (screen_pos.x - offset.x) / cell_width, (screen_pos.y - offset.y) / cell_height };
}

Vector2 Gen_NextPoint(Vector2 p1, Vector2 p2) 
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;

    Vector2 p3 = p2;

    if (dx != 0){

        float m = dy / dx;
        float c = p1.y - (p1.x * m);

        float x3 = snap(p2.x, dx);
        float y3 = x3 * m + c;
        Vector2 newPointX = {x3 ,y3};

        
        Vector2 newPointY = {snap(p2.x, dx), p2.y};
        if ( m != 0 ){
            float y4 = snap(p2.y , dy);
            float x4 = (y4 - c) / m;
            newPointY = {x4, y4};
        }


        if(Vector2Distance(p2, newPointX) < Vector2Distance(p2 , newPointY)){
            p3 = newPointX;
        } else {
            p3 = newPointY;
        } 

    } else {
        Vector2 newPoint = {p2.x, snap(p2.y, dy)};
        p3 = newPoint;
    }

    return p3;
};
class Player
{
    public:
        Vector2 posicion;
        float direction;

        std::tuple<Vector2, Vector2> fovRange(){

            Vector2 dir = fromDirection(this->direction);

            float halfPlane = tanf(FOV / 2);
            Vector2 plane = {-dir.y, dir.x};

            Vector2 focalPoint = this->posicion + dir * DIST_FROM_CAM;

            Vector2 p2 = focalPoint + plane * halfPlane;
            Vector2 p1 = focalPoint + plane * -halfPlane;

            return {p1, p2};
        }
};

Vector2 castRay(Vector2 p1, Vector2 p2) 
{
 for (;;)
    {
        Vector2 c = Cell_snap(p1, p2);
        if (map[(int)c.y][(int)c.x] == 1){
            return p2;
        }
        if (c.x >= GRID_COLS || c.x < 0 || c.y >= GRID_ROWS ||c.y < 0){
            return p2;
        }
        Vector2 pt = p2;
        p2 = Gen_NextPoint(p1, p2);
            p1 = pt;
    };
};

void render(Player *player) 
{
    const int stripWidth = GetScreenWidth() / SCREEN_WIDTH; 
    auto [lpoint, rpoint] = player->fovRange();

    for (size_t i = 0; i < SCREEN_WIDTH; i++)
    {
        Vector2 p1 = Vector2Lerp(lpoint, rpoint, (float)i / (float)SCREEN_WIDTH);
        Vector2 ray = castRay(player->posicion, p1);
        Vector2 cell = Cell_snap(player->posicion, ray);

        if (!inMap(cell) && map[(int)cell.y][(int)cell.x] != 0){
            float stripHeight = (1 / Vector2Distance(player->posicion, ray)) * GetScreenHeight();
            DrawRectangle(i * stripWidth, (GetScreenHeight() - stripHeight ) * 0.5,stripWidth, stripHeight, GREEN);
        }
    }
    

};

void minimap(Player *player,Vector2 offset, float scale)
{
    int width = GetScreenWidth()*scale;
    int height = GetScreenHeight()*scale;

    float Cell_width = (float)width / GRID_COLS;
    float Cell_height = (float)height / GRID_ROWS;
    for (int x = 0; x < GRID_COLS + 1; ++x)
    {
        DrawLine(offset.x + x * Cell_width, offset.y, offset.x + x * Cell_width, offset.y + height, GRAY);
    }
    for (int y = 0; y < GRID_COLS + 1; ++y)
    {
        DrawLine(offset.x, offset.y + y * Cell_height, offset.x + width,  offset.y + y * Cell_height, GRAY);
    }


    for (int y = 0; y <= GRID_COLS; y++)
    {
        for (int x = 0; x <= GRID_ROWS; x++)
        {
            if(map[y][x] == 1){
                Vector2 screenPos = world_to_screen({(float)x, (float)y}, offset, scale);
                DrawRectangle(screenPos.x, screenPos.y, Cell_width, Cell_height, RED);
            }
        }
        
    }
    

    DrawCircleV(world_to_screen(player->posicion, offset ,scale), 10, RED);

    Vector2 dir = fromDirection(player->direction);
    Vector2 focalPoint = player->posicion + dir * DIST_FROM_CAM;

    DrawLineEx(
        world_to_screen(player->posicion, offset, scale),
        world_to_screen(focalPoint, offset, scale), 
        3, RED
    );

    auto [lplane, rplane] = player->fovRange();

    DrawLineEx(
        world_to_screen(lplane, offset, scale),
        world_to_screen(rplane, offset, scale), 
        3, BLUE 
    );

};

int main(void) {

    InitWindow(900, 900, "raylib [core] example - basic window");
    SetTargetFPS(60);

    Player player = {
        {5.5, 5.5},
        -PI/4
    };
    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(backround);

            render(&player); 
            minimap(&player, Vector2{0,0},0.25);
            EndDrawing();
    };

    CloseWindow();
    return 0;
}