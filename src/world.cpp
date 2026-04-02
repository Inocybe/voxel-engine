#include <world.hpp>


bool Chunk::isBlockAir(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE)   return true;
    if (y < 0 || y >= CHUNK_SIZE)  return true;
    if (z < 0 || z >= CHUNK_SIZE)   return true;
    int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
    return blocks[index].type == 0;
}

void Chunk::createBaseChunk() {
    for (int i = 0; i < blocks.size(); i++) {
        Block type;
        type.type = 1;
        blocks[i] = type;
    }
    MeshData meshData;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                 int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
                Block b = blocks[index];

                // Skip air blocks entirely -- nothing to draw.
                // "continue" jumps to the next loop iteration immediately.
                if (b.type == 0) continue;

                // Face culling -- only add a face if the neighbor is air.
                // A face shared between two solid blocks is NEVER visible,
                // so adding it just wastes GPU time drawing nothing.
                // World position = chunk position * chunk size + local position.
                int wx = x;
                int wy = y;
                int wz = z;

                if (this->isBlockAir(x + 1, y, z)) meshData.addFace(wx, wy, wz, Direction::POS_X); // +X
                if (this->isBlockAir(x - 1, y, z)) meshData.addFace(wx, wy, wz, Direction::NEG_X); // -X
                if (this->isBlockAir(x, y + 1, z)) meshData.addFace(wx, wy, wz, Direction::POS_Y); // +Y
                if (this->isBlockAir(x, y - 1, z)) meshData.addFace(wx, wy, wz, Direction::NEG_Y); // -Y
                if (this->isBlockAir(x, y, z + 1)) meshData.addFace(wx, wy, wz, Direction::POS_Z); // +Z
                if (this->isBlockAir(x, y, z - 1)) meshData.addFace(wx, wy, wz, Direction::NEG_Z); // -Z
            }
        }
    }

    mesh.upload(
        meshData.vertices.data(), 
        meshData.vertices.size() * sizeof(Vertex), 
        meshData.indices.data(), 
        meshData.indices.size() * sizeof(unsigned int)
    );
}





void Chunk::createRandomChunk() {
    for (int i = 0; i < blocks.size(); i++) {

    }
}
void Chunk::draw() const {
    mesh.draw();
}


//MeshData::MeshData(World& world) : world(world) {} TEMPORARY


void MeshData::addFace(int wx, int wy, int wz, Direction dir) {
    // ADDING 4 VERTICES TO THE VERTICY ARRAY
    for (int vert = 0; vert < 4; vert++) {
        Vertex v;
        
        v.position = cubeFacePositions[dir][vert] + glm::vec3(wx, wy, wz);
        v.normal = cubeFaceNormals[dir];
        v.uv = cubeFaceUVs[vert];

        vertices.push_back(v);
    }

    // ADDING 6 INDICES TO CREATE THE FACE
    static const unsigned int faceIndices[6] = { 0, 1, 2, 2, 3, 0 };
    unsigned int indexOffset = dir * 4;
    for (int i = 0; i < 6; i++) {
        indices.push_back(indexOffset + faceIndices[i]);
    }
}





ChunkMesh::ChunkMesh() {
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

ChunkMesh::~ChunkMesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void ChunkMesh::upload(const Vertex* vertices, size_t vertexBytes, const unsigned int* indices, size_t indexBytes) {
    indexCount = indexBytes / sizeof(unsigned int);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexBytes, vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBytes, indices, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
}

void ChunkMesh::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}




void World::drawChunks() const {
    for (const auto& [location, data] : world) {
        data.draw();
    }
}