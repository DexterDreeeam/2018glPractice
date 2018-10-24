#version 330 core

in vec2 vs_fs_texCoords;

out float output;

uniform sampler2D g_ssao;

void main(){
    vec2 texelSize = 1.0f / vec2(textureSize(g_ssao, 0));
    float result = 0.0f;
    for(int x = -2; x != 2; ++x){
        for(int y = -2; y != 2; ++y){
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(g_ssao, vs_fs_texCoords + offset).r;
        }
    }
    output = result / (4.0f * 4.0f);
}