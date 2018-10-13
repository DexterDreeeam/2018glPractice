#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
//layout (location = 3) in vec4 aInstancematrix1;
//layout (location = 4) in vec4 aInstancematrix2;
//layout (location = 5) in vec4 aInstancematrix3;
//layout (location = 6) in vec4 aInstancematrix4;
layout (location = 3) in mat4 aInstancematrix;

out vec2 TexCoords;

uniform mat4 revolveMatrix;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    //mat4 aInstancematrix = mat4(aInstancematrix1, aInstancematrix2, aInstancematrix3, aInstancematrix4);
    TexCoords = aTexCoords;
    gl_Position = projection * view * revolveMatrix * aInstancematrix * vec4(aPos, 1.0);
}