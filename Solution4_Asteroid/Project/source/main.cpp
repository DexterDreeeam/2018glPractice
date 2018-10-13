
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

Camera camera(glm::vec3(0.0f, 0.0f, 155.0f));
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
    ///build & compile shaders
    Shader asteroidShader("shader/asteroids.vs", "shader/asteroids.fs");
    Shader planetShader("shader/planet.vs", "shader/planet.fs");
    ///load model
    Model asteroidModel(std::string("rock/rock.obj"));
    Model planetModel(std::string("planet/planet.obj"));
    ///draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    ///set random model matrices
    unsigned int amount = 10000;
    glm::mat4 * modelMatrices;
    modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime());
    float radius = 150.0f;
    float offset = 25.0f;
    for(int i = 0; i != amount; ++i){
        glm::mat4 model = glm::mat4(1.0f);
        ///translation: displace along circle with 'radius' in range [-offset, offset]
        #define getDisplacement ((rand() % (int)(2 * offset * 100)) / 100.0f - offset) 
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = getDisplacement;
        float x = sin(angle) * radius + displacement;
        displacement = getDisplacement;
        float y = displacement * 0.4f;
        displacement = getDisplacement;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));
        ///scale: Scale between 0.05 to 0.25f
        float scale = (rand() % 20) / 100.0f + 0.05f;
        model = glm::scale(model, glm::vec3(scale));
        ///rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
        ///add to matrices
        modelMatrices[i] = model;
    }
    ///configure instances array
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
    ///set transformation matrices as an instance vertex attribute(with divisor 1)
    for(int i = 0; i != asteroidModel.meshes.size(); ++i){
        glBindVertexArray(asteroidModel.meshes[i].VAO);
        ///set aIntancesMatrix -> mat4
        ///set attribute 3
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glVertexAttribDivisor(3, 1);
        ///set attribute 4
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glVertexAttribDivisor(4, 1);
        ///set attribute 5
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * 2));
        glVertexAttribDivisor(5, 1);
        ///set attribute 6
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * 3));
        glVertexAttribDivisor(6, 1);
        ///unbind
        glBindVertexArray(0);
    }
    ///render loop
    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ///input
        processInput(window);
        ///render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ///configure transformation matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 revolveMatrix(1.0f);
        revolveMatrix = glm::rotate(revolveMatrix, (float)glm::radians(glfwGetTime() / 2.0), glm::vec3(0.0f, 1.0f, 0.0f));
        asteroidShader.use();
        asteroidShader.setMat4("revolveMatrix", revolveMatrix);
        asteroidShader.setMat4("projection", projection);
        asteroidShader.setMat4("view", view);
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        ///draw planet
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        planetShader.setMat4("model", model);
        planetModel.Draw(planetShader);
        ///draw asteroids
        asteroidShader.use();
        asteroidShader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, asteroidModel.textures_loaded[0].id);
        for(int i = 0; i != asteroidModel.meshes.size(); ++i){
            glBindVertexArray(asteroidModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, asteroidModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
            glBindVertexArray(0);
        }
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