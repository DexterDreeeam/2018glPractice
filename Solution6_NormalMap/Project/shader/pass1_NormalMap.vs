#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_FS{
    vec3 fragPos;
    vec2 texCoords;
    vec3 tangentLightPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
}vs_fs;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main(){
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vs_fs.fragPos = vec3(model * vec4(position, 1.0f));
    vs_fs.texCoords = texCoords;
    ///build TBN coords
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 B = normalize(normalMatrix * bitangent);
    vec3 N = normalize(normalMatrix * normal);
    ///transform light/view/fragPos to TBN
    mat3 transformTBN = transpose(mat3(T, B, N));
    vs_fs.tangentLightPos = transformTBN * lightPos;
    vs_fs.tangentViewPos = transformTBN * viewPos;
    vs_fs.tangentFragPos = transformTBN * vs_fs.fragPos;
}