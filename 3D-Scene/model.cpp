#include "model.h"

#include "3rd-party/tiny_obj_loader.h"
#include "3rd-party/stb_image.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>

#include <fmt/format.h>

#include <GL/glew.h>

#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "opengl_shader.h"

#include<iostream>
#include<string>

bool FileExists(const std::string& abs_filename) {
    bool ret;
    FILE* fp = fopen(abs_filename.c_str(), "rb");
    if (fp) {
        ret = true;
        fclose(fp);
    } else {
        ret = false;
    }

    return ret;
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

void LoadTexture(Model& model,
                 std::string& texture_filename,
                 const std::string& texture_type,
                 std::vector<Texture>& textures,
                 const std::string& basepath) {
    if (texture_filename.length() == 0)
        return;

    unsigned int texture_id;
    int w, h;
    int comp;

    if (!FileExists(texture_filename)) {
        // Append base dir.
        texture_filename = basepath + texture_filename;
        if (!FileExists(texture_filename)) {
            std::cerr << "Unable to find file: " << texture_filename << std::endl;
            exit(1);
        }
    }

    for (auto loaded_texture : model.textures) {
        if (loaded_texture.path == texture_filename) {
            textures.push_back(loaded_texture);
            return;
        }
    }

    std::cout << "Loading texture: " << texture_filename << std::endl;

    unsigned char *image = stbi_load(texture_filename.c_str(), &w, &h, &comp, STBI_default);
    if (!image) {
        std::cerr << "Unable to load texture: " << texture_filename << std::endl;
        exit(1);
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (comp == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    } else if (comp == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    } else {
        assert(0);
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);

    Texture texture;
    texture.id = texture_id;
    texture.type = texture_type;
    texture.path = texture_filename;
    textures.push_back(texture);
    model.textures.push_back(texture);
}

unsigned int LoadCubemapTexture(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int LoadCubeVertices(float scale) {
    float cubeVertices[] = {
            -scale,  scale, -scale,
            -scale, -scale, -scale,
            scale, -scale, -scale,
            scale, -scale, -scale,
            scale,  scale, -scale,
            -scale,  scale, -scale,

            -scale, -scale,  scale,
            -scale, -scale, -scale,
            -scale,  scale, -scale,
            -scale,  scale, -scale,
            -scale,  scale,  scale,
            -scale, -scale,  scale,

            scale, -scale, -scale,
            scale, -scale,  scale,
            scale,  scale,  scale,
            scale,  scale,  scale,
            scale,  scale, -scale,
            scale, -scale, -scale,

            -scale, -scale,  scale,
            -scale,  scale,  scale,
            scale,  scale,  scale,
            scale,  scale,  scale,
            scale, -scale,  scale,
            -scale, -scale,  scale,

            -scale,  scale, -scale,
            scale,  scale, -scale,
            scale,  scale,  scale,
            scale,  scale,  scale,
            -scale,  scale,  scale,
            -scale,  scale, -scale,

            -scale, -scale, -scale,
            -scale, -scale,  scale,
            scale, -scale, -scale,
            scale, -scale, -scale,
            -scale, -scale,  scale,
            scale, -scale,  scale
    };

    unsigned int VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

void DrawCubemap(unsigned int vao, unsigned int texture, shader_t& shader) {
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

Texture LoadTileTexture(const std::string& path) {
    Texture texture;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        if (nrChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else if (nrChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
        exit(1);
    }
    stbi_image_free(data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    texture.path = path;
    texture.id = textureID;
    return texture;
}

void LoadPlane(Mesh& plane, const std::string& texture_path,
               const std::string& normal_path,
               const std::string& depth_path,
               float scale,
               float density) {
    glm::vec3 pos1(-scale,  0.0f, scale);
    glm::vec3 pos2(-scale, 0.0f, -scale);
    glm::vec3 pos3(scale, 0.0f, -scale);
    glm::vec3 pos4(scale,  0.0f, scale);

    glm::vec2 texcoords1(0.0f, density);
    glm::vec2 texcoords2(0.0f, 0.0f);
    glm::vec2 texcoords3(density, 0.0f);
    glm::vec2 texcoords4(density, density);

    glm::vec3 normal(0.0f, 1.0f, 0.0f);

    glm::vec3 tangent1, bitangent1, tangent2, bitangent2;

    glm::vec2 deltaUV1 = texcoords2 - texcoords1;
    glm::vec2 deltaUV2 = texcoords3 - texcoords1;

    float ratio1 = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    tangent1.x = ratio1 * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent1.y = ratio1 * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent1.z = ratio1 * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent1 = glm::normalize(tangent1);

    bitangent1.x = ratio1 * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent1.y = ratio1 * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent1.z = ratio1 * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent1 = glm::normalize(bitangent1);

    deltaUV1 = texcoords3 - texcoords1;
    deltaUV2 = texcoords4 - texcoords1;

    float ratio2 = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    tangent2.x = ratio2 * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent2.y = ratio2 * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent2.z = ratio2 * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent2 = glm::normalize(tangent2);

    bitangent2.x = ratio2 * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent2.y = ratio2 * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent2.z = ratio2 * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent2 = glm::normalize(bitangent2);

    float planeVertices[] = {
            pos1.x, pos1.y, pos1.z,
            normal.x, normal.y, normal.z,
            texcoords1.x, texcoords1.y,
            tangent1.x, tangent1.y, tangent1.z,
            bitangent1.x, bitangent1.y, bitangent1.z,

            pos2.x, pos2.y, pos2.z,
            normal.x, normal.y, normal.z,
            texcoords2.x, texcoords2.y,
            tangent1.x, tangent1.y, tangent1.z,
            bitangent1.x, bitangent1.y, bitangent1.z,

            pos3.x, pos3.y, pos3.z,
            normal.x, normal.y, normal.z,
            texcoords3.x, texcoords3.y,
            tangent1.x, tangent1.y, tangent1.z,
            bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z,
            normal.x, normal.y, normal.z,
            texcoords1.x, texcoords1.y,
            tangent2.x, tangent2.y, tangent2.z,
            bitangent2.x, bitangent2.y, bitangent2.z,

            pos3.x, pos3.y, pos3.z,
            normal.x, normal.y, normal.z,
            texcoords3.x, texcoords3.y,
            tangent2.x, tangent2.y, tangent2.z,
            bitangent2.x, bitangent2.y, bitangent2.z,

            pos4.x, pos4.y, pos4.z,
            normal.x, normal.y, normal.z,
            texcoords4.x, texcoords4.y,
            tangent2.x, tangent2.y, tangent2.z,
            bitangent2.x, bitangent2.y, bitangent2.z
    };

    unsigned int VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    unsigned int indices[] = { 0, 1, 2, 3, 4, 5};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    plane.MeshVAO = VAO;
    plane.IndexCount = 6;

    plane.textures.push_back(LoadTileTexture(texture_path));
    plane.textures.push_back(LoadTileTexture(normal_path));
    plane.textures.push_back(LoadTileTexture(depth_path));
}
