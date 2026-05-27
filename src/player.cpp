#include <player.hpp>
#include <chunk.hpp>
#include <world.hpp>
#include <heightmap.hpp>
#include <engine.hpp>

#include <iostream>

Player::Player(World* world, InputManager& inputManager, glm::vec3& cameraPos) : world(world), inputManager(inputManager), pos(cameraPos) {
    inputManager.subscribe(InputAction::MoveForward, [this]() { this->moveForward(); });
    inputManager.subscribe(InputAction::MoveBackward, [this]() { this->moveBackward(); });
    inputManager.subscribe(InputAction::MoveLeft, [this]() { this->moveLeft(); });
    inputManager.subscribe(InputAction::MoveRight, [this]() { this->moveRight(); });
    inputManager.subscribe(InputAction::MoveUp, [this]() { this->moveUp(); });
    inputManager.subscribe(InputAction::MoveDown, [this]() { this->moveDown(); });
    inputManager.subscribe(InputAction::Sprint, [this]() { this->sprint(); });
    inputManager.subscribe(InputAction::ReloadWorld, [this]() { this->world->worldCommand = WorldCommands::Update; });
}

void Player::update() {
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
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



void Player::moveForward() {
}
void Player::moveBackward() {
}
void Player::moveLeft() {
}
void Player::moveRight() {
}
void Player::moveUp() {
}
void Player::moveDown() {
}
void Player::sprint() {
}