#version 450 core
out vec4 FragColor;

in vec3 worldPosition;

out vec4 color;

void main(){ color.rgb = worldPosition; }