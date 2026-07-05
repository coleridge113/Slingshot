#include "raylib.h"
#include "raymath.h"
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
    bool flying = false;
    float velocity = 0.f;
    struct 
    {
        Vector2 origin = { 200, 375 };
        double radius = 100;
    } anchor;

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

void update(Window& win, Block& block, Block& anch);
void handleInput(Block& block);
void followMouse(Block& block);
void pickup(Block& block);
void applyGravity(Block& block);
double getDistance(Block& a, Block& b);

int main() 
{
    Window win;

    InitWindow(win.width, win.height, win.title);
    SetTargetFPS(win.fps);

    std::vector<Block> blocks;
    Block block { 200, 375, 30, 30 };
    Block anch { 200, 375, 5, 5 };

    Font font = GetFontDefault();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        {
            double dst = getDistance(anch, block);
            DrawTextEx(font, TextFormat("D: %.f", dst), {5,5}, 32, 2, WHITE);
        }
        
        update(win, block, anch);
        block.draw();
        anch.draw();

        EndDrawing();
    }

    return 0;
}

void update(Window& win, Block& block, Block& anch)
{
    handleInput(block);
    pickup(block);

    if (block.y <= win.floor && block.flying)
    {
        applyGravity(block);
    }

    Vector2 anchorPos = { anch.x, anch.y };
    Vector2 blockPos  = { block.x, block.y };

    double dst = Vector2Distance(anchorPos, blockPos);
    float maxRadius = anch.anchor.radius;

    if (!block.flying && dst > maxRadius)
    {
        Vector2 direction = Vector2Subtract(blockPos, anchorPos);
        Vector2 normalizedDir = Vector2Normalize(direction);
        Vector2 restrictedOffset = Vector2Scale(normalizedDir, maxRadius);
        block.x = anchorPos.x + restrictedOffset.x;
        block.y = anchorPos.y + restrictedOffset.y;
        block.velocity = 0.f;
    }
    else if (!block.flying && dst != 0)
    {

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
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        block.flying = false;
        block.velocity = 0;
        block.x = GetMouseX();
        block.y = GetMouseY();
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        block.flying = true;
    }
}

constexpr float gravity = 0.5f;
void applyGravity(Block& block)
{
    block.velocity += gravity;
    block.y += block.velocity;
}

double getDistance(Block& a, Block& b)
{
    return 
        std::sqrt(
            std::pow(a.x - b.x, 2) +
            std::pow(a.y - b.y, 2)
        );
}
