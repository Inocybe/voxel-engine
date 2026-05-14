#pragma once


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <simplex/simplex.h>

struct Block;

class Heightmap {
public:
    int seed1 = 42;
    int seed2 = 13370;
    int big_height_scale = 40;
    int small_height_scale = 6;
    int min_height = -10;
    int max_height = 50;
    
    Block getBlock(int x, int y, int z) const;
private:

    void getBigDetails();
    void getSmallDetails();
};