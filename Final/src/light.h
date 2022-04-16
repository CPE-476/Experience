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

#define NUM_POINT_LIGHTS 2

vec3 pointLightPositions[] = {
    vec3(-5.0f,  0.0f,  0.0f),
    vec3( 5.0f,  0.0f,  0.0f),
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};

struct LightSystem {
    DirLight dirLight;
    PointLight pointLights[NUM_POINT_LIGHTS];
    SpotLight spotLight;

    LightSystem(Camera &camera)
    {
        dirLight = {vec3(0.0f, 0.0f, -1.0f),   // Direction
                    vec3(0.05f, 0.05f, 0.05f), // Ambient
                    vec3(0.4f, 0.4f, 0.4f),    // Diffuse
                    vec3(0.5f, 0.5f, 0.5f)};   // Specular

        for(int i = 0; i < NUM_POINT_LIGHTS; ++i)
        {
            pointLights[i] = {pointLightPositions[i],    // Position
                              vec3(0.05f, 0.05f, 0.05f), // Ambient
                              vec3(0.8, 0.8f, 0.8f),     // Diffuse
                              vec3(1.0f, 1.0f, 1.0f),    // Specular
                              1.0f,                      // Constant
                              0.09f,                     // Linear
                              0.032f};                   // Quadratic
        }

        spotLight = {camera.Position,          // Position
                     camera.Front,             // Direction
                     vec3(0.0f, 0.0f, 0.0f),   // Ambient
                     vec3(1.0f, 1.0f, 1.0f),   // Diffuse
                     vec3(1.0f, 1.0f, 1.0f),   // Specular
                     1.0f,                     // Constant
                     0.09f,                    // Linear
                     0.032f,                   // Quadratic
                     cos(radians(12.5f)),      // Cutoff Point
                     cos(radians(15.0f))};     // Outer Cutoff Point
    }

    void Render(Shader &shader)
    {
        shader.setVec3("dirLight.direction", dirLight.direction);
        shader.setVec3("dirLight.ambient", dirLight.ambient);
        shader.setVec3("dirLight.diffuse", dirLight.diffuse);
        shader.setVec3("dirLight.specular", dirLight.specular);
        
        /*
        // spotLight
        shader.setVec3("spotLight.position", spotLight.position);
        shader.setVec3("spotLight.direction", spotLight.direction);
        shader.setVec3("spotLight.ambient", spotLight.ambient);
        shader.setVec3("spotLight.diffuse", spotLight.diffuse);
        shader.setVec3("spotLight.specular", spotLight.specular);
        shader.setFloat("spotLight.constant", spotLight.constant);
        shader.setFloat("spotLight.linear", spotLight.linear);
        shader.setFloat("spotLight.quadratic", spotLight.quadratic);
        shader.setFloat("spotLight.cutOff", spotLight.cutOff);
        shader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);
        */

        for(int i = 0; i < NUM_POINT_LIGHTS; ++i)
        {
            shader.setVec3("pointLights[" + to_string(i) + "].position", pointLights[i].position);
            shader.setVec3("pointLights[" + to_string(i) + "].ambient", pointLights[i].ambient);
            shader.setVec3("pointLights[" + to_string(i) + "].diffuse", pointLights[i].diffuse);
            shader.setVec3("pointLights[" + to_string(i) + "].specular", pointLights[i].specular);
            shader.setFloat("pointLights[" + to_string(i) + "].constant", pointLights[i].constant);
            shader.setFloat("pointLights[" + to_string(i) + "].linear", pointLights[i].linear);
            shader.setFloat("pointLights[" + to_string(i) + "].quadratic", pointLights[i].quadratic);
        }

        shader.setFloat("shine", 32.0f);
    }
};

#endif
