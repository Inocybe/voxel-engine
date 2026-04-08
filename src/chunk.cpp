#include <chunk.hpp>


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


void MeshData::addFace(int wx, int wy, int wz, Direction dir) {
    unsigned int baseIndex = static_cast<unsigned int>(vertices.size());

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
    for (int i = 0; i < 6; i++) {
        indices.push_back(baseIndex + faceIndices[i]);
    }
}




Chunk::Chunk(int x, int y, int z) : x(x), y(y), z(z) {}


bool Chunk::isBlockAir(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE)   return false;
    if (y < 0 || y >= CHUNK_SIZE)  return false;
    if (z < 0 || z >= CHUNK_SIZE)   return false;
    int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
    return blocks[index].type == 0;
}


void Chunk::createRandomChunk() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 3);
    int random_number = distrib(gen);
    for (int i = 0; i < blocks.size(); i++) {
        int random_number = distrib(gen);
        Block type;
        type.type = random_number;
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
                int wx = x + this->x * CHUNK_SIZE;
                int wy = y + this->y * CHUNK_SIZE;
                int wz = z + this->z * CHUNK_SIZE;

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

void Chunk::draw() const {
    mesh.draw();
}
