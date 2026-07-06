#include "raylib.h"
#include "raymath.h"
#include <cmath>

// =========================================================
// Configuration Data
// =========================================================
namespace Config {
    constexpr int WindowWidth = 900;
    constexpr int WindowHeight = 540;
    constexpr int TargetFPS = 60;
    
    constexpr float FloorY = 500.0f;
    constexpr float Gravity = 0.5f;
    
    // Slingshot tuning
    constexpr float LaunchPowerMultiplier = 0.20f;
    constexpr float DeadzoneDistance = 10.0f;
    constexpr float MaxTetherRadius = 100.0f;
}

// =========================================================
// Window Manager (RAII Pattern)
// =========================================================
/**
 * @brief Ensures the Raylib window is safely initialized and closed.
 */
class ScopedWindow {
public:
    ScopedWindow(int width, int height, const char* title) {
        InitWindow(width, height, title);
        SetTargetFPS(Config::TargetFPS);
    }
    
    ~ScopedWindow() {
        CloseWindow();
    }

    // Delete copy/move constructors to prevent accidental double-closures
    ScopedWindow(const ScopedWindow&) = delete;
    ScopedWindow& operator=(const ScopedWindow&) = delete;
};

// =========================================================
// Entities
// =========================================================

/**
 * @brief Represents the physical object being thrown.
 * Kept as a simple struct (Plain Old Data) since it only holds state.
 */
struct Projectile {
    Vector2 position { 0.0f, 0.0f };
    Vector2 velocity { 0.0f, 0.0f };
    Vector2 size { 30.0f, 30.0f };
    float rotation = 0.0f;
    bool isFlying = false;

    /// @brief Renders the projectile to the screen.
    void Draw() const {
        Rectangle rect { position.x, position.y, size.x, size.y };
        Vector2 origin { size.x / 2.0f, size.y / 2.0f };
        
        DrawRectanglePro(rect, origin, rotation, WHITE);
    }
};

/**
 * @brief Controller class that manages the slingshot logic, aiming, and physics.
 */
class Slingshot {
private:
    Vector2 m_anchorPos;
    Projectile m_projectile;

public:
    explicit Slingshot(Vector2 anchorPos) : m_anchorPos(anchorPos) {
        ResetProjectile();
    }

    /// @brief Main update loop for the slingshot.
    void Update() {
        HandleManualRotation();

        if (!m_projectile.isFlying) {
            HandleAiming();
        } else {
            UpdatePhysics();
        }
    }

    /// @brief Renders the anchor, the tether (if aiming), and the projectile.
    void Draw() const {
        // Draw the anchor point
        DrawCircleV(m_anchorPos, 5.0f, RED);
        
        // Draw a visual tether line while dragging
        if (!m_projectile.isFlying && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            DrawLineEx(m_anchorPos, m_projectile.position, 2.0f, DARKGRAY);
        }

        m_projectile.Draw();
    }

    /// @brief Calculates current drag distance for UI.
    float GetDistanceToAnchor() const {
        return Vector2Distance(m_anchorPos, m_projectile.position);
    }

private:
    /**
     * @brief Handles mouse dragging and constraining the projectile to the max radius.
     */
    void HandleAiming() {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            float dist = Vector2Distance(m_anchorPos, mousePos);

            // Constrain to maximum radius
            if (dist > Config::MaxTetherRadius) {
                Vector2 direction = Vector2Normalize(Vector2Subtract(mousePos, m_anchorPos));
                m_projectile.position = Vector2Add(m_anchorPos, Vector2Scale(direction, Config::MaxTetherRadius));
            } else {
                m_projectile.position = mousePos;
            }
        }

        // Handle the release/launch
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            float dist = Vector2Distance(m_anchorPos, m_projectile.position);
            
            // Deadzone check: prevent launching if barely pulled
            if (dist > Config::DeadzoneDistance) {
                m_projectile.isFlying = true;
                
                // Direction is from projectile TO anchor (shoot opposite of pull)
                Vector2 direction = Vector2Normalize(Vector2Subtract(m_anchorPos, m_projectile.position));
                
                // Velocity scales with how far it was pulled
                m_projectile.velocity = Vector2Scale(direction, dist * Config::LaunchPowerMultiplier);
            } else {
                ResetProjectile();
            }
        }
    }

    /**
     * @brief Applies gravity and velocity to the projectile, checking for floor collisions.
     */
    void UpdatePhysics() {
        // Apply gravity to velocity, then velocity to position (Euler integration)
        m_projectile.velocity.y += Config::Gravity;
        m_projectile.position = Vector2Add(m_projectile.position, m_projectile.velocity);

        // Floor collision check
        if (m_projectile.position.y >= Config::FloorY) {
            m_projectile.position.y = Config::FloorY;
            m_projectile.isFlying = false;
            m_projectile.velocity = { 0.0f, 0.0f }; // Kill momentum
        }
    }

    /**
     * @brief Allows the user to spin the projectile manually.
     */
    void HandleManualRotation() {
        if (IsKeyDown(KEY_J)) { m_projectile.rotation -= 2.0f; }
        if (IsKeyDown(KEY_L)) { m_projectile.rotation += 2.0f; }
    }

    /**
     * @brief Snaps the projectile back to the resting position at the anchor.
     */
    void ResetProjectile() {
        m_projectile.position = m_anchorPos;
        m_projectile.velocity = { 0.0f, 0.0f };
        m_projectile.isFlying = false;
    }
};

// =========================================================
// Main Entry Point
// =========================================================
int main() {
    // Initializes window and ensures it closes automatically on exit
    ScopedWindow window(Config::WindowWidth, Config::WindowHeight, "Slingshot");

    Slingshot slingshot({ 200.0f, 375.0f });
    Font defaultFont = GetFontDefault();

    while (!WindowShouldClose()) {
        // --- UPDATE ---
        slingshot.Update();

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw Floor Line
        DrawLine(0, Config::FloorY + 15, Config::WindowWidth, Config::FloorY + 15, DARKGRAY);

        // UI
        float distance = slingshot.GetDistanceToAnchor();
        DrawTextEx(defaultFont, TextFormat("Distance: %.1f", distance), { 10.0f, 10.0f }, 24.0f, 2.0f, LIGHTGRAY);

        slingshot.Draw();

        EndDrawing();
    }

    return 0; // ScopedWindow destructor automatically calls CloseWindow() here
}
