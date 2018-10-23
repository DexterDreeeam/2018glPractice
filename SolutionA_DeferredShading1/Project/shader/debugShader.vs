#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 vs_fs_texCoords;

void main()
{
    vs_fs_texCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0f);
}