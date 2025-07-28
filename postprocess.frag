#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture;

uniform bool enableLensing;
uniform bool enableBloom;
uniform vec2 sphereScreenPos; // Posição da esfera na tela (0.0 a 1.0)
uniform float lensingStrength = 0.05;
uniform float bloomStrength = 0.04;

void main()
{
    vec2 currentTexCoords = TexCoords;
    if (enableLensing)
    {
        float dist = distance(TexCoords, sphereScreenPos);
        if (dist > 0.0) // Evita divisão por zero
        {
            vec2 pullDirection = normalize(sphereScreenPos - TexCoords);
            float pullAmount = 1.0 / (dist * 200.0 + 1.0);
            currentTexCoords -= pullDirection * pullAmount * lensingStrength;
        }
    }

    vec4 sceneColor = texture(sceneTexture, currentTexCoords);
    if (enableBloom)
    {
        vec4 bloomSum = vec4(0.0);
        float blurKernel[9] = float[](1.0/16.0, 2.0/16.0, 1.0/16.0,
                                      2.0/16.0, 4.0/16.0, 2.0/16.0,
                                      1.0/16.0, 2.0/16.0, 1.0/16.0);
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                vec2 offset = vec2(float(i), float(j)) / textureSize(sceneTexture, 0);
                bloomSum += texture(sceneTexture, currentTexCoords + offset) * blurKernel[(i+1)*3 + (j+1)];
            }
        }
        sceneColor += bloomSum * bloomStrength;
    }
    
    FragColor = sceneColor;
}