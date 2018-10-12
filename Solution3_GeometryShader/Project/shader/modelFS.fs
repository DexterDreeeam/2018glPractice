#version 330 core

struct directionLight{
    vec3 direction;
    vec3 color;
};

struct pointLight{
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

in gs_fs_interface{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
}gs_fs;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform directionLight uniDirectionLight;
uniform pointLight uniPointLight;
uniform vec3 viewPos;

void main()
{
    FragColor = texture(texture_diffuse1, gs_fs.TexCoords);
/*
    vec3 normalDir = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 pointLightDir = normalize(uniPointLight.position - FragPos);
    vec3 directionLightDir = normalize(-uniDirectionLight.direction);
    ///calculate pointLight
    float pointLightDistance = length(uniPointLight.position - FragPos);
    float attenuation = 1.0 / (uniPointLight.constant + uniPointLight.linear * pointLightDistance + uniPointLight.quadratic * pointLightDistance * pointLightDistance);
    vec3 halfDir = normalize(pointLightDir + viewDir);
    float pointLightDiffuse = attenuation * max(dot(normalDir, pointLightDir), 0.0);
    float pointLightSpecular = pow(max(dot(halfDir, normalDir), 0.0), 32);
    ///calculate directionLight
    halfDir = normalize(directionLightDir + viewDir);
    float directionLightDiffuse = max(dot(normalDir, directionLightDir), 0.0);
    float directionLightSpecular = pow(max(dot(normalDir, halfDir), 0.0), 32);
    ///calculate color
    vec4 objRawColor = texture(texture_diffuse1, TexCoords);
    FragColor = objRawColor * (0.05 + pointLightDiffuse + directionLightDiffuse) + 
        (objRawColor + 0.2 * vec4(1.0, 1.0, 1.0, 1.0)) * (0.6 * (pointLightSpecular + directionLightSpecular));
*/
}