#include <engine.hpp>

Engine::Engine(unsigned int screenWidth, unsigned int screenHeight, const char* windowName) : 
screen_height(screenHeight), screen_width(screenWidth), camera(screenWidth, screenHeight) {
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

    // Setup input bindings for camera movement
    inputManager.bindKey(GLFW_KEY_W, InputAction::MoveForward, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_S, InputAction::MoveBackward, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_A, InputAction::MoveLeft, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_D, InputAction::MoveRight, InputType::Hold);
    inputManager.bindKey(GLFW_KEY_LEFT_SHIFT, InputAction::Sprint, InputType::Hold);

    // Subscribe input actions to camera methods
    inputManager.subscribe(InputAction::MoveForward, [this]() { camera.moveForward(deltaTime); });
    inputManager.subscribe(InputAction::MoveBackward, [this]() { camera.moveBackward(deltaTime); });
    inputManager.subscribe(InputAction::MoveLeft, [this]() { camera.moveLeft(deltaTime); });
    inputManager.subscribe(InputAction::MoveRight, [this]() { camera.moveRight(deltaTime); });
    inputManager.subscribe(InputAction::Sprint, [this]() { camera.setSprintMode(true); });

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
    
    this->inputManager.update(this->getWindow());

    return true;
}

void Engine::EndFrame() {
    glfwSwapBuffers(window);
}




void Engine::error_callback(int id, const char* discriptor) {
    std::cout << discriptor << std::endl;
}



void Engine::framebuffer_size_callback(GLFWwindow* windowInstance, int width, int height) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(windowInstance));
    if (engine) engine->on_framebuffer_size(windowInstance, width, height);
}
void Engine::on_framebuffer_size(GLFWwindow* windowInstance, int width, int height) {
    screen_width = width;
    screen_height = height;
    glViewport(0, 0, width, height);
    camera.updateProjectionMatrix(width, height);
}



void Engine::scroll_callback(GLFWwindow* windowInstance, double xOffset, double yOffset) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(windowInstance));
    if (engine) engine->on_scroll(windowInstance, xOffset, yOffset);
}
void Engine::on_scroll(GLFWwindow* windowInstance, double xOffset, double yOffset) {
    camera.adjustFOV(yOffset);
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
        camera.resetMouseState();
        return;
    }

    float xOffset = xpos - lastX;
    float yOffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    camera.rotateFromMouse(xOffset, yOffset);
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
        camera.resetMouseState();
        camera.setSprintMode(false);
    }
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        isMouseCaptured = true;
        firstMouse = true; // reset first mouse so that when the mouse is captured again it doesn't cause a sudden jump in camera direction
        camera.resetMouseState();
    }
}