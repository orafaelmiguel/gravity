#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "PostProcessor.h"
#include "ParticleSystem.h"
#include "Physics.h"
#include <glm/gtc/type_ptr.hpp> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
GLuint loadShader(const char* vertexPath, const char* fragmentPath);
void generateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, float radius, int sectorCount, int stackCount);
void calculateTargetDeformation(std::vector<float>& targetVertices, const std::vector<GravitationalBody>& allBodies);


const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

glm::vec3 cameraPos   = glm::vec3(0.0f, 15.0f, 25.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.5f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 objectPos = glm::vec3(0.0f, 1.0f, 0.0f);

bool is_panning = false;
double last_mouse_x = 0.0, last_mouse_y = 0.0;

const int GRID_SIZE = 100;
const float GRID_SCALE = 0.5f;
const float GRID_SMOOTHING_FACTOR = 0.08f;

const std::vector<float> horizontalSpeedSettings = { 0.01f, 0.04f, 0.09f };
const std::vector<float> verticalSpeedSettings = { 0.015f, 0.06f, 0.13f };
const std::vector<std::string> speedNames = { "Lenta", "Normal", "Rápida" };
int currentSpeedIndex = 1; 
bool v_key_pressed_last_frame = false;

bool bloomEnabled = true;
bool lensingEnabled = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::string initial_title = "Simulador de Gravidade [Velocidade: " + speedNames[currentSpeedIndex] + "]";
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, initial_title.c_str(), NULL, NULL);
    if (window == NULL) {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);


    if (glewInit() != GLEW_OK) {
        std::cerr << "Falha ao inicializar GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    GLuint gridShader = loadShader("shaders/grid.vert", "shaders/grid.frag");
    GLuint sphereShader = loadShader("shaders/sphere.vert", "shaders/sphere.frag");
    GLuint postProcessShader = loadShader("shaders/postprocess.vert", "shaders/postprocess.frag");
    GLuint particleShader = loadShader("shaders/particle.vert", "shaders/particle.frag");

    std::vector<float> gridVertices;
    std::vector<float> targetGridVertices; 
    std::vector<unsigned int> gridIndices;

    for (int j = 0; j <= GRID_SIZE; ++j) {
        for (int i = 0; i <= GRID_SIZE; ++i) {
            float x = (i - GRID_SIZE / 2.0f) * GRID_SCALE;
            float z = (j - GRID_SIZE / 2.0f) * GRID_SCALE;
            gridVertices.push_back(x);
            gridVertices.push_back(0.0f);
            gridVertices.push_back(z);
        }
    }
    targetGridVertices = gridVertices;

    for (int j = 0; j < GRID_SIZE; ++j) {
        for (int i = 0; i < GRID_SIZE; ++i) {
            int row1 = j * (GRID_SIZE + 1);
            int row2 = (j + 1) * (GRID_SIZE + 1);
            gridIndices.push_back(row1 + i); gridIndices.push_back(row1 + i + 1);
            gridIndices.push_back(row1 + i); gridIndices.push_back(row2 + i);
        }
    }
    for (int i = 0; i < GRID_SIZE; ++i) {
        gridIndices.push_back((GRID_SIZE * (GRID_SIZE + 1)) + i);
        gridIndices.push_back((GRID_SIZE * (GRID_SIZE + 1)) + i + 1);
        gridIndices.push_back(i * (GRID_SIZE + 1) + GRID_SIZE);
        gridIndices.push_back((i + 1) * (GRID_SIZE + 1) + GRID_SIZE);
    }

    GLuint gridVAO, gridVBO, gridEBO;
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glGenBuffers(1, &gridEBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridIndices.size() * sizeof(unsigned int), gridIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    generateSphere(sphereVertices, sphereIndices, 1.0f, 36, 18);
    GLuint sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    PostProcessor effects(postProcessShader, SCR_WIDTH, SCR_HEIGHT);
    ParticleSystem particles(particleShader, 5000);
    
    float sphereMass = 1.0e13; // star

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        std::vector<GravitationalBody> allBodies;
        allBodies.push_back({ objectPos, sphereMass }); 
        if (particles.TotalMass > 0.1f) {
            allBodies.push_back({ particles.CenterOfMass, particles.TotalMass }); 
        }

        particles.Update(deltaTime, allBodies, 5, objectPos);
        calculateTargetDeformation(targetGridVertices, allBodies);

        for (size_t i = 0; i < gridVertices.size(); i += 3) {
            float currentY = gridVertices[i + 1];
            float targetY = targetGridVertices[i + 1];
            gridVertices[i + 1] += (targetY - currentY) * GRID_SMOOTHING_FACTOR;
        }

        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, gridVertices.size() * sizeof(float), gridVertices.data());

        effects.BeginRender();
        cameraFront = glm::normalize(cameraFront);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), objectPos);
        
        glUseProgram(sphereShader);
        glUniformMatrix4fv(glGetUniformLocation(sphereShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(sphereShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(sphereShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(glGetUniformLocation(sphereShader, "objectColor"), 1, glm::value_ptr(glm::vec3(0.8f, 0.8f, 0.9f)));
        glUniform3fv(glGetUniformLocation(sphereShader, "lightColor"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
        glUniform3fv(glGetUniformLocation(sphereShader, "lightPos"), 1, glm::value_ptr(glm::vec3(5.0f, 10.0f, 5.0f)));
        glUniform3fv(glGetUniformLocation(sphereShader, "viewPos"), 1, glm::value_ptr(cameraPos));
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        glUseProgram(gridShader);
        glUniformMatrix4fv(glGetUniformLocation(gridShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(gridShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(gridShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(gridVAO);
        glDrawElements(GL_LINES, gridIndices.size(), GL_UNSIGNED_INT, 0);

        // render particles
        particles.Render(view, projection);

        effects.EndRender(); 

        model = glm::translate(glm::mat4(1.0f), objectPos);
        glm::vec4 clipSpacePos = projection * view * model * glm::vec4(0.0, 0.0, 0.0, 1.0);
        glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;
        glm::vec2 screenPos = (glm::vec2(ndcSpacePos.x, ndcSpacePos.y) + 1.0f) / 2.0f;

        effects.Render(screenPos, bloomEnabled, lensingEnabled);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &gridVAO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &gridEBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteProgram(gridShader);
    glDeleteProgram(sphereShader);
    glDeleteProgram(postProcessShader);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    bool v_key_is_down = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;
    if (v_key_is_down && !v_key_pressed_last_frame) {
        currentSpeedIndex = (currentSpeedIndex + 1) % speedNames.size();
        std::string newTitle = "Simulador de Gravidade [Velocidade: " + speedNames[currentSpeedIndex] + "]";
        glfwSetWindowTitle(window, newTitle.c_str());
    }
    v_key_pressed_last_frame = v_key_is_down;

    float speed = horizontalSpeedSettings[currentSpeedIndex];
    float verticalSpeed = verticalSpeedSettings[currentSpeedIndex];

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        objectPos.z -= speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        objectPos.z += speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        objectPos.x -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        objectPos.x += speed;
    
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        objectPos.y += verticalSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        objectPos.y -= verticalSpeed;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    float zoomSensitivity = 1.5f;
    cameraPos += cameraFront * (float)yoffset * zoomSensitivity;
    
    if (cameraPos.y < 2.0f) cameraPos.y = 2.0f;
    if (cameraPos.y > 80.0f) cameraPos.y = 80.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            is_panning = true;
            glfwGetCursorPos(window, &last_mouse_x, &last_mouse_y);
        } else if (action == GLFW_RELEASE) {
            is_panning = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!is_panning) {
        return;
    }

    float dx = xpos - last_mouse_x;
    float dy = ypos - last_mouse_y;

    last_mouse_x = xpos;
    last_mouse_y = ypos;

    float pan_sensitivity = 0.05f;
    glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));
    
    cameraPos -= right * dx * pan_sensitivity;
    cameraPos += cameraUp * dy * pan_sensitivity;
}

void calculateTargetDeformation(std::vector<float>& targetVertices, const std::vector<GravitationalBody>& allBodies) {
    for (size_t i = 0; i < targetVertices.size(); i += 3) { 
        float x = targetVertices[i];
        float z = targetVertices[i + 2];
        
        float totalPotential = 0.0f;

        for (const auto& body : allBodies)
        {
            float dx = x - body.Position.x;
            float dz = z - body.Position.z;
            float rSq = dx * dx + dz * dz;
            totalPotential += -G * body.Mass / sqrt(rSq + SOFTENING_FACTOR * SOFTENING_FACTOR);
        }
        targetVertices[i + 1] = totalPotential * VISUAL_SCALE;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLuint loadShader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERRO::SHADER::ARQUIVO_NAO_LIDO_COM_SUCESSO: " << e.what() << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    
    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    
    GLuint ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return ID;
}

void generateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, float radius, int sectorCount, int stackCount) {
    vertices.clear();
    indices.clear();

    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;

    float sectorStep = 2 * (float)M_PI / sectorCount;
    float stackStep = (float)M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = (float)M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}