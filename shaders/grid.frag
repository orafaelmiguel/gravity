// shaders/grid.frag
#version 330 core
out vec4 FragColor;

void main()
{
    vec4 emissiveColor = vec4(0.3, 0.7, 1.0, 1.0) * 4.0; 
    FragColor = emissiveColor;
}