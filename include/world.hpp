#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <array>
#include <mutex>
#include <queue>
#include <vector>
#include <unordered_map>

#include <thread_pool.hpp>

constexpr int CHUNK_SIZE = 16;



#pragma pack(push, 1)
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};
#pragma pack(pop)



struct Block {
    uint8_t type;
};

struct Chunk {
    std::array<Block, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> blocks;
    int x, y, z; // chunk coordinates
};

struct MeshData {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int x, y, z; // chunk coordinates
};

struct ChunkMesh {
    unsigned int VAO, VBO, EBO;
    int indexCount;
    int x, y, z; // chunk coordinates
};

class World {
public:
    World() = default;
    std::unordered_map<std::tuple<int, int, int>, Chunk> world;
    std::mutex worldMutex;

    std::queue<MeshData> meshUploadQueue;
    std::mutex meshQueueMutex;
    std::condition_variable meshReadyCV;

    std::atomic<bool> running = true;
};