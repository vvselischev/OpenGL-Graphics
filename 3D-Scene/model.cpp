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

Mesh LoadMesh(Model& model,
        tinyobj::attrib_t& attrib,
        tinyobj::mesh_t& meshToAdd,
        std::vector<tinyobj::material_t>& materials,
        const std::string& basepath,
        float factor) {
    Mesh newMesh;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    int indicesOffset = 0;

    for (int f = 0; f < meshToAdd.num_face_vertices.size(); f++) {
        int faceVerticesNumber = meshToAdd.num_face_vertices[f];

        for (int v = 0; v < faceVerticesNumber; v++) {
            tinyobj::index_t i = meshToAdd.indices[indicesOffset + v];
            vertices.push_back(attrib.vertices[3 * i.vertex_index] / factor);
            vertices.push_back(attrib.vertices[3 * i.vertex_index + 1] / factor);
            vertices.push_back(attrib.vertices[3 * i.vertex_index + 2] / factor);
            vertices.push_back(attrib.normals[3 * i.normal_index]);
            vertices.push_back(attrib.normals[3 * i.normal_index + 1]);
            vertices.push_back(attrib.normals[3 * i.normal_index + 2]);
            vertices.push_back(attrib.texcoords[2 * i.texcoord_index]);
            vertices.push_back(attrib.texcoords[2 * i.texcoord_index + 1]);
        }

        for (int v = 0; v < faceVerticesNumber; v++) {
            indices.push_back(indices.size());
        }

        indicesOffset += faceVerticesNumber;
    }

    for (int i : meshToAdd.material_ids) {
        LoadTexture(model, materials[i].diffuse_texname, "texture_diffuse", textures, basepath);
        LoadTexture(model, materials[i].specular_texname, "texture_specular", textures, basepath);
        LoadTexture(model, materials[i].normal_texname, "texture_normal", textures, basepath);
        LoadTexture(model, materials[i].ambient_texname, "texture_ambient", textures, basepath);

        newMesh.ambient = glm::vec3(materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
        newMesh.specular = glm::vec3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
        newMesh.diffuse = glm::vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);

        if (textures.size() == 1)
            break;
    }

    unsigned int VBO, EBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 6));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    newMesh.vertices = vertices;
    newMesh.indices = indices;
    newMesh.textures = textures;
    newMesh.IndexCount = indices.size();
    newMesh.MeshVAO = VAO;

    return newMesh;
}

void LoadModel(Model& model, const std::string& filename, const std::string& basepath, float factor) {
    tinyobj::attrib_t attrib;
    std::vector <tinyobj::shape_t> shapes;
    std::vector <tinyobj::material_t> materials;
    std::string err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str(), basepath.c_str())) {
        fprintf(stderr, "tinyobj::LoadObj(%s) error: %s\n", filename.c_str(), err.c_str());
        exit(1);
    }

    if (!err.empty()) {
        fprintf(stderr, "tinyobj::LoadObj(%s) warning: %s\n", filename.c_str(), err.c_str());
    }

    for (int k = 0; k < shapes.size(); k++) {
        model.meshes.push_back(LoadMesh(model, attrib, shapes[k].mesh, materials, basepath, factor));
    }
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

void DrawModel(Model& model, shader_t& shader) {
    for (auto mesh : model.meshes) {
        DrawMesh(mesh, shader);
    }
}

void DrawMesh(Mesh& mesh, shader_t& shader) {
    unsigned int diffuseNumber  = 1;
    unsigned int specularNumber  = 1;
    unsigned int ambientNumber  = 1;
    unsigned int normalNumber  = 1;

    if (mesh.textures.size() == 0)
        return;

    for (unsigned int i = 0; i < mesh.textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name = mesh.textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNumber++);
        if (name == "texture_specular")
            number = std::to_string(specularNumber++);
        if (name == "texture_ambient")
            number = std::to_string(ambientNumber++);
        if (name == "texture_normal")
            number = std::to_string(normalNumber++);
        shader.set_uniform(name + number, (int) i);
        glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        glActiveTexture(GL_TEXTURE0);
    }

    shader.set_uniform("in_ambient", mesh.ambient.x, mesh.ambient.y, mesh.ambient.z);
    shader.set_uniform("in_specular", mesh.specular.x, mesh.specular.y, mesh.specular.z);
    shader.set_uniform("in_diffuse", mesh.diffuse.x, mesh.diffuse.y, mesh.diffuse.z);

    glBindVertexArray(mesh.MeshVAO);
    glDrawElements(GL_TRIANGLES, mesh.IndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
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

void DrawWater(Mesh& water, shader_t& shader, unsigned int reflection_texture) {
    glActiveTexture(GL_TEXTURE0);
    shader.set_uniform("reflection_texture", 0);
    glBindTexture(GL_TEXTURE_2D, reflection_texture);

    glBindVertexArray(water.MeshVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void LoadWater(Mesh& water, const std::string& texture_path,
               const std::string& normal_path,
               const std::string& dudv_path,
               float scale,
               float density) {
    float planeVertices[] = {
            scale, 0.0f, scale,
            0.0f, density,
            scale, 0.0f,  -scale,
            0.0f, 0.0f,
            -scale, 0.0f, -scale,
            density, 0.0f,
            -scale, 0.0f,  scale,
            density, density
    };

    unsigned int indices[] = { 0, 1, 3, 1, 2, 3};

    unsigned int VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    water.MeshVAO = VAO;
    water.IndexCount = 6;

    water.textures.push_back(LoadTileTexture(texture_path));
    water.textures.push_back(LoadTileTexture(normal_path));
    water.textures.push_back(LoadTileTexture(dudv_path));
}

void DrawLandscape(Landscape& model, shader_t& shader) {
    glActiveTexture(GL_TEXTURE0);
    shader.set_uniform("sand_texture", 0);
    glBindTexture(GL_TEXTURE_2D, model.mesh.textures[0].id);

    glActiveTexture(GL_TEXTURE0 + 1);
    shader.set_uniform("grass_texture", 1);
    glBindTexture(GL_TEXTURE_2D, model.mesh.textures[1].id);

    glActiveTexture(GL_TEXTURE0 + 2);
    shader.set_uniform("rock_texture", 2);
    glBindTexture(GL_TEXTURE_2D, model.mesh.textures[2].id);

    shader.set_uniform("sand_threshold", model.sandThreshold);
    shader.set_uniform("grass_threshold", model.grassThreshold);

    glBindVertexArray(model.mesh.MeshVAO);
    glDrawElements(GL_TRIANGLES, model.mesh.IndexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void LoadLandscape(Landscape& landscape,
                   const std::string height_path,
                   const std::string sand_path,
                   const std::string grass_path,
                   const std::string rock_path,
                   float heightCoefficient,
                   int texture_density,
                   float sandThreshold,
                   float grassThreshold,
                   int scale) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(height_path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        if (nrChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else if (nrChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
    } else {
        std::cout << "Landscape tex failed to load at path: " << height_path << std::endl;
        stbi_image_free(data);
        exit(1);
    }

    int currentPixel = 0;
    for (int i = 0; i < height; i++) {
        std::vector<float> row;
        for (int j = 0; j < width; j++) {
            row.push_back(data[currentPixel] / 255.0f * heightCoefficient);
            currentPixel += nrChannels;
        }
        landscape.heightMap.push_back(row);
    }

    stbi_image_free(data);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int indicesOffset = 0;
    for (int hh = 1; hh <= height - 2; hh += 2) {
        for (int ww = 1; ww <= width - 2; ww += 2) {
            float i = ((float)hh / height  - 1) * scale;
            float j = ((float)ww / width - 1) * scale;
            float shiftW = scale * 1.0 / width;
            float shiftH = scale * 1.0 / height;

            int ii = hh;
            int jj = ww;

            float density_f = (float)texture_density;
            float dHPlus =  ((hh + 1) % texture_density) / density_f;
            if (dHPlus == 0) {
                dHPlus = 1;
            }
            float dWPlus =  ((ww + 1) % texture_density) / density_f;
            if (dWPlus == 0) {
                dWPlus = 1;
            }

            float current[] = {
                    i, landscape.heightMap[ii][jj + 1], j + shiftW,
                    (hh % texture_density) / density_f, dWPlus,
                    i + shiftH, landscape.heightMap[ii + 1][jj + 1], j + shiftW,
                    dHPlus, dWPlus,
                    i, landscape.heightMap[ii][jj], j,
                    (hh % texture_density) / density_f, (ww % texture_density) / density_f,
                    i + shiftH, landscape.heightMap[ii + 1][jj], j,
                    dHPlus, (ww % texture_density) / density_f,

                    i + shiftH, landscape.heightMap[ii + 1][jj], j,
                    dHPlus, (ww % texture_density) / density_f,
                    i + shiftH, landscape.heightMap[ii + 1][jj - 1], j - shiftW,
                    dHPlus, ((ww - 1) % texture_density) / density_f,
                    i, landscape.heightMap[ii][jj], j,
                    (hh % texture_density) / density_f, (ww % texture_density) / density_f,
                    i, landscape.heightMap[ii][jj - 1], j - shiftW,
                    (hh % texture_density) / density_f, ((ww - 1) % texture_density) / density_f,

                    i - shiftH, landscape.heightMap[ii - 1][jj], j,
                    ((hh - 1) % texture_density) / density_f, (ww % texture_density) / density_f,
                    i, landscape.heightMap[ii][jj], j,
                    (hh % texture_density) / density_f, (ww % texture_density) / density_f,
                    i - shiftH, landscape.heightMap[ii - 1][jj - 1], j - shiftW,
                    ((hh - 1) % texture_density) / density_f, ((ww - 1) % texture_density) / density_f,
                    i, landscape.heightMap[ii][jj - 1], j - shiftW,
                    (hh % texture_density) / density_f, ((ww - 1) % texture_density) / density_f,

                    i - shiftH, landscape.heightMap[ii - 1][jj], j,
                    ((hh - 1) % texture_density) / density_f, (ww % texture_density) / density_f,
                    i - shiftH, landscape.heightMap[ii - 1][jj + 1], j + shiftW,
                    ((hh - 1) % texture_density) / density_f, dWPlus,
                    i, landscape.heightMap[ii][jj], j,
                    (hh % texture_density) / density_f, (ww % texture_density) / density_f,
                    i, landscape.heightMap[ii][jj + 1], j + shiftW,
                    (hh % texture_density) / density_f, dWPlus
            };

            unsigned int currentIndices[] = {
                    0, 1, 2, 2, 1, 3
            };

            for (int k = 0; k < 5 * 4 * 4; k++) {
                vertices.push_back(current[k]);
            }

            for (int q = 0; q < 4; q++) {
                for (int k = 0; k < 6; k++) {
                    indices.push_back(indicesOffset + currentIndices[k]);
                }
                indicesOffset += 4;
            }
        }
    }

    unsigned int VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    landscape.mesh.IndexCount = indices.size();
    landscape.mesh.MeshVAO = VAO;
    landscape.mesh.textures.push_back(LoadTileTexture(sand_path));
    landscape.mesh.textures.push_back(LoadTileTexture(grass_path));
    landscape.mesh.textures.push_back(LoadTileTexture(rock_path));

    landscape.grassThreshold = grassThreshold;
    landscape.sandThreshold = sandThreshold;

    landscape.scale = scale;
    landscape.mapWidth = width;
    landscape.mapHeight = height;
}

float GetHeight(Landscape& landscape, int x, int z) {
    return landscape.heightMap[landscape.mapHeight - x * landscape.mapHeight / landscape.scale]
        [landscape.mapWidth - z * landscape.mapHeight / landscape.scale];
}

unsigned int CreateFrameBuffer() {
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    return FBO;
}

unsigned int CreateTextureAttachment(int height, int width) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
    return textureID;
}

unsigned int CreateDepthTextureAttachment(int height, int width) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    return textureID;
}

unsigned int CreateDepthBufferAttachment(int height, int width) {
    unsigned int RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
    return RBO;
}

unsigned int BindFrameBuffer(unsigned int FBO, int height, int width) {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

unsigned int CreateShadowBuffer(unsigned int shadowWidth, unsigned int shadowHeight, std::vector<unsigned int>& shadowMaps) {
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);

    glGenTextures(shadowMaps.size(), &shadowMaps[0]);

    for (int i = 0 ; i < shadowMaps.size() ; i++) {
        glBindTexture(GL_TEXTURE_2D, shadowMaps[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMaps[0], 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << status;
        exit(1);
    }

    return FBO;
}

void CalculateCascades(std::vector<glm::mat4>& lightProjections, std::vector<float>& cascadePlanes, glm::mat4 cameraView,
                       glm::mat4 lightView, int display_w, int display_h, float displayAngle) {
    float fovV = displayAngle;
    float ar = (float) display_w / (float) display_h;
    float fovH  = glm::atan(glm::tan(fovV / 2) * ar) * 2;
    float tanV = glm::tan(fovV / 2);
    float tanH = glm::tan(fovH / 2);

    glm::mat4 cameraInverse = glm::inverse(cameraView);

    for (int i = 0; i < cascadePlanes.size() - 1; i++) {
        float xNear = cascadePlanes[i] * tanH;
        float xFar = cascadePlanes[i + 1] * tanH;
        float yNear = cascadePlanes[i] * tanV;
        float yFar = cascadePlanes[i + 1] * tanV;

        glm::vec4 frustum[8] {
                glm::vec4(xNear, yNear, -cascadePlanes[i], 1.0),
                glm::vec4(-xNear, yNear, -cascadePlanes[i], 1.0),
                glm::vec4(xNear, -yNear, -cascadePlanes[i], 1.0),
                glm::vec4(-xNear, -yNear, -cascadePlanes[i], 1.0),

                glm::vec4(xFar, yFar, -cascadePlanes[i + 1], 1.0),
                glm::vec4(-xFar, yFar, -cascadePlanes[i + 1], 1.0),
                glm::vec4(xFar, -yFar, -cascadePlanes[i + 1], 1.0),
                glm::vec4(-xFar, -yFar, -cascadePlanes[i + 1], 1.0)
        };

        glm::vec4 lightFrustum[8];

        float minX = 1000;
        float maxX = -1000;
        float minY = 1000;
        float maxY = -1000;
        float minZ = 1000;
        float maxZ = -1000;

        for (int j = 0; j < 8; j++) {
            glm::vec4 worldCorner = cameraInverse * frustum[j];
            lightFrustum[j] = lightView * worldCorner;
            minX = std::min(minX, lightFrustum[j].x);
            maxX = std::max(maxX, lightFrustum[j].x);
            minY = std::min(minY, lightFrustum[j].y);
            maxY = std::max(maxY, lightFrustum[j].y);
            minZ = std::min(minZ, lightFrustum[j].z);
            maxZ = std::max(maxZ, lightFrustum[j].z);
        }

        if (minZ > 0.0f)
            minZ = 0.0f;
        if (maxZ > 0.0f)
            maxZ = 0.0f;
        glm::mat4 lightProjection = glm::ortho(minX - 1, maxX + 1, minY - 1, maxY + 1, -maxZ - 1, -minZ + 1);
        lightProjections[i] = lightProjection;
    }
}

glm::mat4 CalculateOblique(glm::mat4 cameraProjection, glm::vec4 plane) {
    glm::mat4 inverse = glm::inverse(cameraProjection);
    // glm::vec4 v = glm::vec4(sgn(plane.x), sgn(plane.y), 1.0f, 1.0f);
    // glm::vec4 q = inverse *  v;
    glm::vec4 q;
    q.x = (sgn(plane.x) + cameraProjection[0][2]) / cameraProjection[0][0];
    q.y = (sgn(plane.y) + cameraProjection[1][2]) / cameraProjection[1][1];
    q.z = -1.0f;
    q.w = (1.0f + cameraProjection[2][2]) / cameraProjection[2][3];
    glm::vec4 c = plane * (2.0f / (glm::dot(plane, q)));

    glm::mat4 result = cameraProjection;
    result[2][0] = c.x;
    result[2][1] = c.y;
    result[2][2] = c.z + 1.0f;
    result[2][3] = c.w;

    return result;
}
