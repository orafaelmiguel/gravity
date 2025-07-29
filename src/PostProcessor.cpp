// src/PostProcessor.cpp
#include "PostProcessor.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

PostProcessor::PostProcessor(GLuint postProcessShader, GLuint blurShader, unsigned int width, unsigned int height)
    : PostProcessShader(postProcessShader), BlurShader(blurShader), Width(width), Height(height)
{
    glGenFramebuffers(1, &this->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);

    glGenTextures(1, &this->SceneTexture);
    glBindTexture(GL_TEXTURE_2D, this->SceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->SceneTexture, 0);

    glGenTextures(1, &this->BrightnessTexture);
    glBindTexture(GL_TEXTURE_2D, this->BrightnessTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->BrightnessTexture, 0);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    glGenRenderbuffers(1, &this->RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERRO::POSTPROCESSOR: FBO principal incompleto!" << std::endl;

    glGenFramebuffers(2, this->PingPongFBO);
    glGenTextures(2, this->PingPongTexture);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, this->PingPongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, this->PingPongTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->PingPongTexture[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERRO::POSTPROCESSOR: FBO Ping-pong incompleto!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->initRenderData();
}

PostProcessor::~PostProcessor()
{
    glDeleteFramebuffers(1, &this->FBO);
    glDeleteFramebuffers(2, this->PingPongFBO);
    glDeleteTextures(1, &this->SceneTexture);
    glDeleteTextures(1, &this->BrightnessTexture);
    glDeleteTextures(2, this->PingPongTexture);
    glDeleteRenderbuffers(1, &this->RBO);
    glDeleteVertexArrays(1, &this->QuadVAO);
}

void PostProcessor::initRenderData()
{
    GLuint VBO;
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &this->QuadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindVertexArray(this->QuadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PostProcessor::BeginRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::EndRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::ProcessBloom()
{
    bool horizontal = true, first_iteration = true;
    unsigned int amount = 10; 
    
    glUseProgram(this->BlurShader);
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, this->PingPongFBO[horizontal]);
        glUniform1i(glGetUniformLocation(this->BlurShader, "horizontal"), horizontal);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? this->BrightnessTexture : this->PingPongTexture[!horizontal]);
        
        glBindVertexArray(this->QuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::RenderFinalScene(bool enableBloom)
{
    glUseProgram(this->PostProcessShader);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->SceneTexture);
    glUniform1i(glGetUniformLocation(this->PostProcessShader, "sceneTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->PingPongTexture[0]);
    glUniform1i(glGetUniformLocation(this->PostProcessShader, "bloomTexture"), 1);

    glUniform1i(glGetUniformLocation(this->PostProcessShader, "enableBloom"), enableBloom);

    glBindVertexArray(this->QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}