#ifndef PARTICLE_H
#define PARTICLE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
using namespace std;
using namespace glm;

class Particle {

public:
    vec3 pos, speed;
    vec4 color;
    float size, angle, weight;
    float life;
    int alive;
    float cameradistance;

    bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
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

#endif