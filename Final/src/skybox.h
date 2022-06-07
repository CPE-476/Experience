// Author: Alex Hartford
// Program: Base
// File: Skybox
// Date: April 2022

#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h> 

#include "shader.h"

#include <string>
#include <vector>
#include <iostream>
using namespace std;

unsigned int loadCubemap(vector<string> faces);

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

struct Skybox
{
    string dir;
    float timer = 0.0f;

    unsigned int VAO, VBO;
    unsigned int textureID;

    void init(string d, bool png = false)
    {
        this->dir = d;
        vector<string> paths;
        paths.push_back(dir + (png ? "right.png" : "right.jpg"));
        paths.push_back(dir + (png ? "left.png" : "left.jpg"));
        paths.push_back(dir + (png ? "top.png" : "top.jpg"));
        paths.push_back(dir + (png ? "bottom.png" : "bottom.jpg"));
        paths.push_back(dir + (png ? "front.png" : "front.jpg"));
        paths.push_back(dir + (png ? "back.png" : "back.jpg"));

        this->textureID = loadCubemap(paths);
        setup();
    }

    void Draw(Shader &shader, float maskAmount)
    {
        timer += 0.0002;
        glDepthFunc(GL_LEQUAL);
        shader.bind();
	mat4 projection = camera.GetProjectionMatrix();

        mat4 rotY = rotate(mat4(1.0f), timer, vec3(0.0f, 1.0f, 0.0f));
        // Remove Translation part of matrix.
        mat4 view = mat4(mat3(camera.GetViewMatrix())) * rotY;
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        shader.setFloat("threshold", gBloomThreshold);

        shader.setFloat("maskAmount", maskAmount);

        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        shader.unbind();
        glDepthFunc(GL_LESS);
    }
    
    void setup()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }
};

unsigned int loadCubemap(vector<string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for(unsigned int i = 0; i < faces.size(); ++i)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if(data)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap failed to load at path: " << faces[i] << "\n";
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

    return textureID;
}

#endif
