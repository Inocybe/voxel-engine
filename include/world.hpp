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


enum Direction { POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z };


struct Block {
    uint8_t type;
};

class Chunk {
public:
    int x, y, z; // chunk coordinates
    
    bool isAir(int x, int y, int z);
private:
    std::array<Block, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> blocks;
};

class MeshData {
public:
    int x, y, z; // chunk coordinates

    void addFace(Direction dir);
private:

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
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