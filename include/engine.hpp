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

#include <camera.hpp>


enum class InputAction {
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
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


class Engine {
public:
    GLFWwindow*  window;
    InputManager inputManager;
    Camera       camera;        // owns all camera state
    float        deltaTime = 0.0f;

    Engine(unsigned int screenWidth, unsigned int screenHeight, const char* windowName);
    ~Engine();

    bool Run();
    void EndFrame();

    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();

    void process_input();
    void calculate_delta();

    static void error_callback(int id, const char* descriptor);
    static void scroll_callback(GLFWwindow* w, double xOffset, double yOffset);
    static void mouse_callback(GLFWwindow* w, double xpos, double ypos);
    static void framebuffer_size_callback(GLFWwindow* w, int width, int height);

    glm::vec3& getCameraPosLocation();
    glm::vec3  getCameraPos();
    GLFWwindow* getWindow() const { return window; }

private:
    unsigned int screen_width;
    unsigned int screen_height;
    float lastFrame = 0.0f;

    void on_scroll(GLFWwindow* w, double xOffset, double yOffset);
    void on_mouse_move(GLFWwindow* w, double xpos, double ypos);
    void on_framebuffer_size(GLFWwindow* w, int width, int height);
};