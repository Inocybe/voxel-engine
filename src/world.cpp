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

            {
                std::lock_guard<std::mutex> stateLock(chunkStateMutex);
                std::tuple<int, int, int> chunkCoords = std::make_tuple(data.x, data.y, data.z);
                chunkStates[chunkCoords] = ChunkState::Meshed; // Mark the chunk as meshed so it can be drawn
                //queuedForMeshing.erase(chunkCoords); // Remove from meshing queue
            }


            lock.lock(); // lock again to check the queue condition
        }
    }


    this->updateChunks();
    this->updateWorkers();
}



void World::updateChunks() {
    this->updateChunkDataGenerationQueue();
    this->drawChunks();
}


void World::updateWorkers() {
    const size_t maxChunkTasksPerFrame = 10;
    const size_t maxMeshTasksPerFrame = 10;

    {
        std::lock_guard<std::mutex> lock1(chunkDataGenerationQueueMutex);
        size_t tasksThisFrame = 0;
        while (!chunkDataGenerationQueue.empty() && tasksThisFrame < maxChunkTasksPerFrame) {
            glm::ivec3 chunkCoords = tupleToVec3i(chunkDataGenerationQueue.front());
            chunkDataGenerationQueue.pop_front();
            chunkWorkerThreadPool.addTask(chunkWorker, std::ref(*this), chunkCoords);
            tasksThisFrame++;
        }
    }

    {
        std::lock_guard<std::mutex> lock2(chunkVertexGenerationQueueMutex);
        size_t tasksThisFrame = 0;
        while (!chunkVertexGenerationQueue.empty() && tasksThisFrame < maxMeshTasksPerFrame) {
            glm::ivec3 chunkCoords = tupleToVec3i(chunkVertexGenerationQueue.front());
            chunkVertexGenerationQueue.pop_front();
            meshWorkerThreadPool.addTask(meshWorker, std::ref(*this), chunkCoords);
            tasksThisFrame++;
        }


    }
}



void World::drawChunks() {
    std::vector<std::tuple<int, int, int>> chunksToRemove; // Store chunks to remove after drawing to avoid modifying the map while iterating

    for (const auto& [location, data] : renderBuffers) {
        if (!player->isChunkInRenderDistance(tupleToVec3i(location))) {
            chunksToRemove.push_back(location); // Mark the chunk for removal
            continue; // Skip drawing chunks that are outside the player's render distance
        }

        shader->setVec3("chunkPos", (float)std::get<0>(location) * CHUNK_SIZE, (float)std::get<1>(location) * CHUNK_SIZE, (float)std::get<2>(location) * CHUNK_SIZE); // set the chunk position uniform to the world position of the chunk, this is used to calculate the world position of the vertices in the shader, so that they can be drawn in the correct position
        data->draw();
    }

    // Remove chunks that are outside the render distance
    for (const auto& chunkCoords : chunksToRemove) {
        this->removeChunk(chunkCoords);
    }
}


void World::updateChunkDataGenerationQueue() {
    // updating chunk coordinates, stop if player hasn't moved to a new chunk
    if (player->getChunkCoords() == lastPlayerChunkCoords) {
        return; // Player hasn't moved to a new chunk, no need to update the queue
    }
    lastPlayerChunkCoords = player->getChunkCoords();


    for (int x = -PlayerDistance::RENDER_DISTANCE + player->getChunkCoords().x; x <= PlayerDistance::RENDER_DISTANCE + player->getChunkCoords().x; x++) {
        for (int y = -PlayerDistance::RENDER_DISTANCE_HEIGHT + player->getChunkCoords().y; y <= PlayerDistance::RENDER_DISTANCE_HEIGHT + player->getChunkCoords().y; y++) {
            for (int z = -PlayerDistance::RENDER_DISTANCE + player->getChunkCoords().z; z <= PlayerDistance::RENDER_DISTANCE + player->getChunkCoords().z; z++) {
                // HERE FOR MAYBE IF I WANTED TO MAKE THE RENDER DISTANCE A CYLINDAR or smth \0/
                /*
                if (!player->isChunkInRenderDistance(glm::ivec3(x, y, z))) {
                    continue; // Skip adding chunks that are outside the player's render distance
                }
                    */

                std::tuple<int, int, int> chunkCoords = std::make_tuple(x, y, z);
                bool enqueForGeneration = false;
                bool enqueForMeshing = false;

                {
                    std::lock_guard<std::mutex> lock(chunkStateMutex);
                    auto stateIt = chunkStates.find(chunkCoords);
                    if (stateIt == chunkStates.end()) {
                        // Not generated, not queued for generation
                        chunkStates[chunkCoords] = ChunkState::QueuedForGeneration;
                        enqueForGeneration = true;
                    } else if (stateIt->second == ChunkState::Generated) {
                        // Generated but not queued for meshing
                        stateIt->second = ChunkState::QueuedForMeshing;
                        enqueForMeshing = true;
                    }
                }

                if (enqueForGeneration) {
                    std::lock_guard<std::mutex> lock(chunkDataGenerationQueueMutex);
                    chunkDataGenerationQueue.push_back(chunkCoords);
                } else if (enqueForMeshing) {
                    std::lock_guard<std::mutex> lock(chunkVertexGenerationQueueMutex);
                    chunkVertexGenerationQueue.push_back(chunkCoords);
                }
            }
        }
    }


}

void World::removeChunk(const std::tuple<int, int, int>& chunkCoords) {
    renderBuffers.erase(chunkCoords); // Remove the render buffer for the chunk, this will prevent it from being drawn and also free up memory
    {
        std::lock_guard<std::mutex> lock(chunkStateMutex);
        auto stateIt = chunkStates.find(chunkCoords);
        if (stateIt != chunkStates.end() && stateIt->second == ChunkState::Meshed) {
            stateIt->second = ChunkState::Generated; // Mark as generated so it can be re-meshed if the player moves back into range
        }
    }
}

