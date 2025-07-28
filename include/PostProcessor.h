#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include <GL/glew.h>
#include <glm/glm.hpp>

class PostProcessor
{
public:
    PostProcessor(GLuint shader, unsigned int width, unsigned int height);
    ~PostProcessor();

    void BeginRender();
    void EndRender();
    void Render(glm::vec2 sphereScreenPos, bool enableBloom, bool enableLensing);

private:
    GLuint FBO;
    GLuint RBO;
    GLuint TextureID;
    GLuint QuadVAO;
    GLuint Shader;
    unsigned int Width, Height;

    void initRenderData();
};

#endif