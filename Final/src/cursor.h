// Author: Alex Hartford
// Program: Experience
// File: Cursor Class
// Date: June 2022

#ifndef CURSOR_H
#define CURSOR_H

#include <glad/glad.h>

#include "shader.h"

using namespace std;
using namespace glm;

struct Cursor
{
    unsigned int VBO, VAO, EBO;
    vec3 color;

    float vertices[24] = {
         // positions         
         0.5f,  0.75f,  0.0f, // top right
         0.5f, -0.75f,  0.0f, // bottom right
        -0.5f, -0.75f,  0.0f, // bottom left
        -0.5f,  0.75f,  0.0f  // top left
    };
    unsigned int indices[6] = {
        0, 2, 1, // first triangle
        0, 3, 2  // second triangle
    };


    void init(vec3 col)
    {
        this->color = col;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void Draw(Shader &shader)
    {
        shader.bind();
        {
            shader.setVec3("color", color);
            
            mat4 transform = mat4(1.0f);
            transform = scale(transform, vec3(0.01));
            shader.setMat4("transform", transform);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        shader.unbind();
    }
};

#endif
