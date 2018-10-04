#pragma once


#define PRINT(x) (std::cout << (x) << std::endl)

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLFWwindow * initGLFW();
bool initGLEW();

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>


const char * vsSrc =
"#version 330 core\n"
"layout (location = 0) in vec3 pos;"
"void main()"
"{"
"    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);"
"}\0";

const char * fsSrc =
"#version 330 core\n"
"out vec4 FragColor;"
"void main()"
"{"
"    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
"}\0";