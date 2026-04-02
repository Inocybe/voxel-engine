#include <world.hpp>


World::World(glm::vec3& cameraPos) : cameraPos(cameraPos) {};

void World::drawChunks() const {
    for (const auto& [location, data] : world) {
        data->draw();
    }
}