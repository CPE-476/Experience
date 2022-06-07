#ifndef WATER_H
#define WATER_H

#include <glad/glad.h> 

#include "shader.h"
#include "camera.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;
using namespace glm;

struct Water
{
    unsigned int quadVAO, quadVBO, EBO;
    GLuint TextureID;
    unsigned int textureID;
    float height = -4.6f;
    vec4 color = vec4(0.2, 0.2, 1.0, 0.4);

    float partVertices[12] = {
            -0.5f, 0.0f, 0.5f,
            0.5f, 0.0f, 0.5f,
            -0.5f, 0.0f, -0.5f,
            0.5f, 0.0f, -0.5f,
        };

    int indices[6] = {
        0, 1, 2,
        3, 2, 1,
    };

   void Draw(Shader &shader, float delta)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(TextureID, 0);

        mat4 projection = camera.GetProjectionMatrix();
        mat4 view = camera.GetViewMatrix();

        shader.bind();
	{
            shader.setMat4("projection", projection);
            shader.setMat4("view", view);
            shader.setVec3("viewPos", camera.Position);
            shader.setVec4("color", color);

            mat4 model = mat4(1.0f);
            model =  translate(mat4(1.0f),  vec3(0.0f, height, 0.0f)) * scale(mat4(1.0f), 250.0f * vec3(1.0f, 1.0f, 1.0f));
            shader.setMat4("model", model);

            glBindVertexArray(quadVAO);
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(sizeof(indices)), GL_UNSIGNED_INT, 0);
	}
        shader.unbind();
        
    }

    void gpuSetup()
    {
        int width, height, nrComponents;
        unsigned char *data = stbi_load("../resources/testing/grass.jpg", &width, &height, &nrComponents, 0);
        glGenTextures(1, &textureID);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*12, &partVertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*6, &indices[0], GL_STATIC_DRAW);
    }
};

#endif
