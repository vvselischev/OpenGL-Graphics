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

    Mesh plane;
    LoadPlane(plane, "../assets/wall_texture.jpg",
              "../assets/wall_normal.jpg",
              "../assets/wall_height_old.png",
              1, 1);


    // init shader
    shader_t cubemapShader("cubemap_shader.vs", "cubemap_shader.fs");
    shader_t simpleShader("simple_shader.vs", "simple_shader.fs");
    shader_t wallShader("wall_shader.vs", "wall_shader.fs");
    scene.cubemapShader = cubemapShader;
    scene.simpleShader = simpleShader;

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
    float cameraVelocity = 0.02;
    float cameraRotationUpVelocity = 0.04;
    bool dragging = false;
    bool shouldProcessMouse;

    float reflectivity = 200;

    scene.cameraPos = glm::vec3(0, 3, 0);
    scene.cameraDir = glm::vec3(0.2, -1, 0.2);

    DirectionalLight light1;
    light1.direction = glm::vec3(-1, 0.3, -1);

    DirectionalLight light2;
    light2.direction = glm::vec3(0, 0.4, 1);

    DirectionalLight light3;
    light3.direction = glm::vec3(-1, 0.1, 0);

    DirectionalLight light4;
    light4.direction = glm::vec3(-1, 0.4, 1);

    scene.cube = LoadCubeVertices(0.01);

    float maxHeight = 0.006;
    int maxStepCount = 90;
    float stepLength = 1;
    int stepLengthRatio = 10;
    float currentFrames = 0;
    float currentTime = 0;
    float fps = 0;
    auto t1 = std::chrono::steady_clock::now();


    while (!glfwWindowShouldClose(window)) {
        // Gui start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Parallax Occlusion");

        ImGui::SliderFloat("Max height", &maxHeight, 0.0001, 0.01);
        ImGui::SliderInt("Max step count", &maxStepCount, 2, 128);

        stepLength = (float) stepLengthRatio;

        ImGui::SliderFloat("light1.x", &light1.direction.x, -4, 4);
        ImGui::SliderFloat("light1.y", &light1.direction.y, 0, 2);
        ImGui::SliderFloat("light1.z", &light1.direction.z, -4, 4);

        ImGui::SliderFloat("light2.x", &light1.direction.x, -4, 4);
        ImGui::SliderFloat("light2.y", &light1.direction.y, 0, 2);
        ImGui::SliderFloat("light2.z", &light1.direction.z, -4, 4);

        ImGui::SliderFloat("light3.x", &light3.direction.x, -4, 4);
        ImGui::SliderFloat("light3.y", &light3.direction.y, 0, 2);
        ImGui::SliderFloat("light3.z", &light3.direction.z, -4, 4);

        ImGui::SliderFloat("light4.x", &light4.direction.x, -4, 4);
        ImGui::SliderFloat("light4.y", &light4.direction.y, 0, 2);
        ImGui::SliderFloat("light4.z", &light4.direction.z, -4, 4);


        const char* items[] = { "PLAIN", "NORMAL BUMP", "POM", "POM & SHADOWS" };
        static const char* current_item = items[3];

        if (ImGui::BeginCombo("Mode", current_item)) {
            for (int i = 0; i < 4; i++) {
                bool is_selected = (current_item == items[i]);
                if (ImGui::Selectable(items[i], is_selected)) {
                    current_item = items[i];
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::End();

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

        // Get windows size
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Set viewport to fill the whole window area
        glViewport(0, 0, display_w, display_h);
        float fov = glm::radians(45.0f);
        float near = 0.1f;
        float far = 200.0f;
        glm::mat4 Projection = glm::perspective(fov, (float) display_w / (float) display_h, near, far);
        scene.Projection = Projection;
        scene.worldModel = glm::mat4(1.0f);
        scene.View = glm::lookAt(
                scene.cameraPos,
                scene.cameraDir + scene.cameraPos,
                glm::vec3(0, 1, 0)
        );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.waterNormal = -1.0f;

        scene.lights.clear();
        scene.lights.push_back(light1);
        scene.lights.push_back(light2);
        scene.lights.push_back(light3);
        scene.lights.push_back(light4);

        scene.DrawScene();

        wallShader.use();
        wallShader.set_uniform("model", glm::value_ptr(scene.worldModel));
        wallShader.set_uniform("view", glm::value_ptr(scene.View));
        wallShader.set_uniform("projection", glm::value_ptr(scene.Projection));

        wallShader.set_uniform("MAX_STEP_COUNT", maxStepCount);
        wallShader.set_uniform("MAX_HEIGHT", maxHeight);
        wallShader.set_uniform("STEP_LENGTH", stepLength);

        wallShader.set_uniform("normalBump", false);
        wallShader.set_uniform("pom", false);
        wallShader.set_uniform("pomAndShadows", false);

        if (strcmp(current_item, "NORMAL BUMP") == 0) {
            wallShader.set_uniform("normalBump", true);
        } else if (strcmp(current_item, "POM") == 0) {
            wallShader.set_uniform("pom", true);
        } else if (strcmp(current_item, "POM & SHADOWS") == 0) {
            wallShader.set_uniform("pomAndShadows", true);
        }

        auto zBufferParams = glm::vec4(1 - far / near, far / near, 1 / far - 1 / near, 1 / near);
        wallShader.set_uniform("zBufferParams", zBufferParams.x, zBufferParams.y, zBufferParams.z, zBufferParams.w);

        wallShader.set_uniform("cameraPosition", scene.cameraPos.x, scene.cameraPos.y, scene.cameraPos.z);
        wallShader.set_uniform("reflectivity", reflectivity);


        wallShader.set_uniform("worldLightDir1", light1.direction.x, light1.direction.y, light1.direction.z);
        wallShader.set_uniform("worldLightDir2", light2.direction.x, light2.direction.y, light2.direction.z);
        wallShader.set_uniform("worldLightDir3", light3.direction.x, light3.direction.y, light3.direction.z);
        wallShader.set_uniform("worldLightDir4", light4.direction.x, light4.direction.y, light4.direction.z);

        glActiveTexture(GL_TEXTURE0);
        wallShader.set_uniform("wall_texture", 0);
        glBindTexture(GL_TEXTURE_2D, plane.textures[0].id);
        glActiveTexture(GL_TEXTURE0 + 1);
        wallShader.set_uniform("wall_normal", 1);
        glBindTexture(GL_TEXTURE_2D, plane.textures[1].id);
        glActiveTexture(GL_TEXTURE0 + 2);
        wallShader.set_uniform("wall_height", 2);
        glBindTexture(GL_TEXTURE_2D, plane.textures[2].id);


        glBindVertexArray(plane.MeshVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        auto duration = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t1).count();

       // std::cout << duration / 1000 << std::endl;
        currentFrames++;
        currentTime += duration;
        if (currentTime >= 1000) {
            fps = currentFrames / (currentTime / 1000);
            currentTime = 0;
            currentFrames = 0;
        }

        ImGui::Begin("Stats");
        ImGui::Text("ms per frame: %f", duration);
        ImGui::Text("FPS: %f", fps);
        ImGui::End();

        // Generate gui render commands
        ImGui::Render();

        // Execute gui render commands using OpenGL backend
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the backbuffer with the frontbuffer that is used for screen display
        glfwSwapBuffers(window);
        glfwPollEvents();

        t1 = std::chrono::steady_clock::now();
    }


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
