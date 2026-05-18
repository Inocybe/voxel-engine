#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class World;

namespace PlayerDistance {
    // Render distance is a cylindar shape
    constexpr int RENDER_DISTANCE = 8;
    constexpr int RENDER_DISTANCE_HEIGHT = 3;
}


class Player {
public:
    Player(World* world, GLFWwindow* window, glm::vec3& cameraPos);
    void update();

    glm::ivec3 getChunkCoords() const;
    bool isChunkInRenderDistance(const glm::ivec3& chunkCoords) const;
private:
    World* world;
    GLFWwindow* window;
    glm::vec3& pos;

    float lastFrameTime = 0.0f;
    float deltaTime = 0.0f;
    float generationCooldown = 0.0f; // cooldown timer for chunk generation, to prevent generating too many chunks at once when the player moves quickly

    void processInput();
};