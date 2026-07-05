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
    float xVel = 0.f;
    float yVel = 0.f;

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
            { w / 2, h / 2 },
            rotation,
            WHITE
        ); 
    }
};

constexpr float gravity = 0.5f;

// Forward Declarations
void update(Window& win, Block& block, Block& anch);
void handleInput(Block& block);
void followMouse(Block& block); // Left in case you need it later for rotation
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
            // TextFormat is the standard Raylib way to print formatted variables
            DrawTextEx(font, TextFormat("D: %.1f", dst), {5, 5}, 32, 2, WHITE);
        }
        
        update(win, block, anch);
        
        block.draw();
        anch.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void update(Window& win, Block& block, Block& anch)
{
    handleInput(block);
    
    Vector2 anchorPos = { anch.x, anch.y };
    Vector2 blockPos  = { block.x, block.y };
    float maxRadius = anch.anchor.radius;

    // =========================================================
    // STATE 1: DRAGGING / AIMING
    // =========================================================
    if (!block.flying)
    {
        // 1. Handle Mouse Dragging
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            block.x = GetMouseX();
            block.y = GetMouseY();
            blockPos = { block.x, block.y }; // Update local vector
        }

        // 2. Apply Tether Constraint (Keep it inside the circle)
        float dst = Vector2Distance(anchorPos, blockPos);
        if (dst > maxRadius)
        {
            Vector2 direction = Vector2Subtract(blockPos, anchorPos);
            Vector2 normalizedDir = Vector2Normalize(direction);
            Vector2 restrictedOffset = Vector2Scale(normalizedDir, maxRadius);
            
            block.x = anchorPos.x + restrictedOffset.x;
            block.y = anchorPos.y + restrictedOffset.y;
            
            // Re-sync local variables for the launch calculation below
            blockPos = { block.x, block.y };
            dst = maxRadius; 
        }

        // 3. Handle The Release (The Slingshot Launch)
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            if (dst > 10.0f) // Deadzone: Don't launch if barely pulled
            {
                block.flying = true;
                
                Vector2 direction = Vector2Subtract(blockPos, anchorPos);
                Vector2 normalizedDir = Vector2Normalize(direction);
                
                float launchPower = 0.15f; 
                
                // Shoot backwards relative to the pull direction
                block.xVel = -normalizedDir.x * dst * launchPower;
                block.yVel = -normalizedDir.y * dst * launchPower;
            }
            else 
            {
                // If the user just clicked without pulling, snap it back to center
                block.x = anchorPos.x;
                block.y = anchorPos.y;
            }
        }
    }
    // =========================================================
    // STATE 2: FLYING (PHYSICS ENGINE)
    // =========================================================
    else 
    {
        // 1. Apply Gravity to Vertical Velocity
        block.yVel += gravity;
        
        // 2. Apply Velocity to Position
        block.x += block.xVel;
        block.y += block.yVel;

        // 3. Floor Collision Check
        if (block.y >= win.floor)
        {
            block.y = win.floor;       // Snap exactly to floor
            block.flying = false;      // End the flight state
            block.xVel = 0.f;          // Kill kinetic momentum
            block.yVel = 0.f;
        }
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

double getDistance(Block& a, Block& b)
{
    // Using Raylib's highly optimized math instead of raw standard math
    Vector2 vA = { a.x, a.y };
    Vector2 vB = { b.x, b.y };
    return Vector2Distance(vA, vB);
}
