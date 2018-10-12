#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 TexDir;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexDir = aPos;
    //将深度设置为最深，齐次后z为1.0
    gl_Position = (projection * view * vec4(aPos, 1.0)).xyww;
}