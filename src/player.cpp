#include <player.hpp>

Player::Player(glm::vec3& cameraPos) : pos(cameraPos) {}

glm::ivec3 Player::getChunkCoords() const {
    return glm::ivec3(pos) / 16;
}