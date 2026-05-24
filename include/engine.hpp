#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <vector>

//enum class InputAction;
//class InputManager;

/*
CONVNETION TO TRY TO OPEN WINDOW
THIS WILL PREVENT ANY ERRORS

    Engine* engine = nullptr;
    try {
        engine = new Engine(SCR_WIDTH, SCR_HEIGHT, "OpenGL Window");
    } catch (const std::exception& e) {
        std::cerr << "Window initizliation failed" << e.what() << std::endl;
        return -1;
    }

*/
class Engine {
public:
    GLFWwindow* window;
    InputManager inputManager;
    float deltaTime = 0.0;

    // ima just default this to default monitor and window
    Engine(unsigned int screenWidth, unsigned int screenHeight, const char* windowName);
    ~Engine();

    bool Run();
    void EndFrame();
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();

    void process_input();
    void calculate_delta();

    static void error_callback(int id, const char* discriptor);
    static void scroll_callback(GLFWwindow* windowInstance, double xOffset, double yOffset);
    static void mouse_callback(GLFWwindow* windowInstance, double xpos, double ypos);
    static void framebuffer_size_callback(GLFWwindow* windowInstance, int width, int height);

    // function useful for setting pointer to camrea location 
    // can be read from at any time if set to a variable, so no need to call a function
    // #TODO implement a glm::vec3 changeCameraPos(); so that it will update and keep same memory address
    glm::vec3& getCameraPosLocation();   
    glm::vec3 getCameraPos();
    GLFWwindow* getWindow() const { return window; }
private:
    // window vars
    unsigned int screen_width;
    unsigned int screen_height;

    // CAMERA VARS
    glm::vec3 cameraPos   = glm::vec3(0.0f, 70.0f,  0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    bool firstMouse = true;
    bool isMouseCaptured = true;
    float yaw = -90.0f;
    float pitch = 0.0f; 
    float fov = 45.0f;
    glm::mat4 m_projection = glm::perspective(glm::radians(fov), (float)screen_width / (float)screen_height, 0.1f, 1000.0f);


    // MOUSE VARS
    float lastX = screen_width / 2.0f;
    float lastY = screen_height / 2.0f;

    // thing to store for delta time
    float lastFrame = 0.0;


    void on_scroll(GLFWwindow* windowInstance, double xOffset, double yOffset);
    void on_mouse_move(GLFWwindow* windowInstance, double xpos, double ypos);
    void on_framebuffer_size(GLFWwindow* windowInstance, int width, int height);
};


enum class InputAction {
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Sprint,
    ReloadWorld,
};

enum class InputType {
    Press,
    Hold
};

class InputManager {
struct Bindings {
    InputAction action;
    InputType type;
};

public:
    void bindKey(int key, InputAction action, InputType type = InputType::Press) {
        bindings[key] = {action, type};
    }

    void subscribe(InputAction action, std::function<void()> callback) {
        callbacks[action].push_back(callback);
    }

    void update(GLFWwindow* window) {
        for (const auto& [key, binding] : bindings) {
            int currentState = glfwGetKey(window, key);
            bool isPressed = currentState == GLFW_PRESS;
            bool wasPressed = previousStates[key];

            bool shouldTrigger = false;

            if (binding.type == InputType::Hold && isPressed) {
                shouldTrigger = true;
            } else if (binding.type == InputType::Press && isPressed && !wasPressed) {
                shouldTrigger = true;
            }

            if (shouldTrigger) {
                executeAction(key);
            }

            previousStates[key] = isPressed;
        }
    }   



private:
    void executeAction(int key) {
        auto it = bindings.find(key);
        if (it != bindings.end()) {
            InputAction action = it->second.action;
            auto cbIt = callbacks.find(action);
            if (cbIt != callbacks.end()) {
                for (const auto& callback : cbIt->second) {
                    callback();
                }
            }
        }
    }

    std::unordered_map<int, Bindings> bindings;    
    std::unordered_map<int, bool> previousStates;
    std::unordered_map<InputAction, std::vector<std::function<void()>>> callbacks;
};