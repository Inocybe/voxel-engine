#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <array>
#include <mutex>
#include <queue>
#include <deque>
#include <vector>
#include <random>
#include <unordered_map>
#include <unordered_set>

#include <thread_pool.hpp>
#include <shader.hpp>
#include <heightmap.hpp>
#include <player.hpp>
#include <chunk.hpp>



enum class ChunkState : uint8_t {
    QueuedForGeneration,
    Generated,
    QueuedForMeshing,
    Meshed
};


struct TupleHash {
    size_t operator()(const std::tuple<int, int, int>& t) const {
        auto h1 = std::hash<int>{}(std::get<0>(t));
        auto h2 = std::hash<int>{}(std::get<1>(t));
        auto h3 = std::hash<int>{}(std::get<2>(t));
        return h1 ^ (h2 << 32) ^ (h3 << 16);
    }
};


class World {
public:
    std::unordered_map<std::tuple<int, int, int>, std::unique_ptr<Chunk>, TupleHash> world;
    std::mutex worldMutex;
    
    // variables for uploading chunks to world mesh 
    // (I think I'll reserve one thread specficailly for generating world stuff)
    std::deque<std::tuple<int, int, int>> chunkDataGenerationQueue;  // Keep as simple task queue
    std::deque<std::tuple<int, int, int>> chunkVertexGenerationQueue;
    std::mutex chunkDataGenerationQueueMutex;
    std::mutex chunkVertexGenerationQueueMutex;


    // chunk state manager to keep track of which chunks are generated, and which are queued for generation or meshing, this is used to prevent multiple threads from generating the same chunk or mesh at the same time, and also to prevent generating chunks that are already generated but not yet meshed, this will be used in the updateChunkDataGenerationQueue function to determine which chunks need to be generated or meshed when the player moves to a new chunk
    std::unordered_map<std::tuple<int, int, int>, ChunkState, TupleHash> chunkStates;
    std::unordered_set<std::tuple<int, int, int>, TupleHash> queuedForGeneration;
    std::unordered_set<std::tuple<int, int, int>, TupleHash> queuedForMeshing;
    std::mutex chunkStateMutex;




    std::unordered_map<std::tuple<int, int, int>, std::unique_ptr<RenderBuffer>, TupleHash> renderBuffers; // map of chunk coordinates to render buffers, used to store the render buffers for each chunk, so that they can be drawn when needed, and also to prevent them from being deleted when the mesh worker thread finishes
    // variables for uploading renderbuffers to unordered map
    std::queue<ChunkMesh> meshUploadQueue;
    std::mutex meshQueueMutex;

    //std::atomic<bool> running = true;
    std::unique_ptr<Player> player;

    ThreadPool meshWorkerThreadPool{4}; // thread pool for generating chunk meshes, currently set to 4 threads, but can be increased later if needed
    ThreadPool chunkWorkerThreadPool{4}; // thread pool for generating chunks, currently set to 2 threads, but can be increased later if needed

    World(glm::vec3& cameraPos, Shader* shader);


    void update();
private:
    glm::vec3& cameraPos;
    std::unique_ptr<Shader> shader;
    glm::ivec3 lastPlayerChunkCoords = glm::ivec3(0);


    void drawChunks();
    // function that calls other functions
    // currently draws world, and updates chunk generation queue
    void updateChunks();
    void updateWorkers();

    void updateChunkDataGenerationQueue();
    void removeChunk(const std::tuple<int, int, int>& chunkCoords);

    glm::ivec3 tupleToVec3i(const std::tuple<int, int, int>& t) const {
        return glm::ivec3(std::get<0>(t), std::get<1>(t), std::get<2>(t));
    }
};