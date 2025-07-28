#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>

const float G = 50.0f;                  
const float VISUAL_SCALE = 0.00000001f;   
const float SOFTENING_FACTOR = 0.5f;    

struct GravitationalBody {
    glm::vec3 Position;
    float Mass;
};

#endif