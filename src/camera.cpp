#include <camera.hpp>

Camera::Camera(unsigned int screenWidth, unsigned int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight) {
    updateProjectionMatrix(screenWidth, screenHeight);
}

void Camera::rotateFromMouse(float deltaX, float deltaY, float sensitivity) {
    deltaX *= sensitivity;
    deltaY *= sensitivity;

    yaw += deltaX;
    pitch -= deltaY;

    // Clamp pitch to avoid flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    updateCameraDirection();
}

void Camera::resetMouseState() {
    firstMouse = true;
}

void Camera::adjustFOV(float delta) {
    fov -= delta;
    fov = glm::clamp(fov, 1.0f, 45.0f);
    // Update projection matrix to reflect new FOV
    updateProjectionMatrix(screenWidth, screenHeight);
}

void Camera::moveForward(float deltaTime) {
    float speed = normalSpeed * deltaTime;
    if (isSprinting)
        speed *= sprintMultiplier;
    cameraPos += speed * cameraFront;
}

void Camera::moveBackward(float deltaTime) {
    float speed = normalSpeed * deltaTime;
    if (isSprinting)
        speed *= sprintMultiplier;
    cameraPos -= speed * cameraFront;
}

void Camera::moveLeft(float deltaTime) {
    float speed = normalSpeed * deltaTime;
    if (isSprinting)
        speed *= sprintMultiplier;
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
}

void Camera::moveRight(float deltaTime) {
    float speed = normalSpeed * deltaTime;
    if (isSprinting)
        speed *= sprintMultiplier;
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
}

void Camera::setSprintMode(bool sprint) {
    isSprinting = sprint;
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return m_projection;
}

glm::vec3& Camera::getCameraPosLocation() {
    return cameraPos;
}

glm::vec3 Camera::getCameraPos() const {
    return cameraPos;
}

void Camera::updateProjectionMatrix(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
    m_projection = glm::perspective(
        glm::radians(fov),
        (float)width / (float)height,
        0.1f,
        1000.0f
    );
}

void Camera::updateCameraDirection() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}