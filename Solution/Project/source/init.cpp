
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define GLEW_STATIC

const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 900;

void callBackOfFrameBufferResize(GLFWwindow *, int, int);

GLFWwindow * initGLFW() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLTRUE);
#endif

    GLFWwindow * window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "xPractice", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, callBackOfFrameBufferResize);

    return window;

}

bool initGLEW() {

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        return false;
    }

    return true;

}

void callBackOfFrameBufferResize(GLFWwindow * window, int width, int height) {

    glViewport(0, 0, width, height);

}