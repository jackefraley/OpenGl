#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform vec3 displacement;

out vec3 color;
out vec2 texCoord;

void main()
{
    gl_Position = vec4(aPos + displacement, 1.0);
    color = aColor;
    //position = aPos + displacement;
    texCoord = vec2(aTexCoord.x, aTexCoord.y);
}