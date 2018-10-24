#version 330 core

in vec2 vs_fs_texCoords;

out vec4 output;

struct Light{
    vec3 position;
    vec3 color;
    float linear;
    float quadratic;
    ///radius
    //float radius;
};

uniform Light light;
uniform vec3 viewPos;
uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo_specular;
uniform sampler2D g_blur;

void main(){
    ///get data from G-buffer
    vec3 fragPos = texture(g_position, vs_fs_texCoords).rgb;
    vec3 normal = texture(g_normal, vs_fs_texCoords).rgb;
    vec3 diffuse = texture(g_albedo_specular, vs_fs_texCoords).rgb;
    float ambient_occlusion = texture(g_blur, vs_fs_texCoords).r;
    ///ambient
    vec3 lighting = diffuse * 0.3f * ambient_occlusion;
    ///diffuse
    vec3 viewDir = normalize(- fragPos); ///viewPos is (0, 0, 0)
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 diffuseColor = max(dot(normal, lightDir), 0.0f) * diffuse * light.color;
    ///specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), 8.0f);
    vec3 specularColor = light.color * spec;
    ///final
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (1.0f + light.linear * distance + light.quadratic * distance * distance);
    lighting += attenuation * (diffuseColor + specularColor);
    output = vec4(lighting, 1.0f);
}