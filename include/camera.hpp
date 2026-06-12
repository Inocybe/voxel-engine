#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    // Constructor and destructor
    Camera(float screenWidth, float screenHeight);
    ~Camera();

    // Get matrices
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    // Process input
    void ProcessMouseMovement(double xpos, double ypos);
    void ProcessMouseScroll(double yOffset);
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // Camera control
    void SetMouseCapture(bool captured);
    
    // Position access
    glm::vec3& GetCameraPosition();
    glm::vec3 GetCameraPosition() const;
    void SetCameraPosition(const glm::vec3& position);
    
    // Window resize handling
    void OnFrameBufferSizeChanged(int width, int height);
    
    // Update projection matrix with new FOV and aspect ratio
    void UpdateProjection(float fov, float aspectRatio);

private:
    // Camera properties
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraFront;
    glm::vec3 m_cameraUp;
    
    // Mouse tracking
    bool m_firstMouse;
    bool m_isMouseCaptured;
    float m_yaw;
    float m_pitch;
    float m_fov;
    
    // Screen dimensions
    float m_screenWidth;
    float m_screenHeight;
    
    // Mouse position
    float m_lastX;
    float m_lastY;
    
    // Projection matrix
    glm::mat4 m_projection;
};