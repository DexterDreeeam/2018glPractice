#version 330 core

out vec4 output;

uniform vec3 lightColor;

void main(){
    output = vec4(lightColor, 1.0f);
}