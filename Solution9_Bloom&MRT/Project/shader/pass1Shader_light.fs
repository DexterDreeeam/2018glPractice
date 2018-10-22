#version 330 core

layout (location = 0) out vec4 output1;
layout (location = 1) out vec4 output2;

in VS_FS{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
}vs_fs;

uniform vec3 lightColor;

void main(){
    output1 = vec4(lightColor, 1.0f);
    float brightness = dot(lightColor, vec3(0.2126f, 0.7152f, 0.0722f));
    //if(brightness > 1.0f)
        output2 = vec4(lightColor, 1.0f);
}