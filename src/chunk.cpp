#include <chunk.hpp>
#include <world.hpp>



// function to be called to be run on another thread
// this function is what creates the mesh data to mesh
void meshWorker(World& world, glm::ivec3 chunkPos) {
    ChunkMesh chunkMesh;

    chunkMesh.x = (int)chunkPos.x;
    chunkMesh.y = (int)chunkPos.y;
    chunkMesh.z = (int)chunkPos.z;

    {
        // lock the world mutex to provent data from reading at the same time
        std::lock_guard<std::mutex> lock(world.worldMutex);

        auto it = world.world.find(std::make_tuple((int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z));
        if (it == world.world.end()) {
            std::cerr << "Chunk not found at position: " << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << std::endl;
            return;
        }
        Chunk& chunk = *(it->second);

        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    printf("Processing block at local position: %d, %d, %d\n", x, y, z);
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

                    if (chunk.isBlockAir(x + 1, y, z)) chunkMesh.addFace(wx, wy, wz, Direction::POS_X); // +X
                    if (chunk.isBlockAir(x - 1, y, z)) chunkMesh.addFace(wx, wy, wz, Direction::NEG_X); // -X
                    if (chunk.isBlockAir(x, y + 1, z)) chunkMesh.addFace(wx, wy, wz, Direction::POS_Y); // +Y
                    if (chunk.isBlockAir(x, y - 1, z)) chunkMesh.addFace(wx, wy, wz, Direction::NEG_Y); // -Y
                    if (chunk.isBlockAir(x, y, z + 1)) chunkMesh.addFace(wx, wy, wz, Direction::POS_Z); // +Z
                    if (chunk.isBlockAir(x, y, z - 1)) chunkMesh.addFace(wx, wy, wz, Direction::NEG_Z); // -Z
                }
            }
        }
    }

    // lock the mesh queue mutex to provent data from reading at the same time
    {
        std::lock_guard<std::mutex> lock(world.meshQueueMutex);
        world.meshUploadQueue.push(std::move(chunkMesh));
    }
}
    


void ChunkMesh::addFace(int wx, int wy, int wz, Direction dir) {
    unsigned int baseIndex = static_cast<unsigned int>(vertices.size());

    // ADDING 4 VERTICES TO THE VERTICY ARRAY
    for (int vert = 0; vert < 4; vert++) {
        Vertex v;
        v.position = CubeGeometry::cubeFacePositions[dir][vert] + glm::vec3(wx, wy, wz);
        v.normal = CubeGeometry::cubeFaceNormals[dir];
        v.uv = CubeGeometry::cubeFaceUVs[vert];
        vertices.push_back(v);
    }

    // ADDING 6 INDICES TO CREATE THE FACE
    static const unsigned int faceIndices[6] = { 0, 1, 2, 2, 3, 0 };
    for (int i = 0; i < 6; i++) {
        indices.push_back(baseIndex + faceIndices[i]);
    }
}




Chunk::Chunk(int x, int y, int z) : x(x), y(y), z(z) {
    // Default test chunk contents to solid so mesh generation has visible faces.
    blocks.fill(Block{1});
}


bool Chunk::isBlockAir(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE)   return true;
    if (y < 0 || y >= CHUNK_SIZE)  return true;
    if (z < 0 || z >= CHUNK_SIZE)   return true;
    int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
    return blocks[index].type == 0;
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, uv));
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