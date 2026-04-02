#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <memory>
#include <chrono>
#include <mutex>

// my own stuff I made to link
#include <world.hpp>
#include <shader.hpp>
#include <texture.hpp>
#include <engine.hpp>

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;


int main() {
    // LOAD ENGINE
    std::unique_ptr<Engine> engine;
    try {
        engine = std::make_unique<Engine>(SCR_WIDTH, SCR_HEIGHT, "OpenGL Window");
    } catch (const std::exception& e) {
        std::cerr << "Window initizliation failed" << e.what() << std::endl;
        return -1;
    }
/*
    Shader shader("../shaders/shader.vs", "../shaders/shader.fs");
    //World world(*engine);
    World world(*engine);

    shader.use();
    shader.setVec3("uCameraPos", engine->getCameraPos());
    shader.setInt("uAlbedo", 0);


*/

    Shader shader("../shaders/shader.vs", "../shaders/shader.fs");
    shader.use();


    World world;
    std::unique_ptr<Chunk> defaultChunk = std::make_unique<Chunk>();
    defaultChunk->createBaseChunk();

    world.world.emplace(std::make_tuple(0, 0, 0), std::move(defaultChunk));
    


    // RENDER LOOP
    while(engine->Run()) {
        // use matrix's unifrom location and set matrix
        shader.use();

        // Set view and projection and position
        shader.setMat4("view", engine->GetViewMatrix());
        shader.setMat4("projection", engine->GetProjectionMatrix((float)SCR_WIDTH / (float)SCR_HEIGHT));

        world.drawChunks();        


        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);

        engine->EndFrame();
    }
    
    return 0;
}