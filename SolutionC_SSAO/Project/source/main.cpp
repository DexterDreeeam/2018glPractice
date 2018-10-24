
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
#include <random>

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);
void processInput(GLFWwindow * window);
unsigned int loadTexture(const char *path, bool gammaCorrection);
void renderQuad();
void renderCube();

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lerp(float a, float b, float f){
    return a + f * (b - a);
}

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
    Shader pass1Shader_gbuffer("shader/pass1Shader_gbuffer.vs", "shader/pass1Shader_gbuffer.fs");
    Shader pass2Shader_ssao("shader/pass234Shader.vs", "shader/pass2Shader_ssao.fs");
    Shader pass3Shader_blur("shader/pass234Shader.vs", "shader/pass3Shader_blur.fs");
    Shader pass4Shader_lighting("shader/pass234Shader.vs", "shader/pass4Shader_lighting.fs");
    Shader debugShader("shader/debugShader.vs", "shader/debugShader.fs");
    ///load model
    Model nanosuit("nanosuit/nanosuit.obj");
    ///pass1 g-buffer fbo
    unsigned int g_buffer_fbo;
    glGenFramebuffers(1, &g_buffer_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_fbo);
        ///mrt colors
        unsigned int g_position_buffer, g_normal_buffer, g_albedo_spec_buffer;
        glGenTextures(1, &g_position_buffer);
        glBindTexture(GL_TEXTURE_2D, g_position_buffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position_buffer, 0);
        glGenTextures(1, &g_normal_buffer);
        glBindTexture(GL_TEXTURE_2D, g_normal_buffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal_buffer, 0);
        glGenTextures(1, &g_albedo_spec_buffer);
        glBindTexture(GL_TEXTURE_2D, g_albedo_spec_buffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_albedo_spec_buffer, 0);
        unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);
        ///depth rbo
        unsigned int depth_rbo;
        glGenRenderbuffers(1, &depth_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);
        ///check fbo
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "gbuffer framebuffer not complete." << std::endl;
    ///pass2 ssao
    unsigned int ssao_fbo;
    glGenFramebuffers(1, &ssao_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
        unsigned int ssao_color_buffer;
        glGenTextures(1, &ssao_color_buffer);
        glBindTexture(GL_TEXTURE_2D, ssao_color_buffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_color_buffer, 0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ssao framebuffer not complete." << std::endl;
    ///pass3 blur
    unsigned int blur_fbo;
    glGenFramebuffers(1, &blur_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
        unsigned int blur_color_buffer;
        glGenTextures(1, &blur_color_buffer);
        glBindTexture(GL_TEXTURE_2D, blur_color_buffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_color_buffer, 0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "blur framebuffer not complete." << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ///generate sample kernel
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssao_kernel;
    for(int i = 0; i != 64; ++i){
        glm::vec3 sample(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator)
            );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssao_kernel.push_back(sample);
    }
    ///generate noise texture
    std::vector<glm::vec3> ssao_noise;
    for(int i = 0; i != 16; ++i){
        glm::vec3 noise(
            randomFloats(generator) * 2.0f - 1.0f, 
            randomFloats(generator) * 2.0f - 1.0f,
            0.0f
            );
        ssao_noise.push_back(noise);
    }
    unsigned int noise_texture;
    glGenTextures(1, &noise_texture);
    glBindTexture(GL_TEXTURE_2D, noise_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssao_noise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    ///light
    glm::vec3 light_position(0.0f, 5.0f, 0.0f);
    //glm::vec3 light_position(2.0f, 4.0f, -2.0f);
    glm::vec3 light_color(0.2f, 0.2f, 0.7f);
    ///static uniforms
    pass2Shader_ssao.use();
        pass2Shader_ssao.setInt("g_position", 0);
        pass2Shader_ssao.setInt("g_normal", 1);
        pass2Shader_ssao.setInt("tex_noise", 2);
        for (int i = 0; i != 64; ++i) {
            pass2Shader_ssao.setVec3("samples[" + std::to_string(i) + "]", ssao_kernel[i]);
        }
    pass3Shader_blur.use();
        pass3Shader_blur.setInt("g_ssao", 0);
    pass4Shader_lighting.use();
        pass4Shader_lighting.setInt("g_position", 0);
        pass4Shader_lighting.setInt("g_normal", 1);
        pass4Shader_lighting.setInt("g_albedo_specular", 2);
        pass4Shader_lighting.setInt("g_blur", 3);
    ///draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    ///close vertical-sync
    //glfwSwapInterval(0);
    ///render loop
    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ///input
        processInput(window);
        ///clear color set
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        ///pass1 g_buffer render
        glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_fbo);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 model(1.0f);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            pass1Shader_gbuffer.use();
                pass1Shader_gbuffer.setMat4("view", view);
                pass1Shader_gbuffer.setMat4("projection", projection);
                ///room
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 7.0f, 0.0f));
                model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));
                pass1Shader_gbuffer.setMat4("model", model);
                pass1Shader_gbuffer.setInt("invertedNormal", 1);///invert normal as inside cube
                renderCube();
                ///nanosuit model
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::scale(model, glm::vec3(0.5f));
                pass1Shader_gbuffer.setMat4("model", model);
                pass1Shader_gbuffer.setInt("invertedNormal", 0);///recover normal
                nanosuit.Draw(pass1Shader_gbuffer);
        ///pass2 ssao render
        glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
            glClear(GL_COLOR_BUFFER_BIT);
            pass2Shader_ssao.use();
                pass2Shader_ssao.setMat4("projection", projection);
                //for(int i = 0; i != 64; ++i){
                //    pass2Shader_ssao.setVec3("samples[" + std::to_string(i) + "]", ssao_kernel[i]);
                //}
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, g_position_buffer);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, g_normal_buffer);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, noise_texture);
                renderQuad();
        ///pass3 blur render
        glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
            glClear(GL_COLOR_BUFFER_BIT);
            pass3Shader_blur.use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, ssao_color_buffer);
                renderQuad();
        ///pass4 lighting render
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            pass4Shader_lighting.use();
                glm::vec3 light_position_view_space = glm::vec3(view * glm::vec4(light_position, 1.0f));
                pass4Shader_lighting.setVec3("light.position", light_position_view_space);
                pass4Shader_lighting.setVec3("light.color", light_color);
                const float constant = 1.0f;
                const float linear = 0.09f;
                const float quadratic = 0.032f;
                pass4Shader_lighting.setFloat("light.linear", linear);
                pass4Shader_lighting.setFloat("light.quadratic", quadratic);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, g_position_buffer);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, g_normal_buffer);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, g_albedo_spec_buffer);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, blur_color_buffer);
                renderQuad();
        ///debug render
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //debugShader.use();
        //    glActiveTexture(GL_TEXTURE0);
        //    glBindTexture(GL_TEXTURE_2D, g_position_buffer);
        //    renderQuad();
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