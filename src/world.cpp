#include <world.hpp>


World::World(glm::vec3& cameraPos) : cameraPos(cameraPos) {};


void World::update() const {
    // for now, just draw all the chunks in the world
    this->drawChunks();
}


void World::drawChunks() const {
    for (const auto& [location, data] : world) {
        data->draw();
    }
}