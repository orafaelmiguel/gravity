#ifndef UTILS_H
#define UTILS_H

#include <GL/glew.h>
#include <vector>

GLuint loadShader(const char* vertexPath, const char* fragmentPath);
void generateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, float radius, int sectorCount, int stackCount);

#endif