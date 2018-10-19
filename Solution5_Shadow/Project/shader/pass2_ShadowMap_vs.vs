#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texCoords;

out vs_fs_struct{
    vec2 texCoords;
    vec4 pixelPositionOfLightSpace;
}vs_fs;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main(){
    vs_fs.texCoords = texCoords;
    vs_fs.pixelPositionOfLightSpace = lightSpaceMatirx * model * vec4(position, 1.0f);
    gl_Position = projection * view * model * vec4(position, 1.0f);
}