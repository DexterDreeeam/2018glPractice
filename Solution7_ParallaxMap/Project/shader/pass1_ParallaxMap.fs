#version 330 core

in VS_FS{
    vec3 fragPos;
    vec2 texCoords;
    vec3 tangentLightPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
}vs_fs;

out vec4 gl_Color;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform bool parallax;
uniform float height_scale;

/*
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
    float height = texture(depthMap, texCoords).r;
    return texCoords - viewDir.xy / viewDir.z * (height * height_scale);
}
*/

/*
///steep parallax mapping
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0f, 0.0f, 1.0f), viewDir)));
    float layerDepth = 1.0f / numLayers;
    float currentLayerDepth = 0.0f;
    vec2 P = viewDir.xy / viewDir.z * height_scale;
    vec2 deltaTexCoords = P / numLayers;
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
    while(currentLayerDepth < currentDepthMapValue){
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }
    return currentTexCoords;
}
*/

///parallax occlusion mapping
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0f, 0.0f, 1.0f), viewDir)));
    float layerDepth = 1.0f / numLayers;
    float currentLayerDepth = 0.0f;
    vec2 P = viewDir.xy / viewDir.z * height_scale;
    vec2 deltaTexCoords = P / numLayers;
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
    while(currentLayerDepth < currentDepthMapValue){
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0f - weight);
    return finalTexCoords;
}

void main(){
    ///offset texture coords
    vec3 viewDir = normalize(vs_fs.tangentViewPos - vs_fs.tangentFragPos);
    vec2 texCoords = vs_fs.texCoords;
    if(parallax){
        texCoords = ParallaxMapping(vs_fs.texCoords, viewDir);
        if(texCoords.x < 0.0f || texCoords.x > 1.0f || texCoords.y < 0.0f || texCoords.y > 1.0f)
            discard;
    }
    ///obtain normal
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0f - 1.0f);
    ///diffuse color
    vec3 color = texture(diffuseMap, texCoords).rgb;
    ///ambient
    vec3 ambient = 0.1f * color;
    ///diffuse
    vec3 lightDir = normalize(vs_fs.tangentLightPos - vs_fs.tangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0f);
    vec3 diffuse = diff * color;
    ///specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f);
    vec3 specular = vec3(0.2f) * spec;
    ///out
    gl_Color = vec4(ambient + diffuse + specular, 1.0f);
}