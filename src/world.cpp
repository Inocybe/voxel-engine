#include <world.hpp>


bool Chunk::isAir(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE)   return true;
    if (y < 0 || y >= CHUNK_SIZE)  return true;
    if (z < 0 || z >= CHUNK_SIZE)   return true;
    int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
    return blocks[index].type == 0;
}


void MeshData::addFace(int direction) {
    // TODO
}