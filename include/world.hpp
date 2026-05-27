#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <array>
#include <mutex>
#include <shared_mutex>
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

class InputManager;


// enum to call for force updating the world if I want it to remap the noise stuff
enum WorldCommands {
    None,
    Update,
};

// states for generation
enum class ChunkState : uint8_t {
    QueuedForGeneration,
    Generated,
    QueuedForMeshing,
    Meshed
};

// own chunk to help simplify things
struct ChunkCoords {
    int x, y, z;

    bool operator==(const ChunkCoords& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};


namespace std {
    template <>
    struct hash<ChunkCoords> {
        size_t operator()(const ChunkCoords& coords) const {
            auto h1 = std::hash<int>{}(coords.x);
            auto h2 = std::hash<int>{}(coords.y);
            auto h3 = std::hash<int>{}(coords.z);
            size_t seed = h1;
            // some weird ahh magic hashing constants 0x9e3779b9
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}


class World {
public:
    std::unordered_map<ChunkCoords, std::unique_ptr<Chunk>> world;
    std::shared_mutex worldMutex;
    
    // variables for uploading chunks to world mesh 
    // (I think I'll reserve one thread specficailly for generating world stuff)
    std::deque<ChunkCoords> chunkDataGenerationQueue;  // Keep as simple task queue
    std::deque<ChunkCoords> chunkVertexGenerationQueue;
    std::mutex chunkDataGenerationQueueMutex;
    std::mutex chunkVertexGenerationQueueMutex;


    // chunk state manager to keep track of which chunks are generated, and which are queued for generation or meshing, this is used to prevent multiple threads from generating the same chunk or mesh at the same time, and also to prevent generating chunks that are already generated but not yet meshed, this will be used in the updateChunkDataGenerationQueue function to determine which chunks need to be generated or meshed when the player moves to a new chunk
    std::unordered_map<ChunkCoords, ChunkState> chunkStates;
    std::mutex chunkStateMutex;

    std::unordered_map<ChunkCoords, std::unique_ptr<RenderBuffer>> renderBuffers; // map of chunk coordinates to render buffers, used to store the render buffers for each chunk, so that they can be drawn when needed, and also to prevent them from being deleted when the mesh worker thread finishes
    // variables for uploading renderbuffers to unordered map
    // this stores finished vertex data, prepares it for upload
    std::queue<ChunkMesh> meshUploadQueue;
    std::mutex meshQueueMutex;


    //std::atomic<bool> running = true;
    std::unique_ptr<Player> player;
    // allows for force update of the world
    WorldCommands worldCommand = WorldCommands::None;
    // heightmap
    Heightmap heightmap;
    std::shared_mutex heightmapMutex;



    ThreadPool meshWorkerThreadPool{4}; // thread pool for generating chunk meshes, currently set to 4 threads, but can be increased later if needed
    ThreadPool chunkWorkerThreadPool{4}; // thread pool for generating chunks, currently set to 2 threads, but can be increased later if needed


    World(InputManager& inputManager, glm::vec3& cameraPos, Shader* shader);


    void update();
private:
    glm::vec3& cameraPos;
    Shader* shader;
    glm::ivec3 lastPlayerChunkCoords = glm::ivec3(0);


    void drawChunks();
    // function that calls other functions
    // currently draws world, and updates chunk generation queue
    void updateChunks();
    void updateWorkers();
    void updateChunkDataGenerationQueue();
    void removeChunk(const ChunkCoords& chunkCoords);

    glm::ivec3 coordsToVec3i(const ChunkCoords& coords) {
        return glm::ivec3(coords.x, coords.y, coords.z);
    }
};