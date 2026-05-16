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

    // Check if the chunk is within the cylindrical shape defined by RENDER_DISTANCE_SQUARED and RENDER_DISTANCE_HEIGHT
    float distanceSquared = (delta.x * delta.x) + (delta.z * delta.z);
    bool inHorizontalDistance = distanceSquared <= PlayerDistance::RENDER_DISTANCE_SQUARED;
    bool inVerticalDistance = std::abs(delta.y) <= PlayerDistance::RENDER_DISTANCE_HEIGHT;

    return inHorizontalDistance && inVerticalDistance;
}