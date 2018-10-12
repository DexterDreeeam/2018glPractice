#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vs_gs_interface{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
}vs_gs[];

out gs_fs_interface{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
}gs_fs;

uniform float time;
uniform mat4 view;
uniform mat4 projection;

vec4 explode(vec4 position, vec3 normal){
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
    return position + vec4(direction, 0.0);
}

vec3 GetNormal(){

    vec3 a = vec3(vs_gs[0].FragPos) - vec3(vs_gs[1].FragPos);
    vec3 b = vec3(vs_gs[2].FragPos) - vec3(vs_gs[1].FragPos);
    return normalize(cross(a, b));

/*
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
*/
}


void main()
{

    vec3 normal = GetNormal();
    for(int i = 0; i != 3; ++i){
        gs_fs.FragPos = vec3(explode(vec4(vs_gs[i].FragPos, 1.0), normal));
        gs_fs.Normal = vs_gs[i].Normal;
        gs_fs.TexCoords = vs_gs[i].TexCoords;
        gl_Position = projection * view * vec4(gs_fs.FragPos, 1.0);
        EmitVertex();
    }

/*
    vec3 normal = GetNormal();
    for(int i = 0; i != 3; ++i){
        gs_fs.FragPos = vs_gs[i].FragPos;
        gs_fs.Normal = vs_gs[i].Normal;
        gs_fs.TexCoords = vs_gs[i].TexCoords;
        gl_Position = explode(gl_in[i].gl_Position, normal);
        EmitVertex();
    }
*/
    EndPrimitive();
}