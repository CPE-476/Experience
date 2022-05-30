// Author: Alex Hartford
// Program: Experience
// File: Sun Class
// Date: May 2022

#ifndef SUN_H
#define SUN_H

#include <glad/glad.h>

#include "shader.h"
#include "model.h"
#include "spline.h"


using namespace std;
using namespace glm;

struct Sun
{
    Model *mesh;
    float scale_factor = 1.0f;
    vec3 position = vec3(0.0f);
    vec3 color = vec3(1.0f, 0.0f, 0.0f);

    DirLight dirLight = DirLight(vec3(0.0f, 0.0f, 1.0f),  // Direction
                                 vec3(0.4f, 0.2f, 0.2f),  // Ambient
                                 vec3(0.8f, 0.6f, 0.6f),  // Diffuse
                                 vec3(0.5f, 0.3f, 0.3f)); // Specular

    Sun(Model *mod)
    {
        this->mesh = mod;
    }

    void init(float scl, vec3 pos, vec3 col,
              vec3 amb, vec3 diff, vec3 spec)
    {
        this->scale_factor = scl;
        this->position = pos;
        this->color = col;
        this->dirLight.ambient = amb;
        this->dirLight.diffuse = diff;
        this->dirLight.specular = spec;
        updateLight();
    }

    void updateLight()
    {
        this->dirLight.direction = normalize(-(this->position));
    }

    void Draw(Shader &shader)
    {
        shader.bind();
        {
            shader.setVec3("Color", this->color);
            mat4 view = camera.GetViewMatrix();
            mat4 projection = camera.GetProjectionMatrix();
            shader.setMat4("Projection", projection);
            shader.setMat4("View", view);

            mat4 model = mat4(1.0f);
            model = translate(model, this->position);
            model = scale(model, vec3(this->scale_factor));
            shader.setMat4("Model", model);

            mesh->Draw(shader);
        }
        shader.unbind();
    }
};

#endif
