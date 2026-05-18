#include <player.hpp>
#include <chunk.hpp>
#include <world.hpp>
#include <heightmap.hpp>

#include <iostream>

Player::Player(World* world, GLFWwindow* window, glm::vec3& cameraPos) : world(world), window(window), pos(cameraPos) {}

void Player::update() {
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    this->processInput();
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

void Player::processInput() {

    generationCooldown -= deltaTime; // decrease cooldown timer
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (generationCooldown > 0.0f) return; // still in cooldown, skip generation
        world->worldCommand = WorldCommands::Update;
        generationCooldown = 1.0f; // set cooldown (RANDOM NUMBER I SET SO THAT IT JUST DOENS"T SPAM)
    }

    //Heightmap& heightmap = world->heightmap; // get reference to heightmap from world

}