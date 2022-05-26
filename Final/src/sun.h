// Author: Alex Hartford
// Program: Experience
// File: Sun Class
// Date: May 2022

#ifndef SUN_H
#define SUN_H

#include <glad/glad.h>

#include "shader.h"
#include "spline.h"
using namespace std;
using namespace glm;

struct Sun
{
    void Draw(Shader &shader)
    {
        shader.bind();
        {
            
        }
        shader.unbind();
    }
};

#endif
