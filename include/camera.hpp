#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace CameraDirection {
    enum Type {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };
}


class Camera {
public:
    Camera(unsigned int screenWidth = 800, unsigned int screenHeight = 600);

    // Mouse input handling
    void rotateFromMouse(float deltaX, float deltaY, float sensitivity = 0.2f);
    void resetMouseState();

    // Scroll input handling
    void adjustFOV(float delta);

    // Movement input handling
    void moveForward(float deltaTime);
    void moveBackward(float deltaTime);
    void moveLeft(float deltaTime);
    void moveRight(float deltaTime);
    void setSprintMode(bool isSprinting);

    // Matrix getters
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    // Position getters
    glm::vec3& getCameraPosLocation();   
    glm::vec3 getCameraPos() const;

    // Projection update (call when window resizes)
    void updateProjectionMatrix(unsigned int screenWidth, unsigned int screenHeight);

private:
    // Camera state
    glm::vec3 cameraPos   = glm::vec3(0.0f, 70.0f, 0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

    // Rotation angles
    float yaw = -90.0f;
    float pitch = 0.0f;

    // Field of view
    float fov = 45.0f;

    // Movement
    float normalSpeed = 5.0f;
    float sprintMultiplier = 5.0f;
    bool isSprinting = false;

    // Mouse state
    bool firstMouse = true;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;

    // Projection matrix
    unsigned int screenWidth;
    unsigned int screenHeight;
    glm::mat4 m_projection;

    // Helper to update direction vector from yaw/pitch
    void updateCameraDirection();
};