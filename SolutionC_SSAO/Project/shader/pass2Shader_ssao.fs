#version 330 core

in vec2 vs_fs_texCoords;

out float output;

uniform mat4 projection;
uniform vec3 samples[64];
uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D tex_noise;

const vec2 noiseScale = vec2(1600.0f / 4.0f, 900.0f / 4.0f);
int kernelSize = 64;
float radius = 0.5f;
float bias = 0.025f;

void main(){
    vec3 fragPos = texture(g_position, vs_fs_texCoords).xyz;
    vec3 normal = normalize(texture(g_normal, vs_fs_texCoords).rgb);
    vec3 randomVec = normalize(texture(tex_noise, vs_fs_texCoords * noiseScale).xyz);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    float occlusion = 0.0f;
    for(int i = 0; i != kernelSize; ++i){
        ///tangent to view
        vec3 sample = TBN * samples[i];
        sample = fragPos + sample * radius;
        vec4 offset = vec4(sample, 1.0f);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        float sampleDepth = texture(g_position, offset.xy).z;
        float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + bias ? 1.0f : 0.0f) * rangeCheck;
    }
    occlusion = 1.0f - (occlusion / kernelSize);
    output = occlusion;
}