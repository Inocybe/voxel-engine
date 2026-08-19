#pragma once


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <simplex/simplex.h>

struct Block;

class Heightmap {
public:
    Heightmap() = default;
    Heightmap(int seed1, int seed2, int min_height, int max_height);
    

    Block getBlock(int x, int y, int z) const;
    int getHeight(int x, int z) const;
private:
    int seed1 = 42;
    int seed2 = 13370;
    int min_height = -10;
    int max_height = 180;

    int octaves = 5;
    float lacunarity = 2.0f;
    float persistence = 0.5f;
    float base_frequency = 0.004f;

    float redistribution = 3.0f;


    float fbm(float x, float z) const;
    float ridgedFbm(float x, float z) const;
};