#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

struct Particle {
    glm::vec3 Position, Velocity;
    glm::vec4 Color;
    float     Life;

    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};

class ParticleSystem
{
public:
    ParticleSystem(GLuint shader, unsigned int amount);
    ~ParticleSystem();

    void Update(float dt, const glm::vec3& gravityObjectPos, unsigned int newParticles, glm::vec3 spawnOffset = glm::vec3(0.0f));
    
    void Render(const glm::mat4& view, const glm::mat4& projection);

private:
    std::vector<Particle> particles;
    unsigned int amount;
    GLuint shader;
    GLuint VAO;

    // help?
    void init();
    unsigned int firstUnusedParticle();
    void respawnParticle(Particle& particle, glm::vec3 spawnOffset);
};

#endif