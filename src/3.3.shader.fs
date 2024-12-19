#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D texture1;
uniform vec4 overrideColor;
uniform bool useTexture;

void main()
{
    if(useTexture){
        FragColor = texture(texture1, texCoord);
    } else {
        FragColor = overrideColor;
    }
}