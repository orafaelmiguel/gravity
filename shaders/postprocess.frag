// shaders/postprocess.frag
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture; 

uniform bool enableBloom;
uniform float exposure = 1.0;

void main()
{
    vec3 hdrColor = texture(sceneTexture, TexCoords).rgb;
    
    vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;
    
    if(enableBloom)
    {
        hdrColor += bloomColor;
    }
    
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}