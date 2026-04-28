#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

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

    Shader shader("../shaders/shader.vs", "../shaders/shader.fs");
    shader.use();
    

    World world(engine->getCameraPosLocation());
    world.makeTestingMap(4); // creates a 3x3x3 of chunks centered around the origin, each chunk is 16x16x16 blocks
    

    // RENDER LOOP
    while(engine->Run()) {
        // use matrix's unifrom location and set matrix
        shader.use();

        // Set view and projection and position
        shader.setMat4("view", engine->GetViewMatrix());
        shader.setMat4("projection", engine->GetProjectionMatrix());


        world.update();        


        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);

        engine->EndFrame();
    }
    
    return 0;
}