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
    glm::vec4 direction;
};

struct Landscape {
   Mesh mesh;
   std::vector<std::vector<float>> heightMap;
   float heightCoefficient;
   float sandThreshold;
   float grassThreshold;
   int scale;
   int mapWidth;
   int mapHeight;
};

void DrawModel(Model& model, shader_t& shader);
void DrawLandscape(Landscape& model, shader_t& shader);
void DrawMesh(Mesh& mesh, shader_t& shader);
void DrawCubemap(unsigned int vao, unsigned int texture, shader_t& shader);
void DrawWater(Mesh& water, shader_t& shader, unsigned int reflection_texture);

class Scene {
public:
    Landscape landscape;
    Model lighthouse;
    Model boat;
    DirectionalLight sun;
    Spotlight projector;
    Cubemap cubemap;

    shader_t modelShader;
    shader_t cubemapShader;
    shader_t simpleShader;
    shader_t landscapeShader;

    glm::vec3 cameraPos;
    glm::vec3 cameraDir;
    float boatRotation;

    glm::mat4 Projection;
    glm::mat4 worldModel;
    glm::mat4 View;

    unsigned int cube;

    float waterLevel;
    float waterNormal;

    std::vector<unsigned int> shadowDepthTextures;
    std::vector<glm::mat4> lightSpaceMatrices;
    std::vector<float> planes;

    void DrawScene() {
        landscapeShader.use();
        worldModel = glm::translate(worldModel, glm::vec3(0, -0.05, 0));
        landscapeShader.set_uniform("model", glm::value_ptr(worldModel));
        landscapeShader.set_uniform("view", glm::value_ptr(View));
        landscapeShader.set_uniform("projection", glm::value_ptr(Projection));
        landscapeShader.set_uniform("lightSpaceMatrix1", glm::value_ptr(lightSpaceMatrices[0]));
        landscapeShader.set_uniform("lightSpaceMatrix2", glm::value_ptr(lightSpaceMatrices[1]));
        landscapeShader.set_uniform("lightSpaceMatrix3", glm::value_ptr(lightSpaceMatrices[2]));
        landscapeShader.set_uniform("plane1", planes[1]);
        landscapeShader.set_uniform("plane2", planes[2]);
        landscapeShader.set_uniform("plane3", planes[3]);
        landscapeShader.set_uniform("sunPosition", sun.direction.x, sun.direction.y, sun.direction.z);
        landscapeShader.set_uniform("projectorPosition", projector.position.x, projector.position.y, projector.position.z);
        landscapeShader.set_uniform("projectorDirection", projector.direction.x, projector.direction.y, projector.direction.z);
        landscapeShader.set_uniform("projectorAngle", projector.angle);
        landscapeShader.set_uniform("cameraPosition", cameraPos.x, cameraPos.y, cameraPos.z);
        landscapeShader.set_uniform("waterLevel", waterLevel);
        landscapeShader.set_uniform("waterNormal", waterNormal);

        glActiveTexture(GL_TEXTURE0 + 3);
        landscapeShader.set_uniform("shadowMap1", 3);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[0]);
        glActiveTexture(GL_TEXTURE0 + 4);
        landscapeShader.set_uniform("shadowMap1", 4);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[1]);
        glActiveTexture(GL_TEXTURE0 + 5);
        landscapeShader.set_uniform("shadowMap1", 5);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[2]);

        DrawLandscape(landscape, landscapeShader);
        worldModel = glm::translate(worldModel, glm::vec3(0, 0.05, 0));

        modelShader.use();
        worldModel = glm::translate(worldModel, lighthouse.position);
        modelShader.set_uniform("model", glm::value_ptr(worldModel));
        modelShader.set_uniform("view", glm::value_ptr(View));
        modelShader.set_uniform("projection", glm::value_ptr(Projection));

        modelShader.set_uniform("sunPosition", sun.direction.x, sun.direction.y, sun.direction.z);
        modelShader.set_uniform("projectorPosition", projector.position.x, projector.position.y, projector.position.z);
        modelShader.set_uniform("projectorDirection", projector.direction.x, projector.direction.y, projector.direction.z);
        modelShader.set_uniform("projectorAngle", projector.angle);
        modelShader.set_uniform("waterLevel", waterLevel);
        modelShader.set_uniform("waterNormal", waterNormal);
        modelShader.set_uniform("cameraPosition", cameraPos.x, cameraPos.y, cameraPos.z);
        DrawModel(lighthouse, modelShader);

        simpleShader.use();
        glBindVertexArray(cube);
        worldModel = glm::translate(worldModel, -lighthouse.position);
        worldModel = glm::translate(worldModel, projector.position);
        simpleShader.set_uniform("model", glm::value_ptr(worldModel));
        simpleShader.set_uniform("view", glm::value_ptr(View));
        simpleShader.set_uniform("projection", glm::value_ptr(Projection));
        simpleShader.set_uniform("waterLevel", waterLevel);
        simpleShader.set_uniform("waterNormal", waterNormal);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        //worldModel = glm::translate(worldModel, glm::vec3(0, -0.75, 0));
        worldModel = glm::translate(worldModel, -projector.position);

        glm::mat4 cubemapView = glm::mat4(glm::mat3(View));
        glm::mat4 vp = Projection * cubemapView;
        cubemapShader.use();
        cubemapShader.set_uniform("VP", glm::value_ptr(vp));
        DrawCubemap(cubemap.VAO, cubemap.texture, cubemapShader);

        worldModel = glm::translate(worldModel, glm::vec3(0, -0.07, 0));
        worldModel = glm::translate(worldModel, boat.position);
        worldModel = glm::rotate(worldModel, 3.1415f, glm::vec3(0.0, 1.0, 0.0));
        worldModel = glm::rotate(worldModel, boatRotation + 3.1415f / 2, glm::vec3(0.0, 1.0, 0.0));
        modelShader.use();
        modelShader.set_uniform("model", glm::value_ptr(worldModel));
        modelShader.set_uniform("view", glm::value_ptr(View));
        modelShader.set_uniform("projection", glm::value_ptr(Projection));

        modelShader.set_uniform("sunPosition", sun.direction.x, sun.direction.y, sun.direction.z);
        modelShader.set_uniform("projectorPosition", projector.position.x, projector.position.y, projector.position.z);
        modelShader.set_uniform("projectorDirection", projector.direction.x, projector.direction.y, projector.direction.z);
        modelShader.set_uniform("projectorAngle", projector.angle);
        modelShader.set_uniform("waterLevel", waterLevel);
        modelShader.set_uniform("waterNormal", waterNormal);
        modelShader.set_uniform("cameraPosition", cameraPos.x, cameraPos.y, cameraPos.z);
        DrawModel(boat, modelShader);
        worldModel = glm::rotate(worldModel, -3.1415f, glm::vec3(0.0, 1.0, 0.0));
        worldModel = glm::rotate(worldModel, -boatRotation - 3.1415f / 2, glm::vec3(0.0, 1.0, 0.0));
        worldModel = glm::translate(worldModel, glm::vec3(0, 0.07, 0));
        worldModel = glm::translate(worldModel, -boat.position);
    }
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
                   const std::string sand_path,
                   const std::string grass_path,
                   const std::string rock_path,
                   float heightCoefficient,
                   int texture_density,
                   float sandThreshold,
                   float grassThreshold,
                   int scale);

float GetHeight(Landscape& landscape, int x, int z);

unsigned int CreateFrameBuffer();
unsigned int CreateTextureAttachment(int height, int width);
unsigned int CreateDepthTextureAttachment(int height, int width);
unsigned int CreateDepthBufferAttachment(int height, int width);
unsigned int BindFrameBuffer(unsigned int FBO, int height, int width);

unsigned int CreateShadowBuffer(unsigned int shadowWidth, unsigned int shadowHeight, std::vector<unsigned int>& shadowMaps);
void BindShadowBuffer(unsigned int FBO, unsigned int shadowMap);
void BindShadowTexture(std::vector<unsigned  int> shadowMaps);

void CalculateCascades(std::vector<glm::mat4>& lightOrtos, std::vector<float>& cascadePlanes, glm::mat4 cameraView,
                       glm::mat4 lightView, int display_w, int display_h);