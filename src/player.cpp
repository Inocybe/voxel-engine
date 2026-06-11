#include <player.hpp>
#include <chunk.hpp>
#include <world.hpp>
#include <heightmap.hpp>
#include <engine.hpp>

#include <iostream>

Player::Player(World* world, InputManager& inputManager, glm::vec3 pos) : world(world), inputManager(inputManager), pos(pos) {
    camera = new Camera(pos);

    inputManager.bindKey(GLFW_KEY_W, InputAction::MoveForward, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_S, InputAction::MoveBackward, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_A, InputAction::MoveLeft, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_D, InputAction::MoveRight, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_SPACE, InputAction::MoveUp, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_LEFT_CONTROL, InputAction::MoveDown, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_LEFT_SHIFT, InputAction::Sprint, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_R, InputAction::ReloadWorld, InputType::Press);

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
    camera->move(CameraDirection::Type::FORWARD, deltaTime);
}
void Player::moveBackward() {
    camera->move(CameraDirection::Type::BACKWARD, deltaTime);
}
void Player::moveLeft() {
    camera->move(CameraDirection::Type::LEFT, deltaTime);
}
void Player::moveRight() {
    camera->move(CameraDirection::Type::RIGHT, deltaTime);
}
void Player::moveUp() {
    camera->move(CameraDirection::Type::UP, deltaTime);
}
void Player::moveDown() {
    camera->move(CameraDirection::Type::DOWN, deltaTime);
}
void Player::sprint() {
    //camera.setSprintMode(true);
}