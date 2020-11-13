#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <map>

#include <fmt/format.h>

#include <GL/glew.h>

// Imgui + bindings
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// Math constant and routines for OpenGL interop
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_shader.h"
#include "model.h"

#include "3rd-party/stb_image.h"

static void glfw_error_callback(int error, const char *description) {
   std::cerr << fmt::format("Glfw Error {}: {}\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

unsigned int reflectionFrameBuffer;
unsigned int reflectionTexture;
unsigned int reflectionDepthBuffer;
unsigned int refractionFrameBuffer;
unsigned int refractionTexture;
unsigned int refractionDepthTexture;

std::vector<unsigned int> shadowFrameBuffers;
std::vector<unsigned int> shadowDepthTextures;

const int REFLECTION_WIDTH = 1280;
const int REFLECTION_HEIGHT = 720;

const int REFRACTION_WIDTH = 1280;
const int REFRACTION_HEIGHT = 720;


void CleanUp() {
    glDeleteFramebuffers(1, &reflectionFrameBuffer);
    glDeleteTextures(1, &reflectionTexture);
    glDeleteRenderbuffers(1, &reflectionDepthBuffer);
    glDeleteFramebuffers(1, &refractionFrameBuffer);
    glDeleteTextures(1, &refractionTexture);
    glDeleteTextures(1, &refractionDepthTexture);
}

void InitReflectionFrameBuffer() {
    reflectionFrameBuffer = CreateFrameBuffer();
    reflectionTexture = CreateTextureAttachment(REFLECTION_HEIGHT, REFLECTION_WIDTH);
    reflectionDepthBuffer = CreateDepthBufferAttachment(REFLECTION_HEIGHT, REFLECTION_WIDTH);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void InitRefractionFrameBuffer() {
    refractionFrameBuffer = CreateFrameBuffer();
    refractionTexture = CreateTextureAttachment(REFRACTION_HEIGHT, REFRACTION_WIDTH);
    refractionDepthTexture = CreateDepthTextureAttachment(REFRACTION_HEIGHT, REFRACTION_WIDTH);
}


int main(int, char **) {
    // Use GLFW to create a simple window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.3 + GLSL 330
    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Conan", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLEW, i.e. fill all possible function pointers for current OpenGL context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!\n";
        return 1;
    }

    glEnable(GL_DEPTH_TEST);

    Scene scene;

    Model lighthouse;
    LoadModel(lighthouse, "../assets/lighthouse/lighthouse.obj", "../assets/lighthouse/", 4);
    scene.lighthouse = lighthouse;

    Model boat;
    LoadModel(boat, "../assets/boat/gondol.obj", "../assets/boat/", 10);
    scene.boat = boat;

    std::vector <std::string> faces
            {
                    "../assets/C.jpg",
                    "../assets/A.jpg",
                    "../assets/up.jpg",
                    "../assets/down.jpg",
                    "../assets/B.jpg",
                    "../assets/D.jpg"
            };

    Cubemap cubemap;
    cubemap.texture = LoadCubemapTexture(faces);
    cubemap.VAO = LoadCubeVertices(100.0f);
    scene.cubemap = cubemap;

    Mesh water;
    LoadWater(water, "../assets/water.jpg",
              "../assets/water_normal.jpg",
              "../assets/water_dudv.png",
              100.0, 60.0);

    Landscape landscape;
    float scale = 20;
    LoadLandscape(landscape, "../assets/terrain_heightmap.jpg",
                  "../assets/sand_texture.jpg",
                  "../assets/grass_texture.png",
                  "../assets/rock_texture.jpg",
                  1.5,
                  50,
                  0.2,
                  0.5,
                  scale);
    scene.landscape = landscape;

    scene.lighthouse.position = glm::vec3(-14, GetHeight(scene.landscape, 14, 7), -7);


    // init shader
    shader_t modelShader("model_shader.vs", "model_shader.fs");
    shader_t cubemapShader("cubemap_shader.vs", "cubemap_shader.fs");
    shader_t simpleShader("simple_shader.vs", "simple_shader.fs");
    shader_t waterShader("water_shader.vs", "water_shader.fs");
    shader_t landscapeShader("landscape_shader.vs", "landscape_shader.fs");
    shader_t landscapeShaderShadow("landscape_shader.vs", "empty_shader.fs");
    shader_t modelShaderShadow("model_shader.vs", "empty_shader.fs");
    scene.modelShader = modelShader;
    scene.cubemapShader = cubemapShader;
    scene.simpleShader = simpleShader;
    scene.landscapeShader = landscapeShader;
    scene.landscapeShaderShadow = landscapeShaderShadow;
    scene.modelShaderShadow = modelShaderShadow;

    // Setup GUI context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    float radius = 0.8;
    float rotationVelocity = 10;
    float scaleVelocity = 0.1;
    float ratio = 1.54;
    float clickedX = 0;
    float clickedY = 0;
    float windVelocity = 0.0006f;
    float windFactor = 0.0f;
    float waterLevel = 0.0f;
    float boatVelocity = 0.1f;
    float cameraVelocity = 0.05;
    float cameraRotationUpVelocity = 0.04;
    float projectorVelocity = 0.04f;
    bool dragging = false;
    bool shouldProcessMouse;

    glm::vec3 boatCentre = glm::vec3(-10, 0, -10);
    glm::vec3 boatRadius = glm::vec3(0, 0, 8.4);
    glm::vec3 boatOrtoRadius = glm::vec3(8.4, 0, 0);
    glm::vec3 boatStartRadius = boatRadius;
    scene.boat.position = boatCentre + boatStartRadius;
    glm::vec3 boatDir = glm::vec3(1, 0, 0);

    scene.cameraPos = glm::vec3(-14, 1.425, -11.53);
    scene.cameraDir = glm::vec3(-0.516, 0.1, 1.17);

    DirectionalLight sun;
    sun.direction = glm::vec4(-1, 0.6, 1, 0.0);
    scene.sun = sun;

    Spotlight projector;
    projector.angle = glm::radians(15.0f);

    projector.position = scene.lighthouse.position + glm::vec3(0, 0.75, 0);
    projector.direction = glm::vec3(2, -0.5f, 0);
    scene.projector = projector;

    unsigned int cube = LoadCubeVertices(1/64.0f);
    scene.cube = cube;

    scene.waterLevel = waterLevel;

    InitReflectionFrameBuffer();
    InitRefractionFrameBuffer();

    std::vector<float> planes {
            0.1, 15.0, 30.0, 200.0
    };

    std::vector<std::pair<int, int>> resolutions {
            {2048, 2048},
            {1024, 1024},
            {256, 256}
    };

    for (int i = 0; i < 3; i++) {
        shadowFrameBuffers.push_back(CreateFrameBuffer());
        shadowDepthTextures.push_back(CreateDepthTextureAttachment(resolutions[i].second, resolutions[i].first));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    scene.planes = planes;

    while (!glfwWindowShouldClose(window)) {
        // Gui start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            scene.cameraPos += scene.cameraDir * cameraVelocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            glm::mat4 rotationMat(1);
            rotationMat = glm::rotate(rotationMat, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
            glm::vec3 left = glm::vec3(rotationMat * glm::vec4(scene.cameraDir, 1.0));
            left.y = 0;
            scene.cameraPos += left * cameraVelocity;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            scene.cameraPos -= scene.cameraDir * cameraVelocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            glm::mat4 rotationMat(1);
            rotationMat = glm::rotate(rotationMat, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
            glm::vec3 right = glm::vec3(rotationMat * glm::vec4(scene.cameraDir, 1.0));
            right.y = 0;
            scene.cameraPos += right * cameraVelocity;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            glm::mat4 rotationMat(1);
            rotationMat = glm::rotate(rotationMat, cameraVelocity, glm::vec3(0.0, 1.0, 0.0));
            scene.cameraDir = glm::vec3(rotationMat * glm::vec4(scene.cameraDir, 1.0));
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            glm::mat4 rotationMat(1);
            rotationMat = glm::rotate(rotationMat, -cameraVelocity, glm::vec3(0.0, 1.0, 0.0));
            scene.cameraDir = glm::vec3(rotationMat * glm::vec4(scene.cameraDir, 1.0));
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            glm::mat4 rotationMat(1);
            rotationMat = glm::rotate(rotationMat, cameraRotationUpVelocity, glm::cross(scene.cameraDir, glm::vec3(0.0, 1.0, 0.0)));
            glm::vec3 newDir = glm::vec3(rotationMat * glm::vec4(scene.cameraDir, 1.0));
            if (glm::abs(glm::sin(glm::acos(glm::dot(glm::normalize(newDir), glm::vec3(0.0, 1.0, 0.0))))) > 0.1) {
                scene.cameraDir = newDir;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            glm::mat4 rotationMat(1);
            rotationMat = glm::rotate(rotationMat, -cameraRotationUpVelocity, glm::cross(scene.cameraDir, glm::vec3(0.0, 1.0, 0.0)));
            glm::vec3 newDir = glm::vec3(rotationMat * glm::vec4(scene.cameraDir, 1.0));
            if (glm::abs(glm::sin(glm::acos(glm::dot(glm::normalize(newDir), glm::vec3(0.0, 1.0, 0.0))))) > 0.1) {
                scene.cameraDir = newDir;
            }
        }

        std::vector<glm::mat4> lightProjections;
        std::vector<glm::mat4> lightSpaceMatrices;
        for (int i = 0; i < 3; i++) {
            lightSpaceMatrices.push_back(glm::mat4(0.0));
            lightProjections.push_back(glm::mat4(0.0));
        }

        scene.shadowDepthTextures = shadowDepthTextures;
        scene.lightSpaceMatrices = lightSpaceMatrices;

        // Get windows size
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        glm::mat4 rotationBoat(1);
        rotationBoat = glm::rotate(rotationBoat, glm::radians(boatVelocity),
                                   glm::vec3(0.0, 1.0, 0.0));
        boatRadius = glm::vec3(rotationBoat * glm::vec4(boatRadius, 1.0));
        scene.boatRotation = glm::atan(glm::dot(boatRadius, boatOrtoRadius), glm::dot(boatRadius, boatStartRadius));
        scene.boat.position = boatCentre + boatRadius;

        //scene.cameraPos = scene.boat.position;
        //scene.cameraPos.y += 0.2f;

        // Set viewport to fill the whole window area
        glViewport(0, 0, display_w, display_h);
        float fov = glm::radians(45.0f);
        glm::mat4 Projection = glm::perspective(fov, (float) display_w / (float) display_h, 0.1f, 200.0f);
        scene.Projection = Projection;
        scene.worldModel = glm::mat4(1.0f);
        scene.View = glm::lookAt(
                scene.cameraPos,
                scene.cameraDir + scene.cameraPos,
                glm::vec3(0, 1, 0)
        );

        glm::mat4 rotationProjector(1);
        rotationProjector = glm::rotate(rotationProjector, projectorVelocity, glm::vec3(0.0, 1.0, 0.0));
        scene.projector.direction = glm::vec3(rotationProjector * glm::vec4(scene.projector.direction, 1.0));

        windFactor += windVelocity;
        if (windFactor > 1.0) {
            windFactor = 0;
        }

        glEnable(GL_CLIP_DISTANCE0);

        glBindFramebuffer(GL_FRAMEBUFFER, reflectionFrameBuffer);
        glViewport(0, 0, REFLECTION_WIDTH, REFRACTION_HEIGHT);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float cameraToWaterDistance = scene.cameraPos.y - waterLevel;
        scene.cameraPos.y -= 2 * cameraToWaterDistance;
        scene.cameraDir.y *= -1;
        scene.View = glm::lookAt(
                scene.cameraPos,
                scene.cameraDir + scene.cameraPos,
                glm::vec3(0, 1, 0)
        );

        glm::mat4 oldProjection = scene.Projection;
        glm::vec4 waterPlane = glm::vec4(0, 1, 0, -waterLevel);
        Projection = CalculateOblique(oldProjection, scene.View * waterPlane);
        scene.waterNormal = 1.0f;
        scene.DrawScene();
        scene.cameraPos.y += 2 * cameraToWaterDistance;
        scene.cameraDir.y *= -1;
        scene.View = glm::lookAt(
                scene.cameraPos,
                scene.cameraDir + scene.cameraPos,
                glm::vec3(0, 1, 0)
        );
        scene.Projection = oldProjection;

        glBindFramebuffer(GL_FRAMEBUFFER, refractionFrameBuffer);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.waterNormal = -1.0f;
        scene.DrawScene();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CLIP_DISTANCE0);

        float nearPlane = 1.0f, farPlane = 24.0f;
        glm::mat4 lightView = glm::lookAt(glm::vec3(0,-1,0) + 10.0f * glm::vec3(scene.sun.direction.x, scene.sun.direction.y, scene.sun.direction.z),
                                          glm::vec3(0.0f, 0.0f,  0.0f),
                                          glm::vec3(0.0f, 1.0f,  0.0f));

        CalculateCascades(lightProjections, planes, scene.View, lightView, display_w, display_h, fov);

        glm::mat4 oldView = scene.View;
        oldProjection = scene.Projection;

        for (int i = 0; i < 3; i++) {
            glViewport(0, 0, resolutions[i].first, resolutions[i].second);
            glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffers[i]);
            glClear(GL_DEPTH_BUFFER_BIT);

            scene.View = lightView;
            scene.Projection = lightProjections[i];
            lightSpaceMatrices[i] = scene.Projection * scene.View;

            scene.DrawShadows();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.View = oldView;
        scene.Projection = oldProjection;

        scene.shadowDepthTextures = shadowDepthTextures;
        scene.lightSpaceMatrices = lightSpaceMatrices;

        scene.DrawScene();

        waterShader.use();
        waterShader.set_uniform("model", glm::value_ptr(scene.worldModel));
        waterShader.set_uniform("view", glm::value_ptr(scene.View));
        waterShader.set_uniform("projection", glm::value_ptr(scene.Projection));

        waterShader.set_uniform("sunPosition", sun.direction.x, sun.direction.y, -sun.direction.z);
        waterShader.set_uniform("projectorPosition", scene.projector.position.x, scene.projector.position.y, scene.projector.position.z);
        waterShader.set_uniform("projectorDirection", scene.projector.direction.x, scene.projector.direction.y, scene.projector.direction.z);
        waterShader.set_uniform("projectorAngle", projector.angle);
        waterShader.set_uniform("cameraPosition", scene.cameraPos.x, scene.cameraPos.y, scene.cameraPos.z);
        waterShader.set_uniform("windFactor", windFactor);

        glActiveTexture(GL_TEXTURE0);
        waterShader.set_uniform("reflection_texture", 0);
        glBindTexture(GL_TEXTURE_2D, reflectionTexture);
        glActiveTexture(GL_TEXTURE0 + 1);
        waterShader.set_uniform("refraction_texture", 1);
        glBindTexture(GL_TEXTURE_2D, scene.landscape.mesh.textures[0].id);
        glActiveTexture(GL_TEXTURE0 + 2);
        waterShader.set_uniform("water_normal", 2);
        glBindTexture(GL_TEXTURE_2D, water.textures[1].id);

        glActiveTexture(GL_TEXTURE0 + 3);
        waterShader.set_uniform("water_dudv", 3);
        glBindTexture(GL_TEXTURE_2D, water.textures[2].id);


        glBindVertexArray(water.MeshVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Generate gui render commands
        ImGui::Render();

        // Execute gui render commands using OpenGL backend
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the backbuffer with the frontbuffer that is used for screen display
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    CleanUp();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
