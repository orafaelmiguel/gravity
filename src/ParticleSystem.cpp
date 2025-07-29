#include "ParticleSystem.h"
#include <random>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>         
#include <iostream>

ParticleSystem::ParticleSystem(GLuint shader, unsigned int amount)
    : shader(shader), amount(amount)
{
    this->init();
}

ParticleSystem::~ParticleSystem()
{
    glDeleteVertexArrays(1, &this->VAO);
}

void ParticleSystem::init()
{
    GLuint VBO;
    float particle_quad[] = {
        -0.05f,  0.05f,  0.05f, -0.05f, -0.05f, -0.05f,
        -0.05f,  0.05f,  0.05f,  0.05f,  0.05f, -0.05f
    };
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    for (unsigned int i = 0; i < this->amount; ++i)
        this->particles.push_back(Particle());
}

void ParticleSystem::Update(float dt, unsigned int newParticles, glm::vec3 spawnOffset)
{
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle(this->particles[unusedParticle], spawnOffset);
    }
    
    for (Particle& p : this->particles)
    {
        if (p.Life > 0.0f)
        {
            p.Life -= dt; 
            if (p.Life > 0.0f)
            {   
                p.Position += p.Velocity * dt;
                p.Color.a = p.Life / 5.0f;
            }
        }
    }
}

void ParticleSystem::Render(const glm::mat4& view, const glm::mat4& projection)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(this->shader);
    glUniformMatrix4fv(glGetUniformLocation(this->shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(this->shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    for (Particle &particle : this->particles)
    {
        if (particle.Life > 0.0f)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, particle.Position); 
            
            glUniformMatrix4fv(glGetUniformLocation(this->shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform4fv(glGetUniformLocation(this->shader, "particleColor"), 1, glm::value_ptr(particle.Color));

            glBindVertexArray(this->VAO); 
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    
    glDisable(GL_BLEND);
}

unsigned int ParticleSystem::firstUnusedParticle()
{
    for (unsigned int i = 0; i < this->amount; ++i) {
        if (this->particles[i].Life <= 0.0f) {
            return i;
        }
    }
    return 0;
}

unsigned int lastUsedParticle = 0;

void ParticleSystem::respawnParticle(Particle& particle, glm::vec3 spawnOffset)
{
    float jetSpread = 0.3f;  
    float jetSpeed = 3.0f;    
    float spawnRadius = 1.2f; 

    glm::vec3 jetDirection;
    if (lastUsedParticle % 2 == 0) {
        jetDirection = glm::vec3(1.0f, 0.0f, 0.0f); 
    } else {
        jetDirection = glm::vec3(-1.0f, 0.0f, 0.0f); 
    }
    lastUsedParticle++;
    if(lastUsedParticle > 1000) lastUsedParticle = 0; 

    particle.Position = spawnOffset + jetDirection * spawnRadius;
    
    glm::vec3 randomSpread = glm::ballRand(jetSpread);
    
    particle.Velocity = (jetDirection + randomSpread) * jetSpeed;

    if (jetDirection.x > 0) {
         particle.Color = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
    } else {
         particle.Color = glm::vec4(0.2f, 0.5f, 1.0f, 1.0f);
    }

    particle.Life = 8.0f; 
    particle.Mass = 1.0f;
}