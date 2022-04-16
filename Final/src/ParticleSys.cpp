#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include "ParticleSys.h"
#include "GLSL.h"

using namespace std;

particleSys::particleSys(vec3 source) {
    numParticles = 300;	
    time = 0.0f;
    h = 0.01f;
    gravity = vec3(0.0f, -0.098, 0.0f);
    start = source;
    particleCamera = glm::mat4(1.0);
}

void particleSys::gpuSetup() {
    //go through all the particles and update the CPU buffer.
    for (int i = 0; i < numParticles; i++)
    {
        points[i*3+0] = start.x; 
        points[i*3+1] = start.y; 
        points[i*3+2] = start.z; 

        auto particle = make_shared<Particle>(start);
        particles.push_back(particle);
        particle->load(start);  
    }

    //generate the VAO
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    //generate vertex buffer to hand off to OGL - using instancing
    glGenBuffers(1, &vertexBufferObject);
    //set the current state to focus on our vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    //actually memcopy the data - only do this once
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points[0], GL_STREAM_DRAW);

    //generate vertex buffer to hand off to OGL - using instancing
    glGenBuffers(1, &colorBufferObject);
    //set the current state to focus on our vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferObject);
    //actually memcopy the data - only do this once
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), &pointColors[0], GL_STREAM_DRAW);

    assert(glGetError() == GL_NO_ERROR);	
}

void particleSys::reset()
{
    for (int i = 0; i < numParticles; i++)
    {
        particles[i]->load(start);
    }
}

void particleSys::drawMe(std::shared_ptr<Program> prog)
{
    glBindVertexArray(vertexArrayObject);

    int c_pos = prog->getAttribute("ParticleColor");
    GLSL::enableVertexAttribArray(c_pos);
    //std::cout << "Any Gl errors1: " << glGetError() << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferObject);
    //std::cout << "Any Gl errors2: " << glGetError() << std::endl;
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numParticles * 4, pointColors);

    int h_pos = prog->getAttribute("VertexPosition");
    GLSL::enableVertexAttribArray(h_pos);
    //std::cout << "Any Gl errors1: " << glGetError() << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    //std::cout << "Any Gl errors2: " << glGetError() << std::endl;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);

    // Draw the points!
    glDrawArraysInstanced(GL_POINTS, 0, 1, numParticles);

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);

    glDisableVertexAttribArray(0);
}

void particleSys::update()
{
    vec3 pos;
    vec4 col;

    //update the particles
    for(auto particle : particles)
    {
        particle->update(time, h, gravity, start);
    }
    time += h;

    // Sort the particles by Z
    //be sure that camera matrix is updated prior to this update
    vec3 s, t, sk;
    vec4 p;
    quat r;
    glm::decompose(particleCamera, s, r, t, sk, p);
    sorter.C = glm::toMat4(r); 
    sort(particles.begin(), particles.end(), sorter);

    //go through all the particles and update the CPU buffer
    for (int i = 0; i < numParticles; i++) {
        pos = particles[i]->getPosition() - particles[i]->getVelocity();
        col = particles[i]->getColor();
        
        points[i*3+0] = pos.x; 
        points[i*3+1] = pos.y; 
        points[i*3+2] = pos.z; 
        pointColors[i*4+0] = col.x; 
        pointColors[i*4+1] = col.y; 
        pointColors[i*4+2] = col.z;
        pointColors[i*4+3] = col.a;
    } 

    //update the GPU data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numParticles * 3, points);
    // colors
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numParticles * 4, pointColors);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
