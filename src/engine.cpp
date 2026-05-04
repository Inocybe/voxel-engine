#include <engine.hpp>

Engine::Engine(unsigned int screenWidth, unsigned int screenHeight, const char* windowName) : 
screen_height(screenHeight), screen_width(screenWidth) {
    glfwSetErrorCallback(Engine::error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // INITILIZE WINDOW
    window = glfwCreateWindow(screenWidth, screenHeight, windowName, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);

    // INITILIZE GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initizliate GLAD");
    }


    // SETTING RESIZE FUNCTION
    glfwSetFramebufferSizeCallback(window, Engine::framebuffer_size_callback);
    // SETTING MOUSE FUNCTION
    glfwSetCursorPosCallback(window, Engine::mouse_callback);  
    // SETTING SCROLL FUNCTION
    glfwSetScrollCallback(window, Engine::scroll_callback); 

    // Initialize viewport/projection for the initial framebuffer size.
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    on_framebuffer_size(window, fbWidth, fbHeight);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

}

Engine::~Engine() {
    glfwTerminate();
}

bool Engine::Run() {
    if (glfwWindowShouldClose(window))
        return false;

    // poll events
    glfwPollEvents();

    // input
    // ------
    Engine::calculate_delta();
    Engine::process_input();

    // Clear buffers
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    return true;
}

void Engine::EndFrame() {
    glfwSwapBuffers(window);
}

// Optional: Add helper to get view/projection matrices
glm::mat4 Engine::GetViewMatrix() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 Engine::GetProjectionMatrix() {
    return m_projection;
}



void Engine::error_callback(int id, const char* discriptor) {
    std::cout << discriptor << std::endl;
}



void Engine::framebuffer_size_callback(GLFWwindow* windowInstance, int width, int height) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(windowInstance));
    if (engine) engine->on_framebuffer_size(windowInstance, width, height);
}
void Engine::on_framebuffer_size(GLFWwindow* windowInstance, int width, int height) {
    glViewport(0, 0, width, height);
    // set the projection matrix to the new aspect ratio
    m_projection = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 1000.0f);
}



void Engine::scroll_callback(GLFWwindow* windowInstance, double xOffset, double yOffset) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(windowInstance));
    if (engine) engine->on_scroll(windowInstance, xOffset, yOffset);
}
void Engine::on_scroll(GLFWwindow* windowInstance, double xOffset, double yOffset) {
    fov -= (float)yOffset;
    fov = glm::clamp(fov, 1.0f, 45.0f);
}



void Engine::mouse_callback(GLFWwindow* windowInstance, double xpos, double ypos) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(windowInstance));
    if (engine) engine->on_mouse_move(windowInstance, xpos, ypos);
}
void Engine::on_mouse_move(GLFWwindow* windowInstance, double xpos, double ypos) {
    if (!isMouseCaptured) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.2f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch -= yOffset;

    if(pitch > 89.0f)
        pitch =  89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}


void Engine::calculate_delta() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}


void Engine::process_input() {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        isMouseCaptured = false;
        firstMouse = true; // reset first mouse so that when the mouse is captured again it doesn't cause a sudden jump in camera direction
        //glfwSetWindowShouldClose(window, true);
    }
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        isMouseCaptured = true;
        firstMouse = true; // reset first mouse so that when the mouse is captured again it doesn't cause a sudden jump in camera direction
    }
    // camera movement
    const float normalCameraSpeed = 5.0f * deltaTime;
    const float fastCameraSpeed = normalCameraSpeed * 20.0f;
    float currentCameraSpeed = normalCameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        currentCameraSpeed = fastCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        currentCameraSpeed = normalCameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += currentCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= currentCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentCameraSpeed;


}


glm::vec3& Engine::getCameraPosLocation() {
    return cameraPos;
}
glm::vec3 Engine::getCameraPos() {
    return cameraPos;
}