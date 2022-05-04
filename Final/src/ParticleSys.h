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
    Shader particleShader;
    char const *path;

    ParticleSys(Shader &particleShader, char const* path, int partAmt, vec3 pos, float life, vec4 startCol, vec4 endCol, float startScl, float endScl)
    {
        this->startPosition = pos;
        this->lifeSpan = life;
        this->startColor = startCol;
        this->endColor = endCol;
        this->startScale = startScl;
        this->endScale = endScl;
        this->particleAmount = partAmt;
        this->particleShader = particleShader;
        this->path = path;
        this->Setup(particleShader);
    }

    void Setup(Shader &particleShader)
    {
        init();
        gpuSetup(particleShader);
    }

    void Draw(float delta, Camera &camera)
    {
        mat4 projection = perspective(radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
        mat4 view = camera.GetViewMatrix();
        mat4 model;

        particleShader.bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glUniform1i(TextureID, 0);

            // for billboarding
            particleShader.setVec3("CameraRight", view[0][0], view[1][0], view[2][0]);
            particleShader.setVec3("CameraUp", view[0][1], view[1][1], view[2][1]);

            particleShader.setMat4("Projection", projection);
            particleShader.setMat4("View", view);
            particleShader.setVec3("viewPos", camera.Position);

            //lightSystem.Render(m.shaders.particleShader);

            model = mat4(1.0f);
            particleShader.setMat4("Model", model);

            update(delta, camera);
        particleShader.unbind();
    }
    

private:
    static const int MaxParticles = 10000;
    vector<Particle> Particles;
    vec4 colorOffsets[MaxParticles];
    vec3 posOffsets[MaxParticles];
    float scaleOffsets[MaxParticles];
    GLuint TextureID;
    unsigned int textureID;
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

    void SortParticles()
    {
        sort(Particles.begin(), Particles.end());
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
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(sizeof(indices)), GL_UNSIGNED_INT, 0, particleAmount);
        }

    void gpuSetup(Shader &particleShader)
    {

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
        glGenTextures(1, &textureID);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        TextureID  = glGetUniformLocation(particleShader.ID, "myTex");
        glUniform1i(TextureID, 0);

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