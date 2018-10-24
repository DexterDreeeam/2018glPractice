#version 330 core

layout (location = 0) out vec3 output1; //position
layout (location = 1) out vec3 output2; //normal
layout (location = 2) out vec4 output3; //albedo_specular

in VS_FS{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
}vs_fs;

//uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_specular1;

void main(){
    output1 = vs_fs.fragPos;
    output2 = normalize(vs_fs.normal);
    output3.rgb = vec3(0.95f);
    output3.a = 1.0f;
    //output3.rgb = texture(texture_diffuse1, vs_fs.texCoords).rgb;
    //output3.a = texture(texture_specular1, vs_fs.texCoords).r;
}