#version 330 core
layout (location = 2) in vec2 texCoords;

in vs_fs_struct{
    vec2 texCoords;
    vec4 pixelPositionOfLightSpace;
}vs_fs;

out vec4 gl_Color;

uniform sampler2D diffuse1;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 _pixelPositionOfLightSpace){
    vec3 projCoords = _pixelPositionOfLightSpace.xyz / _pixelPositionOfLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float shadow = currentDepth > closestDepth ? 1.0f : 0.0f;
    return shadow;
}

void main(){
    gl_Color = vec4(texture(diffuse1, texCoords), 1.0f);
    gl_Color *= (1.0f - 0.5f * ShadowCalculation(vs_fs.pixelPositionOfLightSpace));
}