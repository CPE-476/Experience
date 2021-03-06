// Author: Alex Hartford
// Program: Experience
// File: Mesh
// Date: April 2022

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

#include "shader.h"

using namespace std;

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture
{
    unsigned int id;
    string type;
    string path;
};

struct Mesh
{
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        setupMesh();
    }

    void SetTextureParams(Shader &shader)
    {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        unsigned int opacityNr = 1;

        shader.setInt("sample_diffuse1", 0);
        shader.setInt("sample_specular1", 0);
        shader.setInt("sample_normal1", 0);
        shader.setInt("sample_height1", 0);
        shader.setInt("sample_opacity1", 0);
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            string number;
            string name = textures[i].type;
            if(name == "diffuse")
            {
                number = to_string(diffuseNr++);
            }
            else if(name == "specular")
            {
                number = to_string(specularNr++);
            }
            else if(name == "normal")
            {
                number = to_string(normalNr++);
            }
            else if(name == "height")
            {
                number = to_string(heightNr++);
            }

            else if(name == "opacity")
            {
                number = to_string(opacityNr++);
            }

            shader.setInt(("texture_" + name + number).c_str(), i);
            shader.setInt(("sample_" + name + number).c_str(), 1);
            //cout << "texture_" + name + number << "\n";
            //cout << "sample_" + name + number << "\n";
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
    }

    void Draw(Shader &shader)
    {
        SetTextureParams(shader);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 
                static_cast<unsigned int>(indices.size()), 
                GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
    
    unsigned int VAO, VBO, EBO;

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                     &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));
        glBindVertexArray(0);
    }
};

#endif
