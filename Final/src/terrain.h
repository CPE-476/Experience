// Author: Alex Hartford
// Program: Experience
// File: Terrain
// Date: April 2022

#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h> 

#include "shader.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;
using namespace glm;

// Tweak these values for different terrain types.
const float Y_SCALE = 64.0f / 256.0f; // Desired Size / Original Image size.
const float Y_SHIFT = 64.0f;          // Height of Mesh


class Terrain
{
public:
    Terrain()
    {
    }

    void computeNormals()
    {
        normals.resize(vertices.size(), 0);
        for(int i = 0; i < indices.size(); i = i + 3)
        {
            vec3 v0, v1, v2;
            v0.x = vertices[3 * indices[i]];
            v0.y = vertices[3 * indices[i] + 1];
            v0.z = vertices[3 * indices[i] + 2];
            v1.x = vertices[3 * indices[i+1]];
            v1.y = vertices[3 * indices[i+1] + 1];
            v1.z = vertices[3 * indices[i+1] + 2];
            v2.x = vertices[3 * indices[i+2]];
            v2.y = vertices[3 * indices[i+2] + 1];
            v2.z = vertices[3 * indices[i+2] + 2];

            vec3 u = v1 - v0;
            vec3 v = v2 - v0;
            vec3 faceNormal = cross(u, v);

            // Format: [3 * vertex number + x/y/z offset]
            normals[3 * indices[i]] += faceNormal.x;
            normals[3 * indices[i] + 1] += faceNormal.y;
            normals[3 * indices[i] + 2] += faceNormal.z;
            normals[3 * indices[i+1]] += faceNormal.x;
            normals[3 * indices[i+1] + 1] += faceNormal.y;
            normals[3 * indices[i+1] + 2] += faceNormal.z;
            normals[3 * indices[i+2]] += faceNormal.x;
            normals[3 * indices[i+2] + 1] += faceNormal.y;
            normals[3 * indices[i+2] + 2] += faceNormal.z;
        }

	/*
        // NOTE(Alex): Normalizing the Normals.
        for(int i = 0; i < normals.size(); i = i + 3) {
            int x = normals[3 * i];
            int y = normals[3 * i + 1];
            int z = normals[3 * i + 2];
            normals[3 * i] = -1 * (x / sqrt(x * x + y * y + z * z));
            normals[3 * i + 1] = -1 * (y / sqrt(x * x + y * y + z * z));
            normals[3 * i + 2] = -1 * (z / sqrt(x * x + y * y + z * z));
        }
	*/
    }

    void init(string path)
    {
        // Load Heightmap
        int width, height, nrChannels;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
 
        // Generate Vertices
        for(int i = 0; i < height; ++i)
        {
            for(int j = 0; j < width; ++j)
            {
                unsigned char* texel = data + (j + width * i) * nrChannels;
                unsigned char y = texel[0];

                float vx = (-height/2.0f + height * i / (float)height); // vx
                float vy = ((int)y * Y_SCALE - Y_SHIFT);                // vy
                float vz = (-width / 2.0f + width * j / (float)width);  // vz
                vertices.push_back(vx);
                vertices.push_back(vy);
                vertices.push_back(vz);
            }
        }
        stbi_image_free(data);

        // Generate Indices (--i/--j for right side up)
        for(int i = height - 2; i > -1; --i)
        {
            for(int j = width - 1; j > -1; --j)
            {
                for(int k = 0; k < 2; ++k)
                {
                    indices.push_back(j + width * (i + k));
                }
            }
        }

        num_strips = height - 1;
        num_tris_per_strip = width * 2;

        computeNormals();

        // Buffers
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // position attribute
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        /*
        // normal attribute
        glGenBuffers(1, &NBO);
        glBindBuffer(GL_ARRAY_BUFFER, NBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        */

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);
    }

    void Draw(Shader &shader, Camera &camera)
    {
        shader.bind();
        {
            mat4 projection = perspective(radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
            // Remove Translation part of matrix.
            mat4 view = camera.GetViewMatrix();
            shader.setMat4("projection", projection);
            shader.setMat4("view", view);
            mat4 model = mat4(1.0f);
            shader.setMat4("model", model);

            glBindVertexArray(VAO);
            for(unsigned int strip = 0; strip < num_strips; ++strip)
            {
                glDrawElements(GL_TRIANGLE_STRIP,
                               num_tris_per_strip,
                               GL_UNSIGNED_INT,
                               (void *)(sizeof(unsigned int) * num_tris_per_strip * strip));
            }
        }
        shader.unbind();
    }

private: 
    unsigned int VAO, VBO, NBO, EBO;

    vector<float> vertices;
    vector<float> normals;
    vector<unsigned int> indices;

    unsigned int num_strips;
    unsigned int num_tris_per_strip;
};

#endif
