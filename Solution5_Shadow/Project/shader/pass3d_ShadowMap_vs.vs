#version 330 core
layout (location = 0) in vec2 position;
layout (location = 2) in vec2 texCoords;

out vs_fs_struct{
    vec2 position;
    vec4 texCoords;
}vs_fs;

void main(){
    vs_fs.position = position;
    vs_fs.texCoords = texCoords;
    gl_Position = (position, 1.0f, 1.0f);
}