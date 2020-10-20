#pragma once

#include "imgui.h"
#include <string>
#include <vector>
#include <map>
#include "opengl_shader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

struct Mesh{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    GLuint IndexCount;
    GLuint MeshVAO;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct Model {
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
};

struct Spotlight {
    glm::vec3 position;
    glm::vec3 direction;
    float angle;
};

struct DirectionalLight {
    glm::vec4 direction;
};

struct Landscape {
   Mesh mesh;
   std::vector<std::vector<float>> heightMap;
   float heightCoefficient;
};

void LoadModel(Model& model,
        const std::string& filename,
        const std::string& basepath,
        float factor);

unsigned int LoadCubemapTexture(std::vector<std::string> faces);
unsigned int LoadCubeVertices(float scale);

void LoadWater(Mesh& water, const std::string& texture_path,
               const std::string& normal_path,
               const std::string& dudv_path,
               float scale,
               float density);

void LoadLandscape(Landscape& landscape,
                   const std::string height_path,
                   const std::string texture_path,
                   const std::string normal_path,
                   float heightCoefficient,
                   int texture_density);

void DrawModel(Model& model, shader_t& shader);
void DrawLandscape(Landscape& model, shader_t& shader);
void DrawMesh(Mesh& mesh, shader_t& shader);
void DrawCubemap(unsigned int vao, unsigned int texture, shader_t& shader);
void DrawWater(Mesh& water, shader_t& shader);