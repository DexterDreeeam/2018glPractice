
#include "Common.h"

void inputProcess(GLFWwindow * window);

int main() {

    GLFWwindow * window = initGLFW();
    if (!window || !initGLEW()) {
        glfwTerminate();
        return -1;
    }

    int success;
    char infoLog[512];
    
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        PRINT(infoLog);
        return -1;
    }

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        PRINT(infoLog);
        return -1;
    }

    unsigned int pro = glCreateProgram();
    glAttachShader(pro, vs);
    glAttachShader(pro, fs);
    glLinkProgram(pro);
    glGetProgramiv(pro, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(pro, 512, NULL, infoLog);
        PRINT(infoLog);
        return -1;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    const float vertices[] = {
        +0.5f, +0.5f, +0.0f,
        +0.5f, -0.5f, +0.0f,
        -0.5f, -0.5f, +0.0f,
        -0.5f, +0.5f, +0.0f
    };
    const unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }
    glBindVertexArray(0);
    
    while (!glfwWindowShouldClose(window)) {
        
        inputProcess(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(pro);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    return 0;
}

void inputProcess(GLFWwindow * window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}