#version 330 core

in vs_fs_struct{
    vec2 texCoords;
}vs_fs;

out vec4 gl_Color;

uniform sampler2D depthMapTexture;

void main(){
    float depthValue = texture(depthMapTexture, vs_fs.texCoords).r;
    gl_Color = vec4(vec3(depthValue), 1.0);
}