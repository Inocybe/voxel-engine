#include <heightmap.hpp>
#include <chunk.hpp>


Block Heightmap::getBlock(int x, int y, int z) const {
    int bigDetails = (int)(Simplex::noise(glm::vec2(x, z)) * big_height_scale);
    int smallDetails = (int)(Simplex::noise(glm::vec2(x + seed2, z + seed2)) * small_height_scale);
    
    // if the y coordinate is below the height, return a solid block, otherwise return air
    if (y < bigDetails + smallDetails) {
        return Block{1}; // solid block
    } else {
        return Block{0}; // air block
    }
}