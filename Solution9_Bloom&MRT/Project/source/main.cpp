
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
unsigned int loadTexture(const char *path, bool gammaCorrection);
void renderQuad();
void renderCube();

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
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
    Shader pass1Shader_box("shader/pass1Shader.vs", "shader/pass1Shader_box.fs");
    Shader pass1Shader_light("shader/pass1Shader.vs", "shader/pass1Shader_light.fs");
    Shader pass2Shader_blur("shader/pass2Shader_blur.vs", "shader/pass2Shader_blur.fs");
    Shader pass3Shader_final("shader/pass3Shader_final.vs", "shader/pass3Shader_final.fs");
    Shader debugShader("shader/debugShader.vs", "shader/debugShader.fs");
    ///load texture
    unsigned int woodTexture = loadTexture("texture/wood.png", true);
    unsigned int containerTexture = loadTexture("texture/container.png", true);
    ///pass1 fbo
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        unsigned int colorBuffers[2];
        glGenTextures(2, colorBuffers);
        for (int i = 0; i != 2; ++i) {
            glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
        }
        unsigned int depthRBO;
        glGenRenderbuffers(1, &depthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
        ///MRT set
        unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);
        ///check fbo
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "pass1 fbo not complete." << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ///pass2 fbo
    unsigned int blurFBO[2];
    unsigned int blurColorBuffers[2];
    glGenFramebuffers(2, blurFBO);
    glGenTextures(2, blurColorBuffers);
    for (int i = 0; i != 2; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);
            glBindTexture(GL_TEXTURE_2D, blurColorBuffers[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColorBuffers[i], 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "pass2 fbo not complete." << std::endl;
    }
    ///lighting info
    std::vector<glm::vec3> lightPositions;
    lightPositions.push_back(glm::vec3(0.0f, 0.5f, 1.5f));
    lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
    lightPositions.push_back(glm::vec3(3.0f, 0.5f, 1.0f));
    lightPositions.push_back(glm::vec3(-0.8f, 2.4f, -1.0f));
    std::vector<glm::vec3> lightColors;
    lightColors.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
    lightColors.push_back(glm::vec3(1.5f, 0.0f, 0.0f));
    lightColors.push_back(glm::vec3(0.0f, 0.0f, 1.5f));
    lightColors.push_back(glm::vec3(0.0f, 1.5f, 0.0f));
    ///static uniforms
    pass1Shader_box.use();
        pass1Shader_box.setInt("diffuseTexture", 0);
    pass2Shader_blur.use();
        pass2Shader_blur.setInt("image", 0);
    pass3Shader_final.use();
        pass3Shader_final.setInt("scene", 0);
        pass3Shader_final.setInt("bloomBlur", 1);
    debugShader.use();
        debugShader.setInt("image", 0);
    ///draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    ///render loop
    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ///input
        processInput(window);
        ///pass1 render
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
        //glClearTexImage(GL_TEXTURE_2D, 0, GL_RGB16F, GL_FLOAT, NULL);
        glm::mat4 model(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        pass1Shader_box.use();
            pass1Shader_box.setMat4("view", view);
            pass1Shader_box.setMat4("projection", projection);
            pass1Shader_box.setVec3("viewPos", camera.Position);
            for (int i = 0; i != lightPositions.size(); ++i) {
                pass1Shader_box.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
                pass1Shader_box.setVec3("lights[" + std::to_string(i) + "].color", lightColors[i]);
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, woodTexture);
            ///box1
            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
            pass1Shader_box.setMat4("model", model);
            renderCube();
            ///box2
            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f));
            pass1Shader_box.setMat4("model", model);
            renderCube();
            ///box3
            model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.5f));
            pass1Shader_box.setMat4("model", model);
            renderCube();
            ///box4
            model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 2.0f));
            model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
            pass1Shader_box.setMat4("model", model);
            renderCube();
            ///box5
            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.7f, 4.0f));
            model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
            model = glm::scale(model, glm::vec3(1.25f));
            pass1Shader_box.setMat4("model", model);
            renderCube();
            ///box6
            model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.0f, -3.0f));
            model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
            pass1Shader_box.setMat4("model", model);
            renderCube();
            ///box7
            model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f));
            pass1Shader_box.setMat4("model", model);
            renderCube();
        pass1Shader_light.use();
            pass1Shader_light.setMat4("view", view);
            pass1Shader_light.setMat4("projection", projection);
            for (int i = 0; i != lightPositions.size(); ++i) {
                model = glm::translate(glm::mat4(1.0f), lightPositions[i]);
                model = glm::scale(model, glm::vec3(0.25f));
                pass1Shader_light.setMat4("model", model);
                pass1Shader_light.setVec3("lightColor", lightColors[i]);
                renderCube();
            }
        ///pass2 render [blur]
        bool horizontal = true, first_iteration = true;
        const int amount = 10;
        pass2Shader_blur.use();
            for (int i = 0; i != amount; ++i) {
                glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);
                pass2Shader_blur.setInt("horizontal", horizontal);
                glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : blurColorBuffers[!horizontal]);
                renderQuad();
                horizontal = !horizontal;
                if (first_iteration)
                    first_iteration = false;
            }
        ///pass3 render blend and hdr deal
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        pass3Shader_final.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, blurColorBuffers[!horizontal]);
            pass3Shader_final.setInt("bloom", bloom);
            pass3Shader_final.setFloat("exposure", exposure);
            renderQuad();
        ///debug render
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //debugShader.use();
        //    glActiveTexture(GL_TEXTURE0);
        //    glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
        //    renderQuad();
        ///print info
        std::cout << "bloom: " << (bloom ? "on" : "off") << " | " << "exposure: " << exposure << std::endl;
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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed){
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE){
        bloomKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        if (exposure > 0.0f)
            exposure -= 0.02f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
        exposure += 0.02f;
    }
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

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube(){
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad(){
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int loadTexture(char const * path, bool gammaCorrection){
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1){
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3){
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4){
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else{
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}