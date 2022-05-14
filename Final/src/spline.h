// Author: Alex Hartford
// Program: Experience
// File: Spline
// Date: May 2022

#ifndef SPLINE_H
#define SPLINE_H

#include <glm/glm.hpp>

using namespace std;
using namespace glm;

vec3 lerp(vec3 a, vec3 b, float t)
{
    return (1.0f - t) * a + t * b;
}

vec3 Bezier(vec3 a, vec3 b, vec3 control, float t)
{
    // set up endpoints of line to interpolate
    vec3 q0 = lerp(a, control, t);
    vec3 q1 = lerp(control, b, t);

    // return the interpolated point along the control line.
    return lerp(q0, q1, t);
}

struct Spline
{
    vec3  start;
    vec3  end;
    float time;
    float duration;
    bool  active;

    void init(vec3 s, vec3 e, float dur)
    {
        this->start = s;
        this->end = e;
        this->duration = dur;
        time = 0.0;
        active = false;
    }

    void update(float deltaTime)
    {
        time += deltaTime / duration;
        if(time > 1)
        {
            time = 1;
            active = false;
        }
    }

    vec3 getPosition()
    {
        return lerp(start, end, time);
    }
};

#endif
