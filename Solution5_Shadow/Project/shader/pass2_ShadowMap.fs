#version 330 core

in vs_fs_struct{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
    vec4 positionOfLightSpace;
}vs_fs;

out vec4 gl_Color;

uniform sampler2D diffuse1;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

///设置深度偏移，但会引入peter panning现象
const float bias = 0.005;

float ShadowCalculation(vec4 _pixelPositionOfLightSpace){
    vec3 projCoords = _pixelPositionOfLightSpace.xyz / _pixelPositionOfLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    ///处理shadowMapTexture没有覆盖到的部分
    if(projCoords.z > 1.0f)
        return 0.0f;
    ///without PCF
    /*
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    */
    ///without PCF end
    ///PCF(percentage-closer filtering)
    float shadow = 0.0f;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float currentDepth = projCoords.z;
    for(int x = -1; x != 2; ++x){
        for(int y = -1; y != 2; ++y){
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0f;
    ///PCF end
    return shadow;
}

void main(){
    vec3 color = texture(diffuse1, vs_fs.texCoords).rgb;
    vec3 normal = normalize(vs_fs.normal);
    vec3 lightColor = vec3(1.0f);
    //ambient
    vec3 ambient = 0.4f * color;
    //diffuse
    vec3 lightDir = normalize(lightPos - vs_fs.position);
    float diff = max(dot(lightDir, normal), 0.0f);
    vec3 diffuse = diff * lightColor;
    //specular
    vec3 viewDir = normalize(viewPos - vs_fs.position);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), 64.0f);
    vec3 specular = spec * lightColor;
    //shadow calculation
    float shadow = ShadowCalculation(vs_fs.positionOfLightSpace);
    vec3 lighting = (ambient + (1.0f - shadow) * (diffuse + specular)) * color;
    gl_Color = vec4(lighting, 1.0f);

}