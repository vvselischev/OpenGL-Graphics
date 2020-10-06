#pragma once

#include "imgui.h"
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

struct Mesh
{
    std::vector<float> vertices;
    GLuint IndexCount;
    GLuint MeshVAO;
};

Mesh LoadMeshes(
        const std::string& filename,
        const std::string& basepath,
        float factor);

unsigned int LoadCubemapTexture(std::vector<std::string> faces);
unsigned int LoadCubemapVertices();