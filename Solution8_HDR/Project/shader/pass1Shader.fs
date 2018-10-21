#version 330 core

in VS_FS{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
}vs_fs;

out vec4 gl_Color;

struct Light{
    vec3 position;
    vec3 color;
};

uniform Light lights[16];
uniform sampler2D diffuseTexture;
uniform vec3 viewPos;

void main(){
    vec3 color = texture(diffuseTexture, vs_fs.texCoords).rgb;
    vec3 normal = normalize(vs_fs.normal);
    //ambient
    vec3 ambient = 0.01f * color;
    //lighting
    vec3 lighting = vec3(0.0f);
    for(int i = 0; i != 16; ++i){
        //diffuse
        vec3 lightDir = normalize(lights[i].position - vs_fs.fragPos);
        float diff = max(dot(lightDir, normal), 0.0f);
        vec3 diffuse = lights[i].color * diff * color;
        vec3 result = diffuse;
        //attenuation
        float distance = length(vs_fs.fragPos - lights[i].position);
        result *= 1.0f / (distance * distance);
        lighting += result;
    }
    gl_Color = vec4(ambient + lighting, 1.0f);
}