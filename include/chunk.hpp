#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <array>
#include <mutex>
#include <queue>
#include <vector>
#include <random>
#include <iostream>
#include <unordered_map>

// FORWARD DELCARATIONS 
class World;


constexpr int CHUNK_SIZE = 16;

#pragma pack(push, 1)
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};
#pragma pack(pop)


enum Direction { POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z };

namespace CubeGeometry {
    // index into this array to get cube face positions 
    // works like [direction][vertex]
    static constexpr glm::vec3 cubeFacePositions[6][4] = {
        // +X (right)
        { {1,0,0}, {1,1,0}, {1,1,1}, {1,0,1} },

        // -X (left)
        { {0,0,1}, {0,1,1}, {0,1,0}, {0,0,0} },

        // +Y (top)
        { {0,1,1}, {1,1,1}, {1,1,0}, {0,1,0} },

        // -Y (bottom)
        { {0,0,0}, {1,0,0}, {1,0,1}, {0,0,1} },

        // +Z (front)
        { {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1} },

        // -Z (back)
        { {0,1,0}, {1,1,0}, {1,0,0}, {0,0,0} }
    };
    // this one just dependant on the direction
    static constexpr glm::vec3 cubeFaceNormals[6] = {
        { 1,  0,  0}, // +X
        {-1,  0,  0}, // -X
        { 0,  1,  0}, // +Y
        { 0, -1,  0}, // -Y
        { 0,  0,  1}, // +Z
        { 0,  0, -1}  // -Z
    };
    // this one dependant on the vertex
    static constexpr glm::vec2 cubeFaceUVs[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
}

// if block is 0 it is air, otherwise will be just block 
struct Block {
    uint8_t type;
};



class ChunkMesh {
public:
    int x, y, z; // chunk coordinates
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    ChunkMesh() = default;

    void addFace(int wx, int wy, int wz, Direction dir);
};

class RenderBuffer {
public:
    RenderBuffer();
    ~RenderBuffer();
    void upload(const Vertex* vertices, size_t vertexBytes, const unsigned int* indices, size_t indexBytes);
    void uploadChunkMesh(const ChunkMesh& chunkMesh);
    void draw() const;
private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
};



void meshWorker(World& world, glm::ivec3 chunkPos); // function that will be run by the mesh worker thread, will wait for chunks to be added to the queue and then generate mesh data for them and upload to gpu, then mark them as ready to draw




class Chunk {
public:
    int x, y, z; // chunk coordinates
    std::array<Block, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> blocks;

    
    Chunk(int x, int y, int z);

    bool isBlockAir(int x, int y, int z);
};

