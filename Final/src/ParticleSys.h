#ifndef PARTICLESYS_H
#define PARTICLESYS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "particle.h"
#include "camera.h"

using namespace std;

class ParticleSys {

public:
    float lifeSpan;
    vec3 startPosition;
    vec4 startColor, endColor;
    float startScale, endScale;
    int particleAmount;

    ParticleSys(int partAmt, vec3 pos, float life, vec4 startCol, vec4 endCol, float startScl, float endScl)
    {
        this->startPosition = pos;
        this->lifeSpan = life;
        this->startColor = startCol;
        this->endColor = endCol;
        this->startScale = startScl;
        this->endScale = endScl;
        this->particleAmount = partAmt;
    }

    void Setup(Shader &particleShader, float partVertices[12], int indices[6])
    {
        init();
        gpuSetup(particleShader, partVertices, indices);
    }

    void Draw(float delta, Camera &camera)
    {
        update(delta, camera);
    }
    

private:
    static const int MaxParticles = 100000;
    vector<Particle> Particles;
    vec4 colorOffsets[MaxParticles];
    vec3 posOffsets[MaxParticles];
    float scaleOffsets[MaxParticles];
    float partVerts[12];
    int indies[6];
    unsigned int instanceVBO, colorVBO, scaleVBO, quadVAO, quadVBO, EBO;
    

    float randFloat(float Low, float High)
    {
        float r = rand() / (float) RAND_MAX;
        return (1.0f - r) * Low + r * High;
    }

    void SortParticles(){
        std::sort(Particles.begin(), Particles.end());
    }

    void init()
    {
        for(int i=0;i<particleAmount;i++)
        {
            vec3 vel = vec3(randFloat(-2, 2), randFloat(2, 3), randFloat(-2, 2));
            Particles.push_back(Particle(startPosition, vel, lifeSpan, startScale));
            posOffsets[i] = startPosition;
            colorOffsets[i] = startColor;
            scaleOffsets[i] = startScale;
        }
    }

    void update(float delta, Camera &camera)
    {
        for(int i=0;i<particleAmount;i++)
        {
            Particle& p = Particles[i];
            if(p.life > 0)
            {
                p.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)delta * 0.5f;
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
                p.life = randFloat(0, lifeSpan);
                p.pos = startPosition;
                vec3 vel = vec3(randFloat(-2, 2), randFloat(5, 10), randFloat(-2, 2));
                p.speed = vel;
                p.cameradistance = distance(p.pos, camera.Position);
                posOffsets[i] = p.pos;
            }
        }

        SortParticles();

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * MaxParticles, &posOffsets[0], GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * MaxParticles, &colorOffsets[0], GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MaxParticles, &scaleOffsets[0], GL_STREAM_DRAW);

        glBindVertexArray(quadVAO);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(sizeof(indies)), GL_UNSIGNED_INT, 0, particleAmount);
        }

    void gpuSetup(Shader &particleShader, float partVertices[12], int indices[6])
    {

        for(int i =0;i<12;i++)
        {
            partVerts[i] = partVertices[i];
        }

        for(int i =0;i<6;i++)
        {
            indies[i] = indices[i];
        }

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
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*12, &partVerts[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*6, &indies[0], GL_STATIC_DRAW);

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