#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>


constexpr int RENDER_DISTANCE = 4; // in chunks, so this is 4 chunks in each direction, for a total of 8x8x8 = 512 chunks loaded at once, which should be fine for testing, but will need to be optimized later

class Player {
public:
    Player(glm::vec3& cameraPos);
    void update();

    glm::ivec3 getChunkCoords() const;
private:
    glm::vec3& pos;
};