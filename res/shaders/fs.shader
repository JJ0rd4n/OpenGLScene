#version 330 core

in vec4 vertexColor;

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor);
};