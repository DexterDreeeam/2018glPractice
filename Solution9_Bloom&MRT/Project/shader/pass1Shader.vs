#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_FS{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
}vs_fs;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    vs_fs.fragPos = vec3(model * vec4(aPos, 1.0f));
    vs_fs.normal = normalize(transpose(inverse(mat3(model))) * aNormal);
    vs_fs.texCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}