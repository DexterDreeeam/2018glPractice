#version 330 core

in vec2 vs_fs_texCoords;

out vec4 output;

uniform sampler2D image;

void main(){
    //output = vec4(texture(image, vs_fs_texCoords).rrr, 1.0f);
    output = vec4(texture(image, vs_fs_texCoords).rbg, 1.0f);
}