#version 330 core

in vec2 vs_fs_texCoords;

out vec4 output;

struct Light{
    vec3 position;
    vec3 color;
    float linear;
    float quadratic;
    ///radius
    float radius;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo_specular;

void main(){
    ///get data from G-buffer
    vec3 fragPos = texture(g_position, vs_fs_texCoords).rgb;
    vec3 normal = texture(g_normal, vs_fs_texCoords).rgb;
    vec3 diffuse = texture(g_albedo_specular, vs_fs_texCoords).rgb;
    float specular = texture(g_albedo_specular, vs_fs_texCoords).a;
    ///calculate lighting
    vec3 lighting = diffuse * 0.1f;
    vec3 viewDir = (viewPos - fragPos);
    for(int i = 0; i != NR_LIGHTS; ++i){
        float distance = length(lights[i].position - fragPos);
        if(distance < lights[i].radius){
            vec3 lightDir = normalize(lights[i].position - fragPos);
            vec3 diffuseColor = max(dot(normal, lightDir), 0.0f) * diffuse * lights[i].color;
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(normal, halfwayDir), 0.0f), 16.0f);
            vec3 specularColor = lights[i].color * spec * specular;
            float attenuation = 1.0f / (1.0f + lights[i].linear * distance + lights[i].quadratic * distance * distance);
            lighting += attenuation * (diffuseColor + specularColor);
        }
    }
    output = vec4(lighting, 1.0f);
}