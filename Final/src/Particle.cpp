//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float Low, float High)
{
    float r = rand() / (float) RAND_MAX;
    return (1.0f - r) * Low + r * High;
}

Particle::Particle(vec3 start) :
    mass(1.0f),
    damping(0.0f),
    position(start),
    velocity(0.0f, 0.0f, 0.0f),
    acceleration(0.0f, -0.001, 0.0),
    lifespan(1.0f),
    timeEnd(3.0f),
    scale(1.0f),
    color(randFloat(0.0f, 1.0f), randFloat(0.0f, 1.0f), randFloat(0.0f, 1.0f), randFloat(0.0f, 1.0f))
{
}

Particle::~Particle()
{
}

void Particle::load(vec3 start)
{
        // Random initialization
        rebirth(0.0f, start);
}

/* all particles born at the origin */
void Particle::rebirth(float time, vec3 start)
{
        mass = 1.0f;
        damping = randFloat(0.0f, 0.2f);
        position = start;
        velocity.x = randFloat(-0.27f, 0.3f);
        velocity.y = randFloat(-0.1f, 0.9f);
        velocity.z = randFloat(-0.3f, 0.27f);
        lifespan = randFloat(1.0f, 2.0f); 
        timeEnd = time + lifespan;
        scale = randFloat(0.2, 1.0f);
        color.r = randFloat(0.5f, 0.8f);
        color.g = randFloat(0.0f, 0.5f);
        color.b = randFloat(0.0f, 0.1f);
        color.a = 1.0f;
}

void Particle::update(float time, float h, const vec3 &g, const vec3 start)
{
    if(time > timeEnd) {
	rebirth(time, start);
    }

    //very simple update
    position += 2 * h * velocity;
    velocity += acceleration;

    //To do - how do you want to update the forces?
    color.a = (timeEnd/time)/lifespan;
}
