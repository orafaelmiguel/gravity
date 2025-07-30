// include/Physics.h
#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>

const float VISUAL_SCALE = 0.01f;     
const float SOFTENING_FACTOR = 0.5f; 

struct GravitationalBody {
    glm::vec3 Position;
    float GravitationalParameter; 
};

#endif