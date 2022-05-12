// Author: Alex Hartford
// Program: Experience
// File: Terrain
// Date: April 2022

/* TODO(Alex)
 * Interpolate Values
 */

#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h> 

#include "shader.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;
using namespace glm;

struct Terrain
{
    float yScale = 16.0f;  // Desired Size
    Material material;

    float widthExtent = 128.0f;
    float heightExtent = 128.0f;

    string path;
    unsigned int VAO, VBO, EBO;

    vector<float> vertices;
    vector<unsigned int> indices;

    unsigned int num_strips;
    unsigned int num_tris_per_strip;

    int width, height;

    // [x][z]
    vector<vector<float>> pointsData;

    float lerp(float a, float b, float x)
    {
        return a + (b - a) * x;
    }

    /*
    float heightAt(float x, float z)
    {
        int leftSide = (int)x + height / 2;
        int rightSide = leftSide + 1;
        int bottomSide = (int)z + width / 2;
        int topSide = bottomSide + 1;

        float Xamount = (x + (float)height / 2.0f) - leftSide;
        float Zamount = (z + (float)width / 2.0f) - bottomSide;
        //cout << leftSide << " - " << rightSide << " | " << bottomSide << " - " << topSide << "\n";
        //cout << Xamount << "//" << Zamount << "\n";
        // L vs R
        float Xval = lerp(pointsData[leftSide][bottomSide], pointsData[rightSide][bottomSide], Xamount);
        float Zval = lerp(pointsData[leftSide][bottomSide], pointsData[leftSide][topSide], Zamount);
        cout << Xval << " || || " << Zval << "\n";

        float ret = Xval + Zval / 2.0f;
        cout << ret;
        
        return ret;
    }
    */

    // Takes an x and z value in world space.
    // Uses Barycentric Coordinates to return the height at a point.
    float heightAt(float x, float z)
    {
        float lam1, lam2, lam3;
        vec3 p1, p2, p3;
        int leftSide = (int)x + height / 2;
        int bottomSide = (int)z + width / 2;
        float xamt = (x - (int)x);
        float zamt = (z - (int)z);
        if(zamt - xamt < 0)  // On the right triangle.
        {
            p1 = vec3(leftSide, pointsData[leftSide][bottomSide], bottomSide);
            p2 = vec3(leftSide + 1, pointsData[leftSide+1][bottomSide], bottomSide);
            p3 = vec3(leftSide + 1, pointsData[leftSide+1][bottomSide+1], bottomSide + 1);
        }
        else // On the left triangle.
        {
            p1 = vec3(leftSide, pointsData[leftSide][bottomSide], bottomSide);
            p2 = vec3(leftSide, pointsData[leftSide][bottomSide+1], bottomSide + 1);
            p3 = vec3(leftSide + 1, pointsData[leftSide+1][bottomSide+1], bottomSide + 1); 
        }

        lam1 = ((p2.z - p3.z) * ((x + height / 2.0f) - p3.x) + (p3.x - p2.x) * ((z + width / 2.0f) - p3.z)) / ((p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z));
        lam2 = ((p3.z - p1.z) * ((x + height / 2.0f) - p3.x) + (p1.x - p3.x) * ((z + width / 2.0f) - p3.z)) / ((p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z));
        lam3 = 1 - lam1 - lam2;
        //cout << lam1 << "=" << lam2 << "=" << lam3 << "\n";

        float ret = p1.y * lam1 + p2.y * lam2 + p3.y * lam3;
        //cout << ret << "\n";

        return ret;
    }

    void init(string p, float y_scale, Material mat)
    {
        this->material = mat;
        vertices.clear();
        indices.clear();
        pointsData.clear();
        this->path = p;
        // Load Heightmap
        int nrChannels;
        unsigned char *data = stbi_load(p.c_str(), &width, &height, &nrChannels, 0);
        if(!data)
        {
            cout << "Unable to load Heightmap: " << path << "\n";
        }

        this->yScale = y_scale;
        float yScaleNormalized = yScale / height;

        this->widthExtent = width / 2;
        this->heightExtent = height / 2;

        for(int i = 0; i < height; ++i)
        {
            vector<float> row;
            for(int j = 0; j < width; ++j)
            {
                unsigned char* texel = data + (j + width * i) * nrChannels;
                unsigned char y = texel[0];
                float vy = (y * yScaleNormalized - yScale / 2);
                row.push_back(vy);
            }
            pointsData.push_back(row);
        }
        stbi_image_free(data);

        // Generate Indices
        for(int i = 0; i < height - 1; ++i)             // For each strip
        {
            for(int j = width - 1; j >= 0; --j)         // For each column (backwards for CCW data)
            {
                indices.push_back(j + width * (i + 0)); // For each side of the strip
                indices.push_back(j + width * (i + 1));
            }
        }

        // Generate Vertices
        for(int i = 0; i < height; ++i)
        {
            for(int j = 0; j < width; ++j)
            {
                float vx = (-height/2.0f + height * i / (float)height); // vx
                float vz = (-width/2.0f + width * j / (float)width);    // vz
                vertices.push_back(vx);
                vertices.push_back(pointsData[i][j]);
                vertices.push_back(vz);

                // Compute Normals, pack into array.
                // With Bounds Checking.
                vec3 v0, v1;
                if(j > 0 && j < pointsData[i].size() - 1)
                {
                    v0 = vec3(0, (pointsData[i][j+1] - pointsData[i][j-1]), 2);
                }
                else
                {
                    v0 = vec3(0, 1, 0);
                }
                if(i > 0 && i < pointsData.size() - 1)
                {
                    v1 = vec3(2, (pointsData[i+1][j] - pointsData[i-1][j]), 0);
                }
                else
                {
                    v1 = vec3(0, 1, 0);
                }
                vec3 normal = normalize(cross(v0, v1));
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);
            }
        }

        num_strips = height - 1;
        num_tris_per_strip = width * 2;

        // Buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // position attribute
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        glBindVertexArray(VAO);
        // Vertex data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normal data
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    }

    void Draw(Shader &shader, DirLight *dirLight)
    {
        shader.bind();
        {
            mat4 projection = camera.GetProjectionMatrix();
            mat4 view = camera.GetViewMatrix();
            shader.setMat4("projection", projection);
            shader.setMat4("view", view);
            mat4 model = mat4(1.0f);
            shader.setMat4("model", model);


            dirLight->Render(shader);
            shader.setVec3("viewPos", camera.Position);

            glBindVertexArray(VAO);
            for(unsigned int strip = 0; strip < num_strips; ++strip)
            {
                glDrawElements(GL_TRIANGLE_STRIP,
                               num_tris_per_strip, // Count of elements to be rendered.
                               GL_UNSIGNED_INT,    // Type of EBO data.
                               (void *)(sizeof(unsigned int) * num_tris_per_strip * strip)); // Pointer to starting index of indices.
            }
        }
        shader.unbind();
    }
};

#endif
