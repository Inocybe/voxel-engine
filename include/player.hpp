#pragma once

class World;
class InputManager;


#include <glm/glm.hpp>
#include <camera.hpp>

namespace PlayerDistance {
    // Render distance is a cylindar shape
    constexpr int RENDER_DISTANCE = 8;
    constexpr int RENDER_DISTANCE_HEIGHT = 3;
}


class Player {
public:

    Player(World* world, InputManager& inputManager, glm::vec3 pos);
    ~Player() {
        delete camera;
    }
    void update();

    glm::ivec3 getChunkCoords() const;
    bool isChunkInRenderDistance(const glm::ivec3& chunkCoords) const;
    Camera& getCamera() { return *camera; }
private:
    World* world;
    Camera* camera = nullptr;
    InputManager& inputManager;
    glm::vec3 pos;

    float lastFrameTime = 0.0f;
    float deltaTime = 0.0f;
    float generationCooldown = 0.0f; // cooldown timer for chunk generation, to prevent generating too many chunks at once when the player moves quickly

    void moveForward();
    void moveBackward();
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void sprint();
};