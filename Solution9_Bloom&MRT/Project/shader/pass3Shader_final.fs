#version 330 core

in vec2 vs_fs_texCoords;

out vec4 output;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform float exposure;

void main(){
    const float gamma = 2.2f;
    vec3 hdrColor = texture(scene, vs_fs_texCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, vs_fs_texCoords).rgb;
    if(bloom)
        hdrColor += bloomColor;
    vec3 result = vec3(1.0f) - exp(- hdrColor * exposure);
    result = pow(result, vec3(1.0f / gamma));
    output = vec4(result, 1.0f);
}