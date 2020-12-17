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
#include <glm/gtc/type_ptr.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

struct Mesh {
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
    glm::vec3 position;
};

struct Spotlight {
    glm::vec3 position;
    glm::vec3 direction;
    float angle;
};

struct Cubemap {
    unsigned int texture;
    unsigned int VAO;
};

struct DirectionalLight {
    glm::vec3 direction;
};

void DrawMesh(Mesh& mesh, shader_t& shader);
void DrawCubemap(unsigned int vao, unsigned int texture, shader_t& shader);

class Scene {
public:
    Model lighthouse;
    Model boat;
    DirectionalLight sun;
    Spotlight projector;
    Cubemap cubemap;

    shader_t modelShader;
    shader_t cubemapShader;
    shader_t simpleShader;
    shader_t landscapeShader;
    shader_t landscapeShaderShadow;
    shader_t modelShaderShadow;

    glm::vec3 cameraPos;
    glm::vec3 cameraDir;
    glm::vec3 cameraUp;
    float boatRotation;

    glm::mat4 Projection;
    glm::mat4 worldModel;
    glm::mat4 View;

    unsigned int cube;
    std::vector<DirectionalLight> lights;

    float waterLevel;
    float waterNormal;

    std::vector<unsigned int> shadowDepthTextures;
    std::vector <glm::mat4> lightSpaceMatrices;
    std::vector<float> planes;

    void DrawScene() {
        glm::mat4 cubemapView = glm::mat4(glm::mat3(View));
        glm::mat4 vp = Projection * cubemapView;
        cubemapShader.use();
        cubemapShader.set_uniform("VP", glm::value_ptr(vp));
        DrawCubemap(cubemap.VAO, cubemap.texture, cubemapShader);

        for (auto light : lights) {
            simpleShader.use();
            glBindVertexArray(cube);
            worldModel = glm::translate(worldModel, light.direction);
            simpleShader.set_uniform("model", glm::value_ptr(worldModel));
            simpleShader.set_uniform("view", glm::value_ptr(View));
            simpleShader.set_uniform("projection", glm::value_ptr(Projection));
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            worldModel = glm::translate(worldModel, -light.direction);
        }
    }
};

unsigned int LoadCubemapTexture(std::vector<std::string> faces);
unsigned int LoadCubeVertices(float scale);

void LoadPlane(Mesh& plane, const std::string& texture_path,
               const std::string& normal_path,
               const std::string& depth_path,
               float scale,
               float density);