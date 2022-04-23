// Author: Brett Hickman, Lucas Li, Alex Hartford
// Program: Base
// File: Object
// Date: April 2022

#ifndef OBJECT_H
#define OBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "model.h"
#include "manager.h"
using namespace std;
using namespace glm;

/*
 * TODO
 *
 * Material data in constructor.
 *
 * Culling Radius
 * Just Width Radius for collisions
 */

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shine;
};

class Object {
public:

    Manager *manager;
    int id;
    Material material;
    vec3 position;
    float angleX;
    float angleY;
    float angleZ;
    float scaleFactor;
    vec3 velocity;
    float height_radius;
    float width_radius;

    ID_Entry entry;
    Model *model;
    Shader *shader;
    int shader_type;


    Object (int id,
            vec3 pos, float agl_x, float agl_y, float agl_z, 
            vec3 vel, float rad_h, float rad_w, 
            float scl, Manager* m)
    {
        this->id = id;
        this->position = pos;
        this->angleX = agl_x;
        this->angleY = agl_y;
        this->angleZ = agl_z;
        this->scaleFactor = scl;
        this->velocity = vel;
        this->height_radius = rad_h;
        this->width_radius = rad_w;
        this->material = {vec3(0.9f, 0.9f, 0.9f), vec3(0.9f, 0.9f, 0.9f), vec3(0.9f, 0.9f, 0.9f), 5.0f};
        this->manager = m;
    }

    void Draw()
    {
        mat4 matrix = mat4(1.0f);
        mat4 pos = translate(mat4(1.0f), position);
        mat4 rotX = rotate(mat4(1.0f), angleX, vec3(1.0f, 0.0f, 0.0f));
        mat4 rotY = rotate(mat4(1.0f), angleY, vec3(0.0f, 1.0f, 0.0f));
        mat4 rotZ = rotate(mat4(1.0f), angleZ, vec3(0.0f, 0.0f, 1.0f));
        mat4 scl = scale(mat4(1.0f), scaleFactor * vec3(1.0f, 1.0f, 1.0f));
        matrix = pos * rotX * rotY * rotZ * scl;

        entry = manager->findbyId(id);
        model = entry.model;
        shader = entry.shader;
        shader_type = entry.shader_type;


        
        shader->setMat4("model", matrix);

        if(shader_type == MATERIAL)
        {
            shader->setVec3("material.ambient", material.ambient);
            shader->setVec3("material.diffuse", material.diffuse);
            shader->setVec3("material.specular", material.specular);
            shader->setFloat("material.shine", material.shine); 
        }

        model->Draw(*shader);
    }

    void Update(float deltaTime)
    {
        position += velocity * deltaTime;
    }
};

#endif
