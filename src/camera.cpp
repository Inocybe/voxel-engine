#include "camera.hpp"

Camera::Camera(float screenWidth, float screenHeight) 
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight),
      m_cameraPos(0.0f, 70.0f, 0.0f),
      m_cameraFront(0.0f, 0.0f, -1.0f),
      m_cameraUp(0.0f, 1.0f, 0.0f),
      m_yaw(-90.0f), m_pitch(0.0f),
      m_fov(45.0f), m_firstMouse(true),
      m_isMouseCaptured(true),
      m_lastX(screenWidth / 2.0f), m_lastY(screenHeight / 2.0f) {
    
    // Initialize projection matrix
    m_projection = glm::perspective(glm::radians(m_fov), screenWidth / screenHeight, 0.1f, 1000.0f);
}

Camera::~Camera() = default;

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return m_projection;
}

void Camera::ProcessMouseMovement(double xpos, double ypos) {
    if (!m_isMouseCaptured) return;

    if (m_firstMouse) {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }

    float xOffset = xpos - m_lastX;
    float yOffset = ypos - m_lastY;
    m_lastX = xpos;
    m_lastY = ypos;

    const float sensitivity = 0.2f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    m_yaw += xOffset;
    m_pitch -= yOffset;

    if(m_pitch > 89.0f)
        m_pitch =  89.0f;
    if(m_pitch < -89.0f)
        m_pitch = -89.0f;

    
    glm::vec3 direction;
    direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    direction.y = sin(glm::radians(m_pitch));
    direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_cameraFront = glm::normalize(direction);
}

void Camera::ProcessMouseScroll(double yOffset) {
    m_fov -= (float)yOffset;
    m_fov = glm::clamp(m_fov, 1.0f, 45.0f);
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
    const float normalCameraSpeed = 5.0f * deltaTime;
    const float fastCameraSpeed = normalCameraSpeed * 20.0f;
    float currentCameraSpeed = normalCameraSpeed;

    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        currentCameraSpeed = fastCameraSpeed;

    switch (direction) {
        case CameraMovement::FORWARD:
            m_cameraPos += currentCameraSpeed * m_cameraFront;
            break;
        case CameraMovement::BACKWARD:
            m_cameraPos -= currentCameraSpeed * m_cameraFront;
            break;
        case CameraMovement::LEFT:
            m_cameraPos -= glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * currentCameraSpeed;
            break;
        case CameraMovement::RIGHT:
            m_cameraPos += glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * currentCameraSpeed;
            break;
        case CameraMovement::UP:
            m_cameraPos += m_cameraUp * currentCameraSpeed;
            break;
        case CameraMovement::DOWN:
            m_cameraPos -= m_cameraUp * currentCameraSpeed;
            break;
    }
}

void Camera::SetMouseCapture(bool captured) {
    m_isMouseCaptured = captured;
    m_firstMouse = true;
}

glm::vec3& Camera::GetCameraPosition() {
    return m_cameraPos;
}

glm::vec3 Camera::GetCameraPosition() const {
    return m_cameraPos;
}

void Camera::SetCameraPosition(const glm::vec3& position) {
    m_cameraPos = position;
}

void Camera::OnFrameBufferSizeChanged(int width, int height) {
    glViewport(0, 0, width, height);
    m_projection = glm::perspective(glm::radians(m_fov), (float)width / (float)height, 0.1f, 1000.0f);
}

void Camera::UpdateProjection(float fov, float aspectRatio) {
    m_fov = fov;
    m_projection = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
}