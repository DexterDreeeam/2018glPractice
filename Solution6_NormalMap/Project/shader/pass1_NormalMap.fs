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
uniform bool normalMapping;

void main(){
    ///sample Normal Map in range [0, 1]
    vec3 normal = texture(normalMap, vs_fs.texCoords).rgb;
    ///transform the normal vector to range [-1, 1] (tangent space)
    normal = normalize(normal * 2.0f - 1.0f);
    ///sample color
    vec3 color = texture(diffuseMap, vs_fs.texCoords).rgb;
    ///ambient
    vec3 ambient = 0.1f * color;
    ///diffuse color
    vec3 lightDir = normalize(vs_fs.tangentLightPos - vs_fs.tangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0f);
    vec3 diffuse = diff * color;
    ///specular
    vec3 viewDir = normalize(vs_fs.tangentViewPos - vs_fs.tangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f);
    vec3 specular = vec3(0.2f) * spec;
    ///out
    gl_Color = vec4(ambient + diffuse + specular, 1.0f);
}