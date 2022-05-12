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

struct Boundary
{
    unsigned int VBO, VAO, EBO;
    float vertices[24] = {
         // positions         // colors
         0.5f,  0.5f,  0.0f,   1.0f, 0.0f, 0.0f,  // top right
         0.5f, -0.5f,  0.0f,   0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f,  0.0f,   0.0f, 0.0f, 1.0f,  // bottom left
        -0.5f,  0.5f,  0.0f,   1.0f, 1.0f, 0.0f,  // top left
    };
    unsigned int indices[6] = {
        0, 2, 1, // first triangle
        0, 3, 2  // second triangle
    };

    void Draw(Shader &shader, int size)
    {
        shader.bind();
        {
            mat4 projection = camera.GetProjectionMatrix();
            mat4 view = camera.GetViewMatrix();
            shader.setMat4("projection", projection);
            shader.setMat4("view", view);
            shader.setVec3("viewPos", camera.Position);
            mat4 model = mat4(1.0f);

            model = scale(model, vec3((float)size, 15.0f, 1.0f));
            model = translate(model, vec3(0.0f, 0.0f, -((float)size / 2.0f)));
            shader.setMat4("model", model); 

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            model = mat4(1.0f);
            model = rotate(model, radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
            model = scale(model, vec3((float)size, 15.0f, 1.0f));
            model = translate(model, vec3(0.0f, 0.0f, -((float)size / 2.0f)));
            shader.setMat4("model", model); 

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            model = mat4(1.0f);
            model = rotate(model, radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
            model = scale(model, vec3((float)size, 15.0f, 1.0f));
            model = translate(model, vec3(0.0f, 0.0f, -((float)size / 2.0f)));
            shader.setMat4("model", model); 

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            model = mat4(1.0f);
            model = rotate(model, radians(270.0f), vec3(0.0f, 1.0f, 0.0f));
            model = scale(model, vec3((float)size, 15.0f, 1.0f));
            model = translate(model, vec3(0.0f, 0.0f, -((float)size / 2.0f)));
            shader.setMat4("model", model); 

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        shader.unbind();
    }

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
};

struct Transition
{
    unsigned int VBO, VAO, EBO;

    float vertices[24] = {
         // positions         // colors
         1.0f,  1.0f,  0.0f,   1.0f, 0.0f, 0.0f,  // top right
         1.0f, -1.0f,  0.0f,   0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,  // bottom left
        -1.0f,  1.0f,  0.0f,   1.0f, 1.0f, 0.0f,  // top left
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
    int counter = 0;
    int speed = 100;
    bool active = false;

    void Draw(Shader &shader)
    {
        shader.bind();
        {
            shader.setFloat("amount", sin((float)counter / this->speed));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        shader.unbind();
    }
};

#endif
