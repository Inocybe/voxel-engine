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
                queuedForMeshing.erase(chunkCoords); // Remove from meshing queue
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
    for (const auto& [location, data] : renderBuffers) {
        if (!player->isChunkInRenderDistance(tupleToVec3i(location))) {
            removeChunk(location); // remove the chunk from the world and render buffer if it's outside the player's render distance, this is used to free up memory and also to prevent drawing chunks that are too far away, which can cause stuttering and also is a waste of resources, this will also be done in the chunk generation queue update function, but this is just to make sure that it's done in case the player moves very fast and the chunk generation queue doesn't update fast enough
            continue; // Skip drawing chunks that are outside the player's render distance
        }
        shader->setVec3("chunkPos", (float)std::get<0>(location) * CHUNK_SIZE, (float)std::get<1>(location) * CHUNK_SIZE, (float)std::get<2>(location) * CHUNK_SIZE); // set the chunk position uniform to the world position of the chunk, this is used to calculate the world position of the vertices in the shader, so that they can be drawn in the correct position
        data->draw();
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
    renderBuffers.erase(chunkCoords); // remove the render buffer for the chunk to free up memory

    {
        std::lock_guard<std::mutex> lock(chunkStateMutex);
        auto stateIt = chunkStates.find(chunkCoords);
        if (stateIt != chunkStates.end() && stateIt->second == ChunkState::Meshed) {
            stateIt->second = ChunkState::Generated; // Mark as generated so it can be re-meshed if the player moves back into range
        }
        queuedForMeshing.erase(chunkCoords);
    }
}

