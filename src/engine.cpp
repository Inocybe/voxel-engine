#include <engine.hpp>

Engine::Engine(unsigned int screenWidth, unsigned int screenHeight, const char* windowName)
    : screen_width(screenWidth),
      screen_height(screenHeight),
      camera((float)screenWidth, (float)screenHeight)   // Camera gets proper dimensions here
{
    glfwSetErrorCallback(Engine::error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(screenWidth, screenHeight, windowName, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glfwSetFramebufferSizeCallback(window, Engine::framebuffer_size_callback);
    glfwSetCursorPosCallback(window, Engine::mouse_callback);
    glfwSetScrollCallback(window, Engine::scroll_callback);

    // Sync projection to actual framebuffer size (may differ from window size on HiDPI)
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

    glfwPollEvents();

    calculate_delta();
    process_input();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    inputManager.update(window);
    return true;
}

void Engine::EndFrame() {
    glfwSwapBuffers(window);
}

glm::mat4 Engine::GetViewMatrix() {
    return camera.getViewMatrix();
}

glm::mat4 Engine::GetProjectionMatrix() {
    return camera.getProjectionMatrix();
}

glm::vec3& Engine::getCameraPosLocation() {
    return camera.getPosRef();
}

glm::vec3 Engine::getCameraPos() {
    return camera.getPos();
}

// ── Input ─────────────────────────────────────────────────────────────────────

void Engine::process_input() {
    // Mouse capture toggle
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        camera.releaseMouse();
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        camera.captureMouse();
    }

    camera.processMovement(window, deltaTime);
}

void Engine::calculate_delta() {
    float currentFrame = (float)glfwGetTime();
    deltaTime  = currentFrame - lastFrame;
    lastFrame  = currentFrame;
}

// ── GLFW static → instance dispatch ──────────────────────────────────────────

void Engine::error_callback(int /*id*/, const char* descriptor) {
    std::cout << descriptor << std::endl;
}

void Engine::framebuffer_size_callback(GLFWwindow* w, int width, int height) {
    auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(w));
    if (engine) engine->on_framebuffer_size(w, width, height);
}
void Engine::on_framebuffer_size(GLFWwindow* /*w*/, int width, int height) {
    glViewport(0, 0, width, height);
    camera.onFramebufferResize(width, height);
}

void Engine::scroll_callback(GLFWwindow* w, double xOffset, double yOffset) {
    auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(w));
    if (engine) engine->on_scroll(w, xOffset, yOffset);
}
void Engine::on_scroll(GLFWwindow* /*w*/, double xOffset, double yOffset) {
    camera.onScroll(xOffset, yOffset);
}

void Engine::mouse_callback(GLFWwindow* w, double xpos, double ypos) {
    auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(w));
    if (engine) engine->on_mouse_move(w, xpos, ypos);
}
void Engine::on_mouse_move(GLFWwindow* /*w*/, double xpos, double ypos) {
    camera.onMouseMove(xpos, ypos);
}