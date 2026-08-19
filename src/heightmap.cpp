#include <heightmap.hpp>
#include <chunk.hpp>
#include <cmath>

Heightmap::Heightmap(int seed1, int seed2, int min_height, int max_height) : seed1(seed1), seed2(seed2), min_height(min_height), max_height(max_height) {}









Block Heightmap::getBlock(int x, int y, int z) const {
    int height = getHeight(x, z);

    if (y < height) {
        return Block{1}; // Solid block
    } else {
        return Block{0}; // Air block
    }
}



float Heightmap::fbm(float x, float z) const {
    float sum = 0.0f;
    float frequency = base_frequency;
    float amplitude = 1.0f;
    float max_amp = 0.0f; // Used for normalizing result to [0,1]

    for (int i = 0; i < octaves; ++i) {
        sum += Simplex::noise(glm::vec2(x * frequency + seed1, z * frequency + seed2)) * amplitude;

        max_amp += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return sum / max_amp; // Normalize the result
}


float Heightmap::ridgedFbm(float x, float z) const {
    float sum = 0.0f;
    float frequency = base_frequency;
    float amplitude = 1.0f;
    float max_amp = 0.0f; // Used for normalizing result to [0,1]

    for (int i = 0; i < octaves; ++i) {
        float noise_value = Simplex::noise(glm::vec2(x * frequency + seed1, z * frequency + seed2));
        noise_value = 1.0f - std::abs(noise_value); // Invert and take absolute value
        noise_value *= noise_value; // Square the value to sharpen it

        sum += noise_value * amplitude;
        max_amp += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return sum / max_amp; // Normalize the result
}


int Heightmap::getHeight(int x, int z) const {
    float height = fbm(x, z);
    float ridges = ridgedFbm(x, z);

    // create low frequency mask for deciding where mountains to form 
    float mask = Simplex::noise(glm::vec2(x * base_frequency * 0.15f, z * base_frequency * 0.15f));
    float mountain_factor = glm::clamp((mask + 1.0f) * 0.5f, 0.0f, 1.0f); // Normalize mask to [0,1]

    float n01 = (height + 1.0f) * 0.5f;
    float shaped = std::pow(n01, redistribution);
    float combined = glm::clamp(shaped + ridges * mountain_factor * 0.6f, 0.0f, 1.0f);

    return min_height + (int)(shaped * (max_height - min_height));
}