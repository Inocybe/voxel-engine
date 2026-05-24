#include <chunk.hpp>
#include <world.hpp>
#include <heightmap.hpp>

#include <shared_mutex>


// function to be called to be run on another thread
// this function is what creates the mesh data to mesh
void meshWorker(World& world, ChunkCoords chunkPos) {
    ChunkMesh chunkMesh;

    chunkMesh.x = (int)chunkPos.x;
    chunkMesh.y = (int)chunkPos.y;
    chunkMesh.z = (int)chunkPos.z;

    {
        // lock the world mutex to provent data from reading at the same time
        std::shared_lock<std::shared_mutex> lock(world.worldMutex);

        auto it = world.world.find(ChunkCoords{(int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z});
        if (it == world.world.end()) {
            printf("Chunk removed before meshing: %d, %d, %d\n", (int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z);
            return;
        }
        Chunk& chunk = *(it->second);

        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
                    Block b = chunk.blocks[index];

                    // Skip air blocks entirely -- nothing to draw.
                    // "continue" jumps to the next loop iteration immediately.
                    if (b.type == 0) continue;

                    // Face culling -- only add a face if the neighbor is air.
                    // A face shared between two solid blocks is NEVER visible,
                    // so adding it just wastes GPU time drawing nothing.
                    // World position = chunk position * chunk size + local position.
                    int wx = x + chunkPos.x * CHUNK_SIZE;
                    int wy = y + chunkPos.y * CHUNK_SIZE;
                    int wz = z + chunkPos.z * CHUNK_SIZE;

                    if (chunk.isBlockAir(world, x + 1, y, z)) chunkMesh.addFace(wx, wy, wz, Direction::POS_X); // +X
                    if (chunk.isBlockAir(world, x - 1, y, z)) chunkMesh.addFace(wx, wy, wz, Direction::NEG_X); // -X
                    if (chunk.isBlockAir(world, x, y + 1, z)) chunkMesh.addFace(wx, wy, wz, Direction::POS_Y); // +Y
                    if (chunk.isBlockAir(world, x, y - 1, z)) chunkMesh.addFace(wx, wy, wz, Direction::NEG_Y); // -Y
                    if (chunk.isBlockAir(world, x, y, z + 1)) chunkMesh.addFace(wx, wy, wz, Direction::POS_Z); // +Z
                    if (chunk.isBlockAir(world, x, y, z - 1)) chunkMesh.addFace(wx, wy, wz, Direction::NEG_Z); // -Z
                }
            }
        }
    } // shared lock release


    // lock the mesh queue mutex to provent data from reading at the same time
    {
        std::lock_guard<std::mutex> lock(world.meshQueueMutex);
        world.meshUploadQueue.push(std::move(chunkMesh));
    }
}
    

// function to be called to be run on another thread
// this function is what creates the chunk data to generate the world
void chunkWorker(World& world, ChunkCoords chunkPos) {
    Chunk chunk(world, (int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z);

    // get the current heightmap data, use shared lock becasue I am not writing any data
    {
        std::shared_lock<std::shared_mutex> lock(world.heightmapMutex);
        chunk.createChunk(world.heightmap);

    }


    {
        // lock the world mutex to provent data from reading at the same time
        std::lock_guard<std::shared_mutex> lock(world.worldMutex);
        world.world[ChunkCoords{(int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z}] = std::make_unique<Chunk>(std::move(chunk));
    }

    
    // automatically when chunk creates, add a task to create a mesh for the chunk
    // maybe change this later on
    {
        std::scoped_lock lock(world.chunkStateMutex, world.chunkVertexGenerationQueueMutex);
        auto chunkCoords = ChunkCoords{(int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z};
        auto it = world.chunkStates.find(chunkCoords);
        if (it != world.chunkStates.end() && it->second == ChunkState::QueuedForGeneration) {
            it->second = ChunkState::QueuedForMeshing;
            world.chunkVertexGenerationQueue.push_back(chunkCoords);
        }
            
    }
}



void ChunkMesh::addFace(int wx, int wy, int wz, Direction dir) {
    unsigned int baseIndex = static_cast<unsigned int>(vertices.size());

    int lx = wx - (this->x * CHUNK_SIZE);
    int ly = wy - (this->y * CHUNK_SIZE);
    int lz = wz - (this->z * CHUNK_SIZE);

    // ADDING 4 VERTICES TO THE VERTICY ARRAY
    for (int vert = 0; vert < 4; vert++) {
        Vertex v;
        v.x = (uint8_t)(lx + CubeGeometry::cubeFacePositions[dir][vert].x);
        v.y = (uint8_t)(ly + CubeGeometry::cubeFacePositions[dir][vert].y);
        v.z = (uint8_t)(lz + CubeGeometry::cubeFacePositions[dir][vert].z);
        v.normal = (uint8_t)dir;
        v.u = (uint8_t)CubeGeometry::cubeFaceUVs[vert].x;
        v.v = (uint8_t)CubeGeometry::cubeFaceUVs[vert].y;
        vertices.push_back(v);
    }

    // ADDING 6 INDICES TO CREATE THE FACE
    static const unsigned int faceIndices[6] = { 0, 1, 2, 2, 3, 0 };
    for (int i = 0; i < 6; i++) {
        indices.push_back(baseIndex + faceIndices[i]);
    }
}




Chunk::Chunk(World& world, int x, int y, int z) : x(x), y(y), z(z) {
    // Default test chunk contents to solid so mesh generation has visible faces.
    blocks.fill(Block{1});
}


void Chunk::createChunk(Heightmap& heightmap) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int wx = x + this->x * CHUNK_SIZE;
                int wy = y + this->y * CHUNK_SIZE;
                int wz = z + this->z * CHUNK_SIZE;

                blocks[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] = heightmap.getBlock(wx, wy, wz);
            }
        }
    }
}

bool Chunk::isBlockAir(World& world, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE) return this->isBlockAirOtherChunk(world, x, y, z);
    if (y < 0 || y >= CHUNK_SIZE) return this->isBlockAirOtherChunk(world, x, y, z);
    if (z < 0 || z >= CHUNK_SIZE) return this->isBlockAirOtherChunk(world, x, y, z);
    int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
    return blocks[index].type == 0;
}

bool Chunk::isBlockAirOtherChunk(World& world, int x, int y, int z) {
    int neighborChunkX = this->x;
    int neighborChunkY = this->y;
    int neighborChunkZ = this->z;

    if (x < 0) neighborChunkX -= 1;
    else if (x >= CHUNK_SIZE) neighborChunkX += 1;

    if (y < 0) neighborChunkY -= 1;
    else if (y >= CHUNK_SIZE) neighborChunkY += 1;

    if (z < 0) neighborChunkZ -= 1;
    else if (z >= CHUNK_SIZE) neighborChunkZ += 1;

    auto it = world.world.find(ChunkCoords{neighborChunkX, neighborChunkY, neighborChunkZ});
    if (it == world.world.end()) {
        return false;
    }
    Chunk& neighborChunk = *(it->second);

    // Calculate local coordinates within the neighboring chunk
    int localX = (x + CHUNK_SIZE) % CHUNK_SIZE; // Wrap around to get local coordinate
    int localY = (y + CHUNK_SIZE) % CHUNK_SIZE; // Wrap around to get local coordinate
    int localZ = (z + CHUNK_SIZE) % CHUNK_SIZE; // Wrap around to get local coordinate

    return neighborChunk.isBlockAir(world, localX, localY, localZ);
}




RenderBuffer::RenderBuffer() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);


    size_t stride = sizeof(Vertex);
    // position attribute
    glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, stride, (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, stride, (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // uv
    glVertexAttribPointer(2, 2, GL_UNSIGNED_BYTE, GL_FALSE, stride, (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

}

RenderBuffer::~RenderBuffer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void RenderBuffer::upload(const Vertex* vertices, size_t vertexBytes, const unsigned int* indices, size_t indexBytes) {
    indexCount = indexBytes / sizeof(unsigned int);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexBytes, vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBytes, indices, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
}

void RenderBuffer::uploadChunkMesh(const ChunkMesh& chunkMesh) {
    this->upload(chunkMesh.vertices.data(), chunkMesh.vertices.size() * sizeof(Vertex), chunkMesh.indices.data(), chunkMesh.indices.size() * sizeof(unsigned int));
}

void RenderBuffer::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}