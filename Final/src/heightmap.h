// Author: Alex Hartford
// Program: Base
// File: Heightmap
// Date: April 2022

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <glad/glad.h> 

#include "shader.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;
using namespace glm;

// Tweak these values for different terrain types.
const float Y_SCALE = 64.0f / 256.0f;
const float Y_SHIFT = 16.0f;

class Heightmap
{
public:
    Heightmap()
    {
    }

    void init(string dir)
    {
        this->setup();
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

    unsigned int height;
    unsigned int width;

private: 
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;

    vector<float> vertices;
    vector<unsigned int> indices;

    unsigned int num_strips;
    unsigned int num_tris_per_strip;

    void setup()
    {
        // Load Heightmap
        int width, height, nrChannels;
        unsigned char *data = stbi_load("../resources/testing/heightmap.png", &width, &height, &nrChannels, 0);

        this->width = width;
        this->height = width;
        
        // Generate Vertices
        for(int i = 0; i < height; ++i)
        {
            for(int j = 0; j < width; ++j)
            {
                unsigned char* texel = data + (j + width * i) * nrChannels;
                unsigned char y = texel[0];

                float vx = (-height/2.0f + height * i / (float)height); // vx
                //float vy = ((int)y * Y_SCALE - Y_SHIFT);                // vy
                float vy = ((float)y * Y_SCALE);                // vy
                float vz = (-width / 2.0f + width * j / (float)width);  // vz
                vertices.push_back(vx);
                vertices.push_back(vy);
                vertices.push_back(vz);
            }
        }
        stbi_image_free(data);

        // Generate Indices (--i/--j for right side up)
        for(int i = height; i > 0; --i)
        {
            for(int j = width; j > 0; --j)
            {
                for(int k = 0; k < 2; ++k)
                {
                    indices.push_back(j + width * (i + k));
                }
            }
        }

        num_strips = height - 1;
        num_tris_per_strip = width * 2;

        // Buffers
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);
    }
};

#endif
