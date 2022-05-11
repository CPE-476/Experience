// Author: Alex Hartford
// Program: Experience
// File: Transition Class
// Date: May 2022

#ifndef TRANS_H
#define TRANS_H

#include <glad/glad.h>

#include "shader.h"
using namespace std;
using namespace glm;

struct Transition
{
    unsigned int VBO, VAO, EBO;

    float vertices[24] = {
         // positions         // colors
         1.0f,  1.0f,  0.0f,   1.0f, 0.0f, 0.0f,  // top right
         1.0f, -1.0f,  0.0f,   0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f,  1.0f,  0.0f,   0.0f, 0.0f, 1.0f,  // bottom left
        -1.0f, -1.0f,  0.0f,   1.0f, 1.0f, 0.0f,  // top left
    };
    unsigned int indices[6] = {
        0, 2, 1, // first triangle
        0, 3, 2  // second triangle
    };

    void init()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void Draw(Shader &shader)
    {
        shader.bind();
        {
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        shader.unbind();
    }
};

#endif
