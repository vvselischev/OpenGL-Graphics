#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

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

    auto mesh = LoadMeshes("../assets/teapot/teapot.obj", "../assets/teapot/", 10);

    std::vector <std::string> faces
            {
                    "../assets/posx.jpg",
                    "../assets/negx.jpg",
                    "../assets/posy.jpg",
                    "../assets/negy.jpg",
                    "../assets/posz.jpg",
                    "../assets/negz.jpg"
            };
    auto cubemap = LoadCubemapTexture(faces);
    auto cubemapVAO = LoadCubemapVertices();

    // init shader
    shader_t modelShader("model_shader.vs", "model_shader.fs");
    shader_t cubemapShader("cubemap_shader.vs", "cubemap_shader.fs");
    shader_t simpleShader("simple_shader.vs", "simple_shader.fs");

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
    bool dragging = false;
    bool shouldProcessMouse;

    glm::vec3 cameraPos = glm::vec3(0, 0, 1);

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
        ImGui::SliderFloat("ratio", &ratio, 1.01, 10);
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
            radius = std::max(0.1f, std::min(1.0f, radius - io.MouseWheel * scaleVelocity));
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
        glm::mat4 vp = Projection * cubemapView;

        glDepthFunc(GL_LESS);
        simpleShader.use();
        simpleShader.set_uniform("model", glm::value_ptr(Model));
        simpleShader.set_uniform("view", glm::value_ptr(View));
        simpleShader.set_uniform("projection", glm::value_ptr(Projection));
        glBindVertexArray(mesh.MeshVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        glDrawElements(GL_TRIANGLES, mesh.IndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDepthFunc(GL_LEQUAL);
        modelShader.use();
        modelShader.set_uniform("model", glm::value_ptr(Model));
        modelShader.set_uniform("view", glm::value_ptr(View));
        modelShader.set_uniform("projection", glm::value_ptr(Projection));
        modelShader.set_uniform("cameraPosition", cameraPos.x, cameraPos.y, cameraPos.z);
        modelShader.set_uniform("ratio", ratio);
        glBindVertexArray(mesh.MeshVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        glDrawElements(GL_TRIANGLES, mesh.IndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        cubemapShader.use();
        cubemapShader.set_uniform("VP", glm::value_ptr(vp));
        glBindVertexArray(cubemapVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

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
