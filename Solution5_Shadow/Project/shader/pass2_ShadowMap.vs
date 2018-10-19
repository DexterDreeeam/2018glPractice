#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vs_fs_struct{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
    vec4 positionOfLightSpace;
}vs_fs;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main(){
    vs_fs.position = vec3(model * vec4(position, 1.0f));
    vs_fs.normal = transpose(inverse(mat3(model))) * normal;
    vs_fs.texCoords = texCoords;
    vs_fs.positionOfLightSpace = lightSpaceMatrix * model * vec4(position, 1.0f);
    gl_Position = projection * view * model * vec4(position, 1.0f);
}