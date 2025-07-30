// include/PostProcessor.h
#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include <GL/glew.h>
#include <glm/glm.hpp>

class PostProcessor
{
public:
    PostProcessor(GLuint postProcessShader, GLuint blurShader, unsigned int width, unsigned int height);
    ~PostProcessor();

    void BeginRender();
    void EndRender();

    void ProcessBloom();
    
    void RenderFinalScene(bool enableBloom);

private:
    // main framebuffer
    GLuint FBO;
    GLuint RBO;
    GLuint SceneTexture; 
    GLuint BrightnessTexture; 

    GLuint PingPongFBO[2];
    GLuint PingPongTexture[2];

    GLuint QuadVAO;
    
    GLuint PostProcessShader;
    GLuint BlurShader;

    unsigned int Width, Height;

    void initRenderData();
};
#endif