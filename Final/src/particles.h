// Author: Brett Hickman 
// Program: Experience
// File: Particle System
// Date: May 2022

#ifndef PARTICLES_H
#define PARTICLES_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "camera.h"

using namespace std;

class Particle {

public:
    vec3 pos, speed;
    vec4 color;
    float size;
    float life;
    int alive;
    float cameradistance;

    bool operator<(const Particle& that) const {
		return this->cameradistance > that.cameradistance;
    }

    Particle(vec3 pos, vec3 speed, float life, float size){
        this->pos = pos;
        this->speed = speed;
        this->life = life;
        this->size = size;
        this->alive = 0;
    }
};


class Emitter {
public:
    float lifeSpan;
    float gravity;
    vec3 startPosition, startVelocity;
    float radius, radiusTop, height;
    vec4 startColor, endColor;
    float startScale, endScale;
    int particleAmount;
    string path;
    int bugMode, fogMode;

    Emitter(string path, int partAmt, vec3 pos, float rad1, float rad2, 
	        float height, vec3 vel, float life, float grav, vec4 startCol, 
		vec4 endCol, float startScl, float endScl)
    {
        this->startPosition = pos;
        this->radius = rad1;
        this->radiusTop = rad2;
        this->height = height;
        this->startVelocity = vel;
        this->lifeSpan = life;
        this->gravity = grav;
        this->startColor = startCol;
        this->endColor = endCol;
        this->startScale = startScl;
        this->endScale = endScl;
        this->particleAmount = partAmt;
        this->path = path;
        this->bugMode = 0;
        this->fogMode = 0;
        this->Setup();
    }

    void Setup()
    {
        vec3 fogArray[4];
        for(int i=0;i<particleAmount;i++)
        {
            float r = radius * sqrt(randFloat(0, 1));
            float theta = randFloat(0, 1) * 2.0f * M_PI;
            vec3 startPos = vec3(startPosition.x + r * cos(theta), startPosition.y, startPosition.z + r * sin(theta));
            if(particleAmount == 20000)
                    {
                        int width = 256;
                        lifeSpan = randFloat(1.5f, 2.5f);
                        fogArray[0] = vec3(width/2, startPosition.y, -width/2 + randFloat(0, width));
                        fogArray[1] = vec3(-width/2, startPosition.y, -width/2 + randFloat(0, width));
                        fogArray[2] = vec3(-width/2+ randFloat(0, width), startPosition.y, width/2);
                        fogArray[3] = vec3(-width/2+ randFloat(0, width), startPosition.y, -width/2);
                        startPos = fogArray[(int)randFloat(0, 4)];
                    }
            vec3 vel = vec3(randFloat(-startVelocity.x, startVelocity.x), randFloat(startVelocity.y-(startVelocity.y/2), startVelocity.y), randFloat(-startVelocity.z, startVelocity.z));
            float rTop = radiusTop * sqrt(randFloat(0, 1));
            float thetaTop = randFloat(0, 1) * 2.0f * M_PI;
            vec3 V = vec3(startPos.x + rTop * cos(thetaTop), startPos.y+height, startPos.z + rTop * sin(thetaTop)) - startPos; 
            vec3 G = cross(vel, cross(V, vel));
            vel = (magnitude(vel)/magnitude(V)) * V;

            Particles.push_back(Particle(startPos, vel, lifeSpan, startScale));
            posOffsets[i] = startPos;
            colorOffsets[i] = startColor;
            scaleOffsets[i] = startScale;
        }
        gpuSetup();
    }

    void Draw(Shader &shader, float delta, int terrainWidth)
    {
        mat4 projection = camera.GetProjectionMatrix();
        mat4 view = camera.GetViewMatrix();
        mat4 model;

        shader.bind();
	{
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glUniform1i(TextureID, 0);

            // for billboarding
            shader.setVec3("CameraRight", view[0][0], view[1][0], view[2][0]);
            shader.setVec3("CameraUp", view[0][1], view[1][1], view[2][1]);

            shader.setMat4("Projection", projection);
            shader.setMat4("View", view);
            shader.setVec3("viewPos", camera.Position);

            model = mat4(1.0f);
            shader.setMat4("Model", model);

	    // Where the actual draw calls are, involves instancing.
            update(delta, terrainWidth);
	}
        shader.unbind();
    } 

private:
    static const int MaxParticles = 20000;
    vector<Particle> Particles;
    vec4 colorOffsets[MaxParticles];
    vec3 posOffsets[MaxParticles];
    float scaleOffsets[MaxParticles];
    GLuint TextureID;
    int spawned = 1;
    unsigned int textureID, counter;
    unsigned int instanceVBO, colorVBO, scaleVBO, quadVAO, quadVBO, EBO;

    float partVertices[12] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
        };

    int indices[6] = {
        0, 1, 2,
        3, 2, 1,
    };

    float randFloat(float Low, float High)
    {
        float r = rand() / (float) RAND_MAX;
        return (1.0f - r) * Low + r * High;
    }

    float magnitude(vec3 v)
    {
        int sum = v.x * v.x + v.y * v.y + v.z * v.z;
        return sqrt(sum);
    }

    void SortParticles()
    {
        sort(Particles.begin(), Particles.end());
    }

    void update(float delta, int width)
    {
        ++counter;
        width = width + 10;
        vec3 fogArray[4];
        for(int i=0;i<particleAmount;i++)
        {
            if(bugMode || fogMode)
                startPosition.y = randFloat(-7.5, 7.5);
            Particle& p = Particles[i];
            if(p.alive == 1)
            {
                if(p.life > 0)
                {
                    p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f;
                    p.pos += p.speed * (float)delta;
                    p.size = mix(endScale, startScale, p.life/lifeSpan);
                    p.color = mix(endColor, startColor, p.life/lifeSpan);
                    p.cameradistance = distance(p.pos, camera.Position);
                    posOffsets[i] = p.pos;
                    scaleOffsets[i] = p.size;
                    colorOffsets[i] = p.color;
                    p.life -= delta;
                }
                else
                {
                    p.life = lifeSpan;
                    if(fogMode)
                        p.life = randFloat(1.5f, 2.5f);
                    p.size = startScale;
                    float r = radius * sqrt(randFloat(0, 1));
                    float theta = randFloat(0, 1) * 2.0f * M_PI;
                    p.pos = vec3(startPosition.x + r * cos(theta), startPosition.y, startPosition.z + r * sin(theta));
                    if(fogMode)
                    {
                        p.pos = vec3(width/2*cos(theta), startPosition.y, width/2*sin(theta));
                    }
                    vec3 vel = vec3(randFloat(-startVelocity.x, startVelocity.x), randFloat(startVelocity.y-(startVelocity.y/2), startVelocity.y), randFloat(-startVelocity.z, startVelocity.z));
                    float rTop = radiusTop * sqrt(randFloat(0, 1));
                    float thetaTop = randFloat(0, 1) * 2.0f * M_PI;
                    vec3 V = vec3(startPosition.x + rTop * cos(thetaTop), startPosition.y+height, startPosition.z + rTop * sin(thetaTop)) - p.pos; 
                    vec3 G = cross(vel, cross(V, vel));
                    G = (magnitude(vel)/magnitude(V)) * V;
                    p.speed = G;
                    p.cameradistance = distance(p.pos, camera.Position);
                    posOffsets[i] = p.pos;
                    scaleOffsets[i] = p.size;
                    colorOffsets[i] = startColor;
                }
            }
            else if(counter % 2 == 1)
            {
                p.alive = 1;
                if(!fogMode)
                    break;
            }
            // if(fogMode)
            //     {
            //         for(int i = 0; i < 201; i ++)
            //         {
            //             ++counter;
            //             if(counter % 2 == 1)
            //             {
            //                 p.alive = 1;
            //                 break;
            //             }
            //         }
            //     }
        }

        SortParticles();

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * particleAmount, &posOffsets[0], GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * particleAmount, &colorOffsets[0], GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * particleAmount, &scaleOffsets[0], GL_STREAM_DRAW);

        glBindVertexArray(quadVAO);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(sizeof(indices)), GL_UNSIGNED_INT, 0, particleAmount);
    }

    void gpuSetup()
    {
        int width, height, nrComponents;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        glGenTextures(1, &textureID);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * MaxParticles, &posOffsets[0], GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &colorVBO);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * MaxParticles, &colorOffsets[0], GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &scaleVBO);
        glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MaxParticles, &scaleOffsets[0], GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

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

        glEnableVertexAttribArray(2); //set instance data
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); 
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(2, 1);

        glEnableVertexAttribArray(3); //set instance data
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO); 
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(4); //set instance data
        glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(4, 1);
    }
};

#endif
