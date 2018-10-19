#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

out vs_fs_struct{
    vec2 texCoords;
}vs_fs;

void main(){
    vs_fs.texCoords = texCoords;
    gl_Position = vec4(position, 1.0f, 1.0f);
}