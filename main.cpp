#include "raylib.h"
#include <cmath>
#include <vector>

struct Window 
{
    static constexpr int width = 900;
    static constexpr int height = 540;
    static constexpr int fps = 60;
    static constexpr float floor = 450;
    const char *title = "Slingshot";
};

struct Block 
{
    float x, y, w, h;
    float rotation = 0;
    bool dynamic = true;
    float velocity = 0.f;

    Rectangle getRect() const { return Rectangle { x, y, w, h}; }
    void draw() const 
    { 
        DrawRectanglePro(
            getRect(), 
            { w / 2, h / 2},
            rotation,
            WHITE
        ); 
    }
};

void update(Window& win, Block& block);
void handleInput(Block& block);
void followMouse(Block& block);
void pickup(Block& block);
void applyGravity(Block& block);

int main() 
{
    Window win;

    InitWindow(win.width, win.height, win.title);
    SetTargetFPS(win.fps);

    std::vector<Block> blocks;
    Block block { win.width / 2.0f, win.height / 1.35f, 30, 30 };

    

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        
        update(win, block);
        block.draw();

        EndDrawing();
    }

    return 0;
}

void update(Window& win, Block& block)
{
    handleInput(block);
    // followMouse(block);
    pickup(block);

    if (block.y <= win.floor && block.dynamic)
    {
        applyGravity(block);
    }
}

void handleInput(Block& block)
{
    if (IsKeyDown(KEY_J))
    {
        block.rotation -= 1;
    }
    if (IsKeyDown(KEY_L))
    {
        block.rotation += 1;
    }
}


void followMouse(Block& block)
{
    Vector2 mousePos = GetMousePosition();
    
    float dx = mousePos.x - block.x;
    float dy = mousePos.y - block.y;
    float angle = std::atan2(dy, dx);
    block.rotation = angle * RAD2DEG; 
}

void pickup(Block& block)
{
    block.dynamic = true;
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        block.dynamic = false;
        block.velocity = 0;
        block.x = GetMouseX();
        block.y = GetMouseY();
    }
}

constexpr float gravity = 0.5f;
void applyGravity(Block& block)
{
    block.velocity += gravity;
    block.y += block.velocity;
}
