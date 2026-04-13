#pragma once


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <simplex/simplex.h>

#include <chunk.hpp>

class Heightmap {
public:
    int seed1 = 42;
    int seed2 = 1337;
    int MIN_2D_NOISE_HEIGHT = 0;
    int MAX_2D_NOISE_HEIGHT = 100;
    
    Heightmap();

private:

    void getBigDetails();
    void getSmallDetails();
};