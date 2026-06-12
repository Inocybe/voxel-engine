#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    bool isMouseCaptured = true;

    Camera(float screenWidth, float screenHeight,
           glm::vec3 startPos = glm::vec3(0.0f, 70.0f, 0.0f));

    // Called from Engine's GLFW callbacks
    void onMouseMove(double xpos, double ypos);
    void onScroll(double xOffset, double yOffset);
    void onFramebufferResize(int width, int height);

    // Called each frame from process_input
    void processMovement(GLFWwindow* window, float deltaTime);

    // Toggle capture state; both reset firstMouse to avoid a jump on re-capture
    void captureMouse();
    void releaseMouse();

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::vec3& getPosRef();   // reference so World/Player can hold a pointer to it
    glm::vec3  getPos() const;

private:
    glm::vec3 pos;
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up    = glm::vec3(0.0f, 1.0f,  0.0f);

    float yaw   = -90.0f;
    float pitch =   0.0f;
    float fov   =  45.0f;

    float lastX, lastY;
    float aspectRatio;      // stored so scroll-triggered FOV changes can rebuild projection
    bool  firstMouse = true;

    glm::mat4 projection;

    void rebuildProjection();
    void updateDirectionVectors();
};