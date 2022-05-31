// Author: Alex Hartford
// Program: Base
// File: Light
// Date: April 2022

#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

#include "shader.h"

using namespace std;
using namespace glm;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    DirLight(vec3 dir, vec3 amb, vec3 dif, vec3 spec)
    {
        this->direction = dir;
        this->ambient = amb;
        this->diffuse = dif;
        this->specular = spec;
    }

    void Render(Shader &shader)
    {
        shader.setVec3("dirLight.direction", direction);
        shader.setVec3("dirLight.ambient", ambient);
        shader.setVec3("dirLight.diffuse", diffuse);
        shader.setVec3("dirLight.specular", specular);

        shader.setFloat("shine", 32.0f);
    }
};


struct Light
{
    vec3  position;

    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    vec3  color;

    float constant;
    float linear;
    float quadratic;

    Light(vec3 pos, vec3 amb, vec3 dif, vec3 spec,
          float con, float lin, float quad)
    {
        this->position = pos;
        this->ambient = amb;
        this->diffuse = dif;
        this->specular = spec;
        this->constant = con;
        this->linear = lin;
        this->quadratic = quad;
        this->color = vec3(100.0f);
    }

    void Render(Shader &shader, int i)
    {
        shader.setVec3("pointLights[" + to_string(i) + "].position", position);
        shader.setVec3("pointLights[" + to_string(i) + "].ambient", ambient);
        shader.setVec3("pointLights[" + to_string(i) + "].diffuse", diffuse);
        shader.setVec3("pointLights[" + to_string(i) + "].specular", specular);
        shader.setFloat("pointLights[" + to_string(i) + "].constant", constant);
        shader.setFloat("pointLights[" + to_string(i) + "].linear", linear);
        shader.setFloat("pointLights[" + to_string(i) + "].quadratic", quadratic);
    }
};

#endif
