#version 330 core

layout (location = 0) out vec4 output1;
layout (location = 1) out vec4 output2;

in VS_FS{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
}vs_fs;

struct Light{
    vec3 position;
    vec3 color;
};

uniform Light lights[4];
uniform sampler2D diffuseTexture;
uniform vec3 viewPos;

void main(){
    vec3 color = texture(diffuseTexture, vs_fs.texCoords).rgb;
    vec3 normal = normalize(vs_fs.normal);
    ///ambient
    vec3 ambient = 0.05f * color;
    ///lighting
    vec3 lighting = vec3(0.0f);
    vec3 viewDir = normalize(viewPos - vs_fs.fragPos);
    for(int i = 0; i != 4; ++i){
        ///diffuse
        vec3 lightDir = normalize(lights[i].position - vs_fs.fragPos);
        float diff = max(dot(lightDir, normal), 0.0f);
        vec3 result = lights[i].color * diff * color;
        ///attenuation
        float distance = length(vs_fs.fragPos - lights[i].position);
        result *= 1.0f / (distance * distance);
        lighting += result;
    }
    vec3 result = ambient + lighting;
    ///check whether need bloom(higher than threshold)
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    output1 = vec4(result, 1.0f);
    if(brightness > 1.0f)
        output2 = vec4(result, 1.0f);
    else
        output2 = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}