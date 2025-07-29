#include "ParticleSystem.h"
#include "Physics.h"
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>         
#include <iostream>

const float BASE_AMPLITUDE = -4.0f;
const float BASE_Y = 1.0f;
const float HEIGHT_SENSITIVITY = 2.0f;
const float STEEPNESS = 0.2f; 

float calculateGridHeight(float x, float z, const glm::vec3& gravityObjectPos) {
    float dynamic_amplitude = BASE_AMPLITUDE + (gravityObjectPos.y - BASE_Y) * HEIGHT_SENSITIVITY;
    dynamic_amplitude = std::min(0.0f, dynamic_amplitude);
    
    float dx = x - gravityObjectPos.x;
    float dz = z - gravityObjectPos.z;
    float distanceSq = dx * dx + dz * dz;

    return dynamic_amplitude * exp(-STEEPNESS * distanceSq); // <-- CORREÇÃO: Usando a constante correta
}


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

void ParticleSystem::Update(float dt, const std::vector<GravitationalBody>& allBodies, unsigned int newParticles, glm::vec3 spawnOffset)
{
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle(this->particles[unusedParticle], spawnOffset);
    }

    this->TotalMass = 0.0f;
    this->CenterOfMass = glm::vec3(0.0f);
    
    for (Particle& p : this->particles)
    {
        if (p.Life > 0.0f)
        {
            p.Life -= dt;
            if (p.Life > 0.0f)
            {
                glm::vec3 totalForce(0.0f);
                for (const auto& body : allBodies)
                {
                    float distSq = glm::dot(body.Position - p.Position, body.Position - p.Position);
                    float forceMagnitude = body.GravitationalParameter * p.Mass / (distSq + SOFTENING_FACTOR * SOFTENING_FACTOR);
                    
                    glm::vec3 forceDir = glm::normalize(body.Position - p.Position);
                    totalForce += forceDir * forceMagnitude;
                }

                p.Velocity += totalForce / p.Mass * dt;
                p.Position += p.Velocity * dt;
                p.Color.a = p.Life / 5.0f;

                this->TotalMass += p.Mass;
                this->CenterOfMass += p.Position * p.Mass;
            }
        }
    }

    if (this->TotalMass > 0.0f)
    {
        this->CenterOfMass /= this->TotalMass;
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

void ParticleSystem::respawnParticle(Particle& particle, glm::vec3 spawnOffset)
{
    float randomX = ((rand() % 100) - 50) / 20.0f;
    float randomZ = ((rand() % 100) - 50) / 20.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    particle.Position = spawnOffset + glm::vec3(randomX, 2.0f, randomZ); 
    particle.Life = 5.0f;
    particle.Velocity = glm::vec3(0.0f);
    particle.Mass = 1.0f; //mass 
    particle.Color = glm::vec4(rColor, rColor, 1.0f, 1.0f);
}