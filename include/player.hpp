#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

constexpr int RENDER_DISTANCE = 4; // in chunks, so this is 4 chunks in each direction, for a total of 8x8x8 = 512 chunks loaded at once, which should be fine for testing, but will need to be optimized later

class Player {
public:
    Player(glm::vec3& cameraPos);
    void update();

    glm::ivec3 getChunkCoords() const;
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();
private:
    glm::vec3& pos;
    glm::mat4 m_projection = glm::perspective(glm::radians(45.0f), (float)1200 / (float)900, 0.1f, 1000.0f);

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
};