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
        assert(0);  // TODO
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

    //std::cout << meshToAdd.material_ids.size() << ' ' << textures.size() << '\n';
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

void DrawWater(Mesh& water, shader_t& shader) {
    glActiveTexture(GL_TEXTURE0);
    shader.set_uniform("water_texture", 0);
    glBindTexture(GL_TEXTURE_2D, water.textures[0].id);

    glActiveTexture(GL_TEXTURE0 + 1);
    shader.set_uniform("water_normal", 1);
    glBindTexture(GL_TEXTURE_2D, water.textures[1].id);

    glActiveTexture(GL_TEXTURE0 + 2);
    shader.set_uniform("water_dudv", 2);
    glBindTexture(GL_TEXTURE_2D, water.textures[2].id);

    glBindVertexArray(water.MeshVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

Texture LoadTileTexture(const std::string& path) {
    Texture texture;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

void LoadWater(Mesh& water, const std::string& texture_path,
               const std::string& normal_path,
               const std::string& dudv_path,
               float scale,
               float density) {
    float planeVertices[] = {
            scale, 0.0f, scale,
            0.0f, 0.0f,
            scale, 0.0f,  -scale,
            0.0f, density,
            -scale, 0.0f, -scale,
            density, density,
            -scale, 0.0f,  scale,
            density, 0.0f
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
    shader.set_uniform("terrain_texture", 0);
    glBindTexture(GL_TEXTURE_2D, model.mesh.textures[0].id);

//    glActiveTexture(GL_TEXTURE0 + 1);
//    shader.set_uniform("terrain_normal", 1);
//    glBindTexture(GL_TEXTURE_2D, model.mesh.textures[1].id);

    glBindVertexArray(model.mesh.MeshVAO);
    glDrawElements(GL_TRIANGLES, model.mesh.IndexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void LoadLandscape(Landscape& landscape,
                   const std::string height_path,
                   const std::string texture_path,
                   const std::string normal_path,
                   float heightCoefficient,
                   int texture_density) {
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
            float i = (float)hh / height  - 1;
            float j = (float)ww / width - 1;
            float shiftW = 1.0 / width;
            float shiftH = 1.0 / height;

            int ii = hh;
            int jj = ww;

            // hh % texture_density / 4.0
            //(ww - 1) % texture_density
            // ww % texture_density
            //(ww + 1) % texture_density

            float density_f = (float)texture_density;

            float current[] = {
                    i, landscape.heightMap[ii][jj + 1], j + shiftW,
                    (hh % texture_density) / density_f, ((ww + 1) % texture_density) / density_f,
                    i + shiftH, landscape.heightMap[ii + 1][jj + 1], j + shiftW,
                    ((hh + 1) % texture_density) / density_f, ((ww + 1) % texture_density) / density_f,
                    i, landscape.heightMap[ii][jj], j,
                    (hh % texture_density) / density_f, (ww % texture_density) / density_f,
                    i + shiftH, landscape.heightMap[ii + 1][jj], j,
                    ((hh + 1) % texture_density) / density_f, (ww % texture_density) / density_f,

                    i + shiftH, landscape.heightMap[ii + 1][jj], j,
                    ((hh + 1) % texture_density) / density_f, (ww % texture_density) / density_f,
                    i + shiftH, landscape.heightMap[ii + 1][jj - 1], j - shiftW,
                    ((hh + 1) % texture_density) / density_f, ((ww - 1) % texture_density) / density_f,
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
                    ((hh - 1) % texture_density) / density_f, ((ww + 1) % texture_density) / density_f,
                    i, landscape.heightMap[ii][jj], j,
                    (hh % texture_density) / density_f, (ww % texture_density) / density_f,
                    i, landscape.heightMap[ii][jj + 1], j + shiftW,
                    (hh % texture_density) / density_f, ((ww + 1) % texture_density) / density_f
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
    landscape.mesh.textures.push_back(LoadTileTexture(texture_path));
   // landscape.mesh.textures.push_back(LoadTileTexture(normal_path));
}