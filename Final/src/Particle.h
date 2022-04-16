//
// sueda
// November, 2014/ wood 16
//

#pragma once

#ifndef LAB471_PARTICLE_H_INCLUDED
#define LAB471_PARTICLE_H_INCLUDED

#include <vector>
#include <memory>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Program;
class Texture;

class Particle
{
public:
    Particle(vec3 pos);
    virtual ~Particle();
    void load(vec3 start);
    void rebirth(float time, vec3 start);
    void update(float time, float high, const glm::vec3 &g, const vec3 start);
    const vec3 &getPosition() const { return position; };
    const vec3 &getVelocity() const { return velocity; };
    const vec3 &getAcceleration() const { return acceleration; };
    const vec4 &getColor() const { return color; };
        
private:
    float mass;
    float damping; 
    vec3 position; 
    vec3 velocity; 
    vec3 acceleration;
    float lifespan;
    float timeEnd;
    float scale;
    vec4 color;
};

#endif // LAB471_PARTICLE_H_INCLUDED
