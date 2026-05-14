#include <world.hpp>


World::World(glm::vec3& cameraPos, Shader* shader) : cameraPos(cameraPos), shader(shader) {
    player = std::make_unique<Player>(cameraPos);
};



void World::update() {
    player->update();

    int maxMeshUploadsPerFrame = 50; // Limit the number of mesh uploads per frame to prevent stuttering
    int uploadsThisFrame = 0;

    // render and add the meshes from the meshqueue
    {
        std::unique_lock<std::mutex> lock(meshQueueMutex);
        
        while(!meshUploadQueue.empty() && uploadsThisFrame < maxMeshUploadsPerFrame) {
            ChunkMesh data = std::move(meshUploadQueue.front());
            meshUploadQueue.pop();
            uploadsThisFrame++;

            lock.unlock(); // unlock while processing the mesh to allow other threads to push meshes to the queue
            
            std::unique_ptr<RenderBuffer> renderBuffer = std::make_unique<RenderBuffer>();

            // adding the render buffer to the map of render buffers, so that it can be drawn when needed, 
            // also to prevent it from being deleted when the mesh worker thread finishes
            renderBuffers.emplace(std::make_tuple(data.x, data.y, data.z), std::move(renderBuffer));
            renderBuffers[std::make_tuple(data.x, data.y, data.z)]->uploadChunkMesh(data);

            lock.lock(); // lock again to check the queue condition
        }
    }


    this->updateChunks();
    this->updateWorkerThreads();
}



void World::updateChunks() {
    this->updateChunkGenerationQueue();
    this->drawChunks();
}


void World::drawChunks() {
    for (const auto& [location, data] : renderBuffers) {
        if (!player->isChunkInRenderDistance(tupleToVec3i(location))) {
            removeChunk(location); // remove the chunk from the world and render buffer if it's outside the player's render distance, this is used to free up memory and also to prevent drawing chunks that are too far away, which can cause stuttering and also is a waste of resources, this will also be done in the chunk generation queue update function, but this is just to make sure that it's done in case the player moves very fast and the chunk generation queue doesn't update fast enough
            continue; // Skip drawing chunks that are outside the player's render distance
        }
        shader->setVec3("chunkPos", (float)std::get<0>(location) * CHUNK_SIZE, (float)std::get<1>(location) * CHUNK_SIZE, (float)std::get<2>(location) * CHUNK_SIZE); // set the chunk position uniform to the world position of the chunk, this is used to calculate the world position of the vertices in the shader, so that they can be drawn in the correct position
        data->draw();
    }
}


void World::updateChunkGenerationQueue() {
    std::lock_guard<std::mutex> lock(chunkGenerationQueueMutex);

    for (int x = -PlayerDistance::RENDER_DISTANCE + player->getChunkCoords().x; x <= PlayerDistance::RENDER_DISTANCE + player->getChunkCoords().x; x++) {
        for (int y = -PlayerDistance::RENDER_DISTANCE_HEIGHT + player->getChunkCoords().y; y <= PlayerDistance::RENDER_DISTANCE_HEIGHT + player->getChunkCoords().y; y++) {
            for (int z = -PlayerDistance::RENDER_DISTANCE + player->getChunkCoords().z; z <= PlayerDistance::RENDER_DISTANCE + player->getChunkCoords().z; z++) {
                if (!player->isChunkInRenderDistance(glm::ivec3(x, y, z))) {
                    continue; // Skip adding chunks that are outside the player's render distance
                }

                std::tuple<int, int, int> chunkCoords = std::make_tuple(x, y, z);
                if (world.find(chunkCoords) == world.end() 
                    && std::find(chunkGenerationQueue.begin(), chunkGenerationQueue.end(), chunkCoords) == chunkGenerationQueue.end()) {
                    
                    chunkGenerationQueue.push_front(chunkCoords);
                }
            }
        }
    }


}

void World::removeChunk(const std::tuple<int, int, int>& chunkCoords) {
    //std::lock_guard<std::mutex> lock(worldMutex);
    std::tuple<int, int, int> chunkCoordsTuple = std::make_tuple(std::get<0>(chunkCoords), std::get<1>(chunkCoords), std::get<2>(chunkCoords));
    // TEMOPORARY THIS IS TO POTENTIALLY MAKE IT SO THAT MAYBE I DON"T REMOVE THE WORLD DATA BECASUE TAKES UP NOT MUCH MEMORY ON CPU
    //world.erase(chunkCoordsTuple);
    renderBuffers.erase(chunkCoordsTuple); // also remove the render buffer for the chunk to free up memory
}

