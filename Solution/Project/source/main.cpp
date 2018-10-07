
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <../include/Shader.h>
#include <../include/Camera.h>
#include <../include/Model.h>

#include <iostream>
#include <string>
#include <stdio.h>

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);
void processInput(GLFWwindow * window);

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    ///glfw window creation
    GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Loading", NULL, NULL);
    if(!window){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    ///setCallback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    ///mode setting of mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    ///set glew
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        return -1;
    }
    ///configure global openGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    ///build & compile shaders
    Shader ourShader("shader/modelVS.vs", "shader/modelFS.fs");
    Shader ourBoaderShader("shader/modelVS.vs", "shader/modelSingleColorFS.fs");
    ///load model
    Model ourModel(std::string("nanosuit/nanosuit.obj"));
    ///draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    ///render loop
    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ///input
        processInput(window);
        ///render
        glClearColor(0.05f, 0.55f, 0.35f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        ///enable shader [befor setting uniforms]
        ourShader.use();
        struct{
            glm::vec3 direction;
            glm::vec3 color;
        }directionLight = { {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} };
        struct{
            glm::vec3 position;
            glm::vec3 color;
            float constant;
            float linear;
            float quadratic;
        }pointLight = { {2.0f, 2.0f, 2.0f}, {1.0f, 1.0f, 1.0f}, 1.0f, 0.09f, 0.032f };
        ourShader.setVec3("uniPointLight.position", pointLight.position);
        ourShader.setVec3("uniPointLight.color", pointLight.color);
        ourShader.setFloat("uniPointLight.constant", pointLight.constant);
        ourShader.setFloat("uniPointLight.linear", pointLight.linear);
        ourShader.setFloat("uniPointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("uniDirectionLight.direction", directionLight.direction);
        ourShader.setVec3("uniDirectionLight.color", directionLight.color);
        //glUniform1fv(glGetUniformLocation(ourShader.ID, "uniDirectionLight"), sizeof(directionLight) / 4, (float *)&directionLight);
        //glUniform1fv(glGetUniformLocation(ourShader.ID, "uniPointLight"), sizeof(pointLight) / 4, (float *)&pointLight);
        ourShader.setVec3("viewPos", camera.Position);
        ///view & projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ///render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        ourShader.setMat4("model", model);
        ///set stencil test
        glStencilFunc(GL_ALWAYS, 1, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xff);
        ourModel.Draw(ourShader);
        ///set stencil to draw edge
        ourBoaderShader.use();
        ourBoaderShader.setMat4("projection", projection);
        ourBoaderShader.setMat4("view", view);
        model = glm::scale(model, glm::vec3(1.003f, 1.003f, 1.003f));
        ourShader.setMat4("model", model);
        glStencilFunc(GL_NOTEQUAL, 1, 0xff);
        ourModel.Draw(ourShader);
        ///glfw swap buffers and poll io events
        glfwSwapBuffers(window);
        static double sumTime = 0.0;
        static int count = 0;
        sumTime += deltaTime;
        if (++count == 10) {
            glfwSetWindowTitle(window, ("FPS " + std::to_string(int(10.0 / (sumTime)))).c_str());
            sumTime = 0;
            count = 0;
        }
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow * window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow * window, int width, int height){
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow * window, double xpos, double ypos){
    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    xoffset *= 0.5f;
    float yoffset = lastY - ypos;
    yoffset *= 0.5f;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset){
    camera.ProcessMouseScroll(yoffset);
}