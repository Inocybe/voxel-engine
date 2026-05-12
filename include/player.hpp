#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace PlayerDistance {
    // Render distance is a cylindar shape
    constexpr int RENDER_DISTANCE = 16;
    constexpr int RENDER_DISTANCE_SQUARED = RENDER_DISTANCE * RENDER_DISTANCE;
    constexpr int RENDER_DISTANCE_HEIGHT = 8;
}


class Player {
public:
    Player(glm::vec3& cameraPos);
    void update();

    glm::ivec3 getChunkCoords() const;
    bool isChunkInRenderDistance(const glm::ivec3& chunkCoords) const;
private:
    glm::vec3& pos;
};