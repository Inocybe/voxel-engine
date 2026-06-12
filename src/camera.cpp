#include <camera.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float screenWidth, float screenHeight, glm::vec3 startPos)
    : pos(startPos),
      lastX(screenWidth  / 2.0f),
      lastY(screenHeight / 2.0f),
      aspectRatio(screenWidth / screenHeight)
{
    rebuildProjection();
}

// ── Matrix accessors ──────────────────────────────────────────────────────────

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(pos, pos + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return projection;
}

glm::vec3& Camera::getPosRef() { return pos; }
glm::vec3  Camera::getPos()  const { return pos; }

// ── Mouse capture ─────────────────────────────────────────────────────────────

void Camera::captureMouse() {
    isMouseCaptured = true;
    firstMouse = true;  // prevent jump when cursor re-enters
}

void Camera::releaseMouse() {
    isMouseCaptured = false;
    firstMouse = true;
}

// ── GLFW event handlers ───────────────────────────────────────────────────────

void Camera::onMouseMove(double xpos, double ypos) {
    if (!isMouseCaptured) return;

    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    const float sensitivity = 0.2f;
    float xOffset = ((float)xpos - lastX) * sensitivity;
    float yOffset = (lastY - (float)ypos) * sensitivity;  // inverted: moving mouse down = looking down
    lastX = (float)xpos;
    lastY = (float)ypos;

    yaw   += xOffset;
    pitch += yOffset;
    pitch  = glm::clamp(pitch, -89.0f, 89.0f);

    updateDirectionVectors();
}

void Camera::onScroll(double /*xOffset*/, double yOffset) {
    fov -= (float)yOffset;
    fov  = glm::clamp(fov, 1.0f, 45.0f);
    rebuildProjection();  // FOV changed, so rebuild
}

void Camera::onFramebufferResize(int width, int height) {
    aspectRatio = (float)width / (float)height;
    rebuildProjection();
}

// ── Per-frame movement ────────────────────────────────────────────────────────

void Camera::processMovement(GLFWwindow* window, float deltaTime) {
    const float normalSpeed = 5.0f * deltaTime;
    const float fastSpeed   = normalSpeed * 20.0f;
    float speed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? fastSpeed : normalSpeed;

    glm::vec3 right = glm::normalize(glm::cross(front, up));

    if (glfwGetKey(window, GLFW_KEY_W)            == GLFW_PRESS) pos += speed * front;
    if (glfwGetKey(window, GLFW_KEY_S)            == GLFW_PRESS) pos -= speed * front;
    if (glfwGetKey(window, GLFW_KEY_A)            == GLFW_PRESS) pos -= speed * right;
    if (glfwGetKey(window, GLFW_KEY_D)            == GLFW_PRESS) pos += speed * right;
    if (glfwGetKey(window, GLFW_KEY_SPACE)        == GLFW_PRESS) pos += speed * up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) pos -= speed * up;
}

// ── Private helpers ───────────────────────────────────────────────────────────

void Camera::rebuildProjection() {
    projection = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
}

void Camera::updateDirectionVectors() {
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(dir);
}