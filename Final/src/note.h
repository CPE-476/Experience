// Author: Alex Hartford
// Program: Experience
// File: Note Class
// Date: May 2022

#ifndef NOTE_H
#define NOTE_H

#include <glad/glad.h>

#include "shader.h"
#include "spline.h"
using namespace std;
using namespace glm;

/* TODO (Alex):
 * Animal Dialogue
 *  Spline
 *  Sound
 */

struct Note
{
    unsigned int VBO, VAO, EBO;
    unsigned int noteTexture;

    float vertices[32] = {
         // positions        // texture coords
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f  // top left
    };
    unsigned int indices[6] = {
        0, 2, 1, // first triangle
        0, 3, 2  // second triangle
    };

    Note(string path)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        glGenTextures(1, &noteTexture);
        glBindTexture(GL_TEXTURE_2D, noteTexture); 
         // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load image, create texture and generate mipmaps
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
    }
    float counter = 0;

    float scl = 1.0f;
    vec3 pos  = vec3(0.0f, -0.5f, 0.0f);

    void Draw(Shader &shader)
    {
        shader.bind();
        {
            shader.setFloat("noteTexture", noteTexture);
            shader.setFloat("amount", sin(counter / 100.0f));
            mat4 transform = mat4(1.0f);
            transform = scale(transform, vec3(scl));
            transform = translate(transform, pos);
            shader.setMat4("transform", transform);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, noteTexture);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        shader.unbind();
    }

    void DrawSmall(Shader &shader, int xIndex, int yIndex)
    {
        shader.bind();
        {
            shader.setFloat("noteTexture", noteTexture);
            shader.setFloat("amount", 1.0f);
            mat4 transform = mat4(1.0f);
            transform = scale(transform, vec3(0.5));
            transform = translate(transform, vec3((float)xIndex - 1.5, 1.5 - (float)yIndex, 0.0f));
            shader.setMat4("transform", transform);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, noteTexture);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        shader.unbind();
    }

    // TODO(Alex): Tweak these values so you can edit spline dynamically.
    void Update(FloatSpline *fspline)
    {
        if(!pauseNote)
        {
            counter += 2.0f;
        }
        if(counter == 158)
        {
            counter += 2.0f;
            pauseNote = true;
        }
        if(counter >= 314)
        {
            drawNote = false;
            counter = 0;
        }
    }
};

#endif
