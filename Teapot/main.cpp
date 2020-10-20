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

static void glfw_error_callback(int error, const char *description)
{
   std::cerr << fmt::format("Glfw Error {}: {}\n", error, description);
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
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLEW, i.e. fill all possible function pointers for current OpenGL context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!\n";
        return 1;
    }

    Model model;
    LoadModel(model, "../assets/lighthouse/lighthouse.obj", "../assets/lighthouse/", 10);

    std::vector <std::string> faces
            {
                    "../assets/bluecloud_ft.jpg",
                    "../assets/bluecloud_bk.jpg",
                    "../assets/bluecloud_up.jpg",
                    "../assets/bluecloud_dn.jpg",
                    "../assets/bluecloud_rt.jpg",
                    "../assets/bluecloud_lf.jpg"
            };
    auto cubemapTexture = LoadCubemapTexture(faces);
    auto cubemapVAO = LoadCubeVertices(24.0f);

    Mesh water;
    LoadWater(water, "../assets/water.jpg",
              "../assets/water_normal.jpg",
              "../assets/water_dudv.png",
              16.0, 8.0);

    Landscape landscape;
    LoadLandscape(landscape, "../assets/terrain_heightmap.jpg",
                  "../assets/water.jpg",
                  "../assets/grass_normal.png",
                  0.5f,
                  50);

    // init shader
    shader_t modelShader("model_shader.vs", "model_shader.fs");
    shader_t cubemapShader("cubemap_shader.vs", "cubemap_shader.fs");
    shader_t simpleShader("simple_shader.vs", "simple_shader.fs");
    shader_t waterShader("water_shader.vs", "water_shader.fs");
    shader_t landscapeShader("landscape_shader.vs", "landscape_shader.fs");

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
    float windVelocity = 0.00035f;
    float windFactor = 0.0f;
    bool dragging = false;
    bool shouldProcessMouse;

    glm::vec3 cameraPos = glm::vec3(0, 0, 1);

    DirectionalLight sun;
    sun.direction = glm::vec4(1, 0.8, 1, 0.0);

    Spotlight projector;
    projector.angle = glm::radians(15.0f);
    projector.position = glm::vec3(0, 0.3, 0);
    projector.direction = glm::vec3(2, -2, 0);

    unsigned int cube = LoadCubeVertices(1/64.0f);

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Fill background with solid color
        glClearColor(0.30f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Gui start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Teapot");
        ImGui::SliderFloat("ratio", &ratio, 1, 8);
        shouldProcessMouse = !ImGui::IsWindowFocused();
        ImGui::End();

        // Get windows size
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        if (shouldProcessMouse && ImGui::IsMouseDown(0)) {
            if (!dragging) {
                dragging = true;
                clickedX = ImGui::GetMousePos().x;
                clickedY = ImGui::GetMousePos().y;
            }
            float deltaX = ImGui::GetMousePos().x - clickedX;
            float deltaY = ImGui::GetMousePos().y - clickedY;

            glm::mat4 rotationMat(1);
            rotationMat = glm::rotate(rotationMat, glm::radians(-deltaX / display_w * rotationVelocity),
                                      glm::vec3(0.0, 1.0, 0.0));
            cameraPos = glm::vec3(rotationMat * glm::vec4(cameraPos, 1.0));

            float angle = glm::acos(glm::dot(glm::normalize(cameraPos), glm::vec3(0.0, 1.0, 0.0)));
            if (angle > 0.1 && angle < M_PI - 0.1) {
                rotationMat = glm::rotate(rotationMat, glm::radians(-deltaY / display_h * rotationVelocity),
                                          glm::cross(glm::vec3(0.0, 1.0, 0.0), cameraPos));
                glm::vec3 cameraPosAfterYRotation = glm::vec3(rotationMat * glm::vec4(cameraPos, 1.0));

                angle = glm::acos(glm::dot(glm::normalize(cameraPosAfterYRotation), glm::vec3(0.0, 1.0, 0.0)));

                if (angle > 0.1 && angle < M_PI - 0.1) {
                    cameraPos = cameraPosAfterYRotation;
                }
            }
        } else {
            dragging = false;
        }

        if (shouldProcessMouse) {
            radius = std::max(0.1f, radius - io.MouseWheel * scaleVelocity);
        }

        cameraPos = glm::normalize(cameraPos);
        cameraPos *= radius;

        // Set viewport to fill the whole window area
        glViewport(0, 0, display_w, display_h);

        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) display_w / (float) display_h, 0.1f, 100.0f);
        glm::mat4 Model = glm::mat4(1.0f);
        glm::mat4 View = glm::lookAt(
                cameraPos,
                glm::vec3(0, 0, 0),
                glm::vec3(0, 1, 0)
        );

        glm::mat4 cubemapView = glm::mat4(glm::mat3(View));

        windFactor += windVelocity;
        if (windFactor > 1.0) {
            windFactor = 0;
        }

        modelShader.use();
        Model = glm::translate(Model, glm::vec3(0, 0.1, 0));
        modelShader.set_uniform("model", glm::value_ptr(Model));
        modelShader.set_uniform("view", glm::value_ptr(View));
        modelShader.set_uniform("projection", glm::value_ptr(Projection));
        Model = glm::translate(Model, glm::vec3(0, -0.1, 0));

        modelShader.set_uniform("sunPosition", sun.direction.x, sun.direction.y, sun.direction.z);
        modelShader.set_uniform("projectorPosition", projector.position.x, projector.position.y, projector.position.z);
        modelShader.set_uniform("projectorDirection", projector.direction.x, projector.direction.y, projector.direction.z);
        modelShader.set_uniform("projectorAngle", projector.angle);
        modelShader.set_uniform("cameraPosition", cameraPos.x, cameraPos.y, cameraPos.z);
       // DrawModel(model, modelShader);

        waterShader.use();
        waterShader.set_uniform("model", glm::value_ptr(Model));
        waterShader.set_uniform("view", glm::value_ptr(View));
        waterShader.set_uniform("projection", glm::value_ptr(Projection));

        waterShader.set_uniform("sunPosition", sun.direction.x, sun.direction.y, sun.direction.z);
        waterShader.set_uniform("projectorPosition", projector.position.x, projector.position.y, projector.position.z);
        waterShader.set_uniform("projectorDirection", projector.direction.x, projector.direction.y, projector.direction.z);
        waterShader.set_uniform("projectorAngle", projector.angle);
        waterShader.set_uniform("cameraPosition", cameraPos.x, cameraPos.y, cameraPos.z);
        waterShader.set_uniform("windFactor", windFactor);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        //DrawWater(water, waterShader);

        landscapeShader.use();
        landscapeShader.set_uniform("model", glm::value_ptr(Model));
        landscapeShader.set_uniform("view", glm::value_ptr(View));
        landscapeShader.set_uniform("projection", glm::value_ptr(Projection));
        landscapeShader.set_uniform("sunPosition", sun.direction.x, sun.direction.y, sun.direction.z);
        landscapeShader.set_uniform("projectorPosition", projector.position.x, projector.position.y, projector.position.z);
        landscapeShader.set_uniform("projectorDirection", projector.direction.x, projector.direction.y, projector.direction.z);
        landscapeShader.set_uniform("projectorAngle", projector.angle);
        landscapeShader.set_uniform("cameraPosition", cameraPos.x, cameraPos.y, cameraPos.z);
        DrawLandscape(landscape, landscapeShader);

        simpleShader.use();
        glBindVertexArray(cube);
        Model = glm::translate(Model, glm::vec3(0, 0.4, 0));
        simpleShader.set_uniform("model", glm::value_ptr(Model));
        simpleShader.set_uniform("view", glm::value_ptr(View));
        simpleShader.set_uniform("projection", glm::value_ptr(Projection));
        Model = glm::translate(Model, glm::vec3(0, -0.4, 0));

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glm::mat4 vp = Projection * cubemapView;
        cubemapShader.use();
        cubemapShader.set_uniform("VP", glm::value_ptr(vp));
        //DrawCubemap(cubemapVAO, cubemapTexture, cubemapShader);

        // Generate gui render commands
        ImGui::Render();

        // Execute gui render commands using OpenGL backend
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the backbuffer with the frontbuffer that is used for screen display
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
