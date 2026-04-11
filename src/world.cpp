#include <world.hpp>


World::World(glm::vec3& cameraPos) : cameraPos(cameraPos) {};



void World::update() {
    
    {
        std::unique_lock<std::mutex> lock(meshQueueMutex);
        
        while(!meshUploadQueue.empty()) {
            ChunkMesh data = std::move(meshUploadQueue.front());
            meshUploadQueue.pop();


            lock.unlock(); // unlock while processing the mesh to allow other threads to push meshes to the queue

            RenderBuffer renderBuffer;
            renderBuffer.uploadChunkMesh(data);

            // adding the render buffer to the map of render buffers, so that it can be drawn when needed, 
            // also to prevent it from being deleted when the mesh worker thread finishes
            renderBuffers.emplace(std::make_tuple(data.x, data.y, data.z), std::make_unique<RenderBuffer>(std::move(renderBuffer)));

            lock.lock(); // lock again to check the queue condition
        }
    }


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
                //chunk->createRandomChunk();
                world.emplace(std::make_tuple(x, y, z), std::move(chunk));
            }
        }
    }

}


void World::drawChunks() const {
    for (const auto& [location, data] : renderBuffers) {
        data->draw();
    }
}