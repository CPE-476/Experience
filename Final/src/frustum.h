#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

using namespace std;
using namespace glm;

/* helper function to compute distance to the plane */
// NOTE Input is Normalized!
float DistToPlane(float A, float B, float C, float D, vec3 point) {
    return (A * point.x + B * point.y + C * point.z + D);
}

struct Frustum
{
    vec4 Left, Right, Bottom, Top, Near, Far;
    vec4 planes[6];

    Frustum()
    {
    };

    void ExtractVFPlanes(mat4 Projection, mat4 View)
    {
        /* composite matrix */
        mat4 comp = Projection * View;
        float l;

        Left.x = comp[0][3] + comp[0][0];
        Left.y = comp[1][3] + comp[1][0];
        Left.z = comp[2][3] + comp[2][0];
        Left.w = comp[3][3] + comp[3][0];

        l = glm::length(vec3(Left));
        Left.x /= l;
        Left.y /= l;
        Left.z /= l;
        Left.w /= l;
        planes[0] = Left;

        Right.x = comp[0][3] - comp[0][0]; 
        Right.y = comp[1][3] - comp[1][0]; 
        Right.z = comp[2][3] - comp[2][0]; 
        Right.w = comp[3][3] - comp[3][0];

        l = glm::length(vec3(Right));
        Right.x /= l;
        Right.y /= l;
        Right.z /= l;
        Right.w /= l;
        planes[1] = Right;

        Bottom.x = comp[0][3] + comp[0][1]; 
        Bottom.y = comp[1][3] + comp[1][1]; 
        Bottom.z = comp[2][3] + comp[2][1]; 
        Bottom.w = comp[3][3] + comp[3][1];

        l = glm::length(vec3(Bottom));
        Bottom.x /= l;
        Bottom.y /= l;
        Bottom.z /= l;
        Bottom.w /= l;
        planes[2] = Bottom;

        Top.x = comp[0][3] - comp[0][1]; 
        Top.y = comp[1][3] - comp[1][1]; 
        Top.z = comp[2][3] - comp[2][1]; 
        Top.w = comp[3][3] - comp[3][1];

        l = glm::length(vec3(Top));
        Top.x /= l;
        Top.y /= l;
        Top.z /= l;
        Top.w /= l;
        planes[3] = Top;

        Near.x = comp[0][3];
        Near.y = comp[1][3];
        Near.z = comp[2][3];
        Near.w = comp[3][3];

        l = glm::length(vec3(Near));
        Near.x /= l;
        Near.y /= l;
        Near.z /= l;
        Near.w /= l;
        planes[4] = Near;

        Far.x = comp[0][3] - comp[0][2];
        Far.y = comp[1][3] - comp[1][2];
        Far.z = comp[2][3] - comp[2][2];
        Far.w = comp[3][3] - comp[3][2];
        
        l = glm::length(vec3(Far));
        Far.x /= l;
        Far.y /= l;
        Far.z /= l;
        Far.w /= l;
        planes[5] = Far;
    }

    int ViewFrustCull(vec3 center, float radius)
    {
        float dist;
        for(int i=0; i < 6; i++) 
        {
            dist = DistToPlane(planes[i].x, planes[i].y, planes[i].z, planes[i].w, center);
            if(dist < 0 && fabs(dist) > radius)
            {
                return 1;
            }
        }
        return 0;
    }
};

#endif
