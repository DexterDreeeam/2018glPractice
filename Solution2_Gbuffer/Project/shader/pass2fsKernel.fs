#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 500.0;
const vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), //leftUp
    vec2(   0.0f,  offset), //Up
    vec2( offset,  offset), //rightUp
    vec2(-offset,    0.0f), //left
    vec2(   0.0f,    0.0f), //mid
    vec2( offset,    0.0f), //right
    vec2(-offset, -offset), //leftDown
    vec2(   0.0f, -offset), //down
    vec2( offset, -offset)  //rightDown
);
///Sharpen Kernel
/*
const float kernel[9] = float[](
    -1, -1, -1,
    -1,  9, -1,
    -1, -1, -1
);
*/
///Blur Kernel
/*
const float kernel[9] = float[](
    1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
    2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
    1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
);
*/
///Edge-detection kernel
const float kernel[9] = float[](
    1,  1,  1,
    1, -8,  1,
    1,  1,  1
);

void main()
{
    vec3 sampleTex[9];
    for(int i = 0; i != 9; ++i){
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 sumColor = vec3(0.0);
    for(int i = 0; i != 9; ++i){
        sumColor += sampleTex[i] * kernel[i];
    }
    FragColor = vec4(sumColor, 1.0);
}