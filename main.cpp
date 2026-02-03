#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <tuple>

const int GRID_COLS =11;
const int GRID_ROWS =11;

const float FOV           =PI / 2;
const float DIST_FROM_CAM =1;
const float EPSI = 1e-3;

const int SCREEN_WIDTH = 200;

Color map[11][11] = {
    { {0}, {0},    {0},    {0}, {0}, {0}, {0},    {0},    {0},    {0},    {0} },
    { {0}, RED,    GOLD,   {0}, {0}, {0}, LIME,   {0},    {0},    {0},    {0} },
    { {0}, {0},    {0},    {0}, {0}, {0}, VIOLET, SKYBLUE, {0},    {0},    {0} },
    { {0}, {0},    {0},    {0}, {0}, {0}, {0},    ORANGE,  PINK,   {0},    {0} },
    { {0}, {0},    {0},    {0}, {0}, {0}, {0},    {0},    BLUE,   PURPLE, {0} },
    { {0}, {0},    {0},    {0}, {0}, {0}, {0},    {0},    {0},    {0},    {0} },
    { {0}, {0},    {0},    {0}, {0}, {0}, {0},    {0},    {0},    {0},    {0} },
    { {0}, {0},    {0},    {0}, {0}, {0}, {0},    {0},    {0},    {0},    {0} },
    { {0}, {0},    {0},    {0}, {0}, {0}, {0},    {0},    {0},    {0},    {0} },
    { {0}, {0},    {0},    {0}, {0}, {0}, {0},    {0},    {0},    {0},    {0} },
    { {0}, {0},    {0},    {0}, {0}, {0}, {0},    {0},    {0},    {0},    {0} }
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
        void playerUpdate(){
            if (IsKeyDown(KEY_A)){
                this->direction -= PI * 0.025;
            };

            if (IsKeyDown(KEY_D)){
                this->direction += PI * 0.025;
            };

            if (IsKeyDown(KEY_W)){
                this->posicion = this->posicion + fromDirection(this->direction) * 0.025;
            };
    
            if (IsKeyDown(KEY_S)){
                this->posicion = this->posicion - fromDirection(this->direction) * 0.025;
            };
        }
};

Vector2 castRay(Vector2 p1, Vector2 p2) 
{
 for (;;)
    {
        Vector2 c = Cell_snap(p1, p2);
        if (map[(int)c.y][(int)c.x].a != 0){
            return p2;
        }
        if ((c.x >= GRID_COLS || c.x < 0 || c.y >= GRID_ROWS ||c.y < 0)){
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
    Vector2 dir =fromDirection(player->direction);

    for (size_t i = 0; i < SCREEN_WIDTH; i++)
    {
        const Vector2 p1 = Vector2Lerp(lpoint, rpoint, (float)i / (float)SCREEN_WIDTH);
        const Vector2 ray = castRay(player->posicion, p1);
        const Vector2 cell = Cell_snap(player->posicion, ray);

        if (!inMap(cell) && map[(int)cell.y][(int)cell.x].a != 0){

            Vector2 rayVec = Vector2Subtract(ray, player->posicion);
            float perpDist = Vector2DotProduct(rayVec, dir);

            if (perpDist < 0.1f) perpDist = 0.1f;

            float stripHeight = (GetScreenHeight() / perpDist) * (DIST_FROM_CAM / (2.0f * tanf(FOV*0.5)));
            DrawRectangle(i * stripWidth, (GetScreenHeight() - stripHeight ) * 0.5,stripWidth + 1,
             stripHeight,
             map[(int)cell.y][(int)cell.x]
            );
        }
    }
    

};

void minimap(Player *player, Vector2 offset, float scale)
{
    float cellSize = ((float)GetScreenHeight() * scale) / GRID_ROWS;

    for (int y = 0; y < GRID_ROWS; y++) {
        for (int x = 0; x < GRID_COLS; x++) {
            Color cellColor = map[y][x];
            
            Vector2 drawPos = { offset.x + x * cellSize, offset.y + y * cellSize };

            if (cellColor.a != 0) {
                DrawRectangleV(drawPos, {cellSize, cellSize}, cellColor);
            } else {
                DrawRectangleLines(drawPos.x, drawPos.y, cellSize, cellSize, DARKGRAY);
            }
        }
    }

    Vector2 playerMinimapPos = {
        offset.x + player->posicion.x * cellSize,
        offset.y + player->posicion.y * cellSize
    };
    DrawCircleV(playerMinimapPos, 5, WHITE);

    Vector2 dir = fromDirection(player->direction);
    Vector2 viewLine = Vector2Add(playerMinimapPos, Vector2Scale(dir, cellSize));
    DrawLineEx(playerMinimapPos, viewLine, 2, YELLOW);

    auto [lplane, rplane] = player->fovRange();
    
    Vector2 lScreen = { offset.x + lplane.x * cellSize, offset.y + lplane.y * cellSize };
    Vector2 rScreen = { offset.x + rplane.x * cellSize, offset.y + rplane.y * cellSize };
    
    DrawLineEx(lScreen, rScreen, 2, BLUE);
}

int main(void) {

    InitWindow(1200, 900, "raylib [core] example - basic window");
    SetTargetFPS(60);

    Player player = {
        {5.5, 5.5},
        -PI/4
    };
    while (!WindowShouldClose()) {
        player.playerUpdate();

            BeginDrawing();
        ClearBackground(backround);

        render(&player);
        minimap(&player, Vector2{0, 0}, 0.35);
        EndDrawing();
    };

    CloseWindow();
    return 0;
}