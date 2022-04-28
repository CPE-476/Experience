#ifndef OBJECT_H
#define OBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "model.h"
using namespace std;
using namespace glm;

class Object {

public:
    Model *model;
    Shader *shader;
    vec3 position;
    float angle;
    vec3 rotation;
    vec3 Scale;
    vec3 velocity;
    float height_radius;
    float width_radius;

    Object (Model *mod, Shader *sdr, vec3 pos, float agl, vec3 rot, 
	vec3 vel, float rad_h, float rad_w, vec3 scl)
    {
        this->model = mod;
        this->shader = sdr;
        this->position = pos;
        this->angle = agl;
        this->rotation = rot;
        this->Scale = scl;
        this->velocity = vel;
        this->height_radius = rad_h;
        this->width_radius = rad_w;
    }

    void Draw()
    {
        mat4 matrix = mat4(1.0f);
        mat4 pos = translate(mat4(1.0f), position);
        mat4 rot = rotate(mat4(1.0f), angle, rotation);
        mat4 scl = scale(mat4(1.0f), Scale);
        matrix = pos * rot * scl;
        
        shader->setMat4("model", matrix);
        model->Draw(*shader);
        //cout << "drawing skull" << endl;
    }

    void Update(float deltaTime)
    {
        position += velocity * deltaTime;
    }
};

#endif
