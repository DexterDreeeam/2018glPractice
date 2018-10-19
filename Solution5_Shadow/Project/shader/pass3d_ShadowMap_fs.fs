#version 330 core

out vs_fs_struct{
    vec2 position;
    vec4 texCoords;
}vs_fs;

out vec4 gl_Color;

uniform sampler2D depthMapTexture;

void main(){
    gl_Color = vec4(texture(depthMapTexture, vs_fs.texCoords), 1.0f);
}