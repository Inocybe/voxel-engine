#include <world.hpp>


World::World(glm::vec3& cameraPos) : cameraPos(cameraPos) {};



void World::update() const {
    // for now, just draw all the chunks in the world
    this->drawChunks();
}


void World::makeTestingMap(int size) {
    for (int x = -size; x <= size; x++) {
        for (int y = -size; y <= size; y++) {
            for (int z = -size; z <= size; z++) {
                std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>();
                chunk->x = x;
                chunk->y = y;
                chunk->z = z;
                chunk->createRandomChunk();
                world.emplace(std::make_tuple(x, y, z), std::move(chunk));
            }
        }
    }

}


void World::drawChunks() const {
    for (const auto& [location, data] : world) {
        data->draw();
    }
}