#include <player.hpp>
#include <chunk.hpp>

#include <iostream>

Player::Player(glm::vec3& cameraPos) : pos(cameraPos) {}

void Player::update() {
    //printf("Player position: %f, %f, %f\n", pos.x, pos.y, pos.z);
}

glm::ivec3 Player::getChunkCoords() const {
    return glm::ivec3(pos) / CHUNK_SIZE;
}

bool Player::isChunkInRenderDistance(const glm::ivec3& chunkCoords) const {
    glm::ivec3 playerChunkCoords = this->getChunkCoords();
    glm::ivec3 delta = chunkCoords - playerChunkCoords;

    bool inHorizontalDistance = std::abs(delta.x) <= PlayerDistance::RENDER_DISTANCE && std::abs(delta.z) <= PlayerDistance::RENDER_DISTANCE;
    bool inVerticalDistance = std::abs(delta.y) <= PlayerDistance::RENDER_DISTANCE_HEIGHT;

    return inHorizontalDistance && inVerticalDistance;
}