#include "raylib.h"

struct Window 
{
    static constexpr int width = 900;
    static constexpr int height = 540;
    static constexpr int fps = 60;
    const char *title = "Slingshot";
};

struct Block 
{
    float x, y, w, h;
    float rotation = 0;

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

void update(Block& block);
void handleInput(Block& block);

int main() 
{
    Window win;

    InitWindow(win.width, win.height, win.title);
    SetTargetFPS(win.fps);

    Block block { win.width / 2.0f, win.height / 2.0f, 30, 30 };

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        update(block);
        block.draw();

        EndDrawing();
    }

    return 0;
}

void update(Block& block)
{
    handleInput(block);
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
