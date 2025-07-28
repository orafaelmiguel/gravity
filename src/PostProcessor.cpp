// PostProcessor.cpp
#include "PostProcessor.h"
#include <glm/gtc/type_ptr.hpp> 
#include <iostream>

PostProcessor::PostProcessor(GLuint shader, unsigned int width, unsigned int height)
    : Shader(shader), Width(width), Height(height)
{
    glGenFramebuffers(1, &this->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glGenTextures(1, &this->TextureID);
    glBindTexture(GL_TEXTURE_2D, this->TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->TextureID, 0);
    glGenRenderbuffers(1, &this->RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERRO::POSTPROCESSOR: Falha ao inicializar FBO" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->initRenderData();
}

PostProcessor::~PostProcessor()
{
    glDeleteFramebuffers(1, &this->FBO);
    glDeleteTextures(1, &this->TextureID);
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::EndRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::Render(glm::vec2 sphereScreenPos, bool enableBloom, bool enableLensing)
{
    glUseProgram(this->Shader);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(this->Shader, "enableBloom"), enableBloom);
    glUniform1i(glGetUniformLocation(this->Shader, "enableLensing"), enableLensing);
    glUniform2fv(glGetUniformLocation(this->Shader, "sphereScreenPos"), 1, glm::value_ptr(sphereScreenPos));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->TextureID);
    glBindVertexArray(this->QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}