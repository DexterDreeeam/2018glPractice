#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Position;
in vec3 Normal;

uniform sampler2D texture1;
uniform samplerCube skybox;
uniform vec3 cameraPos;

void main(){
    vec3 lightIn = normalize(Position - cameraPos);
    ///reflection
    vec3 lightReflectionOut = reflect(lightIn, normalize(Normal));
    ///refraction
    float ratio = 1.00 / 1.52;
    vec3 lightRefractionOut = refract(lightIn, normalize(Normal), ratio);
    //FragColor = texture(texture1, TexCoords);
    FragColor = vec4(texture(skybox, lightReflectionOut).rgb, 1.0);
    FragColor += vec4(texture(skybox, lightRefractionOut).rgb, 1.0);
    FragColor /= 1.5;
}