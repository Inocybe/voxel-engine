#pragma once


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <simplex/simplex.h>

struct Block;

class Heightmap {
public:
    Heightmap(int seed1, int seed2, int big_height_scale, int small_height_scale, int min_height, int max_height);
    

    Block getBlock(int x, int y, int z) const;
private:
    int seed1 = 42;
    int seed2 = 13370;
    int big_height_scale = 400;
    int small_height_scale = 6;
    int min_height = -10;
    int max_height = 50;

    std::unique_ptr<SimplexNoise> noise;


    void getBigDetails();
    void getSmallDetails();
};