#include <player.hpp>

#include <iostream>

Player::Player(glm::vec3& cameraPos) : pos(cameraPos) {}

void Player::update() {
    //printf("Player position: %f, %f, %f\n", pos.x, pos.y, pos.z);
}

glm::ivec3 Player::getChunkCoords() const {
    return glm::ivec3(pos) / 16;
}


glm::mat4 Player::GetViewMatrix() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 Player::GetProjectionMatrix() {
    return m_projection;
}