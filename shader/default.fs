#version 330 core
out vec4 FragColor;

in vec3 worldPos;
in vec3 normal;

uniform vec4 albedo;

void main()
{
    FragColor = albedo;
}