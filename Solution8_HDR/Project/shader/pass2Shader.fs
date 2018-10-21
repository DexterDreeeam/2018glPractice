#version 330 core

in vec2 texCoords;

out vec4 gl_Color;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure;

void main(){
    const float gamma = 2.2f;
    vec3 hdrColor = texture(hdrBuffer, texCoords).rgb;
    if(hdr){
        ///reinhard
        //vec3 result = hdrColor / (hdrColor + vec3(1.0f));
        ///exposure
        vec3 result = vec3(1.0f) - exp(- hdrColor * exposure);
        //gamma correct
        result = pow(result, vec3(1.0f / gamma));
        gl_Color = vec4(result, 1.0f);
    }
    else{
        vec3 result = pow(hdrColor, vec3(1.0f / gamma));
        gl_Color = vec4(result, 1.0f);
    }
}