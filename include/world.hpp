#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <array>
#include <mutex>
#include <queue>
#include <vector>
#include <random>
#include <unordered_map>

#include <thread_pool.hpp>
#include <chunk.hpp>




struct TupleHash {
    size_t operator()(const std::tuple<int, int, int>& t) const {
        auto h1 = std::hash<int>{}(std::get<0>(t));
        auto h2 = std::hash<int>{}(std::get<1>(t));
        auto h3 = std::hash<int>{}(std::get<2>(t));
        return h1 ^ (h2 << 32) ^ (h3 << 16);
    }
};


class World {
public:
    World(glm::vec3& cameraPos);
    std::unordered_map<std::tuple<int, int, int>, std::unique_ptr<Chunk>, TupleHash> world;
    std::mutex worldMutex;

    std::queue<MeshData> meshUploadQueue;
    std::mutex meshQueueMutex;
    std::condition_variable meshReadyCV;

    //std::atomic<bool> running = true;

    void update() const;
private:
    glm::vec3& cameraPos;

    void drawChunks() const;

};