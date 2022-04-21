#ifndef OBJECT_H
#define OBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "model.h"
using namespace std;
using namespace glm;

/*
 * TODO
 *
 * Material data in constructor.
 *
 * Culling Radius
 * Just Width Radius for collisions
 *
 */

enum ShaderTypes {
    MATERIAL,
    TEXTURE
};

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shine;
};

class Object {
public:
    Model *model;
    Shader *shader;
    int shader_type;
    Material material;
    vec3 position;
    float angle;
    vec3 rotation;
    vec3 Scale;
    vec3 velocity;
    float height_radius;
    float width_radius;

    // TODO(Alex): Create a more robust system. This sucks.
    string MODEL_ID;
    string SHADER_ID;

    Object (Model *mod, Shader *sdr, int shad_t, vec3 pos, float agl, vec3 rot, 
	vec3 vel, float rad_h, float rad_w, vec3 scl, string m, string s)
    {
        this->model = mod;
        this->shader = sdr;
        this->shader_type = shad_t;
        this->position = pos;
        this->angle = agl;
        this->rotation = rot;
        this->Scale = scl;
        this->velocity = vel;
        this->height_radius = rad_h;
        this->width_radius = rad_w;
        this->material = {vec3(0.31f, 0.1f, 1.0f), vec3(0.31f, 0.1f, 1.0f), vec3(0.5f, 0.5f, 0.5f), 32.0f};
        this->MODEL_ID = m;
        this->SHADER_ID = s;
    }

    void Draw()
    {
        mat4 matrix = mat4(1.0f);
        mat4 pos = translate(mat4(1.0f), position);
        mat4 rot = rotate(mat4(1.0f), angle, rotation);
        mat4 scl = scale(mat4(1.0f), Scale);
        matrix = pos * rot * scl;
        
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
