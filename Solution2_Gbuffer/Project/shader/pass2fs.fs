#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    ///normal - 常规模式
    FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);
    
    ///inversion - 反相模式
    //FragColor = vec4(1.0 - texture(screenTexture, TexCoords).rgb, 1.0);

    ///grayscale - 灰度模式
    //FragColor = texture(screenTexture, TexCoords);
    //float average = (0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b) / 3.0;
    //FragColor = vec4(average, average, average, 1.0);
}