// Author: Alex Hartford
// Program: Experience
// File: Camera Class
// Date: April 2022

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

using namespace std;
using namespace glm;

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

enum Modes {
    FREE,
    FAST,
    WALK,
    SPRINT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float FASTSPEED = 100.0f;
const float SENSITIVITY = 0.03f;
const float ZOOM = 45.0f;

class Camera
{
public:
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MouseSensitivity;
    float Zoom;

    int Mode;

    Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), 
           vec3 up = vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH) : 
           Front(vec3(0.0f, 0.0f, -1.0f)), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), Mode(FREE)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    mat4 GetProjectionMatrix()
    {
        return perspective(radians(Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
    }

    mat4 GetViewMatrix()
    {
        return lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = 0;
        if(Mode == FREE || Mode == WALK)
        {
            velocity = SPEED * deltaTime;
        }
        else if(Mode == FAST || Mode == SPRINT)
        {
            velocity = FASTSPEED * deltaTime;
        }
        if(Mode == FREE || Mode == FAST)
        {
            if(direction == FORWARD)
                Position += Front * velocity;
            if(direction == BACKWARD)
                Position -= Front * velocity;
            if(direction == LEFT)
                Position -= Right * velocity;
            if(direction == RIGHT)
                Position += Right * velocity;
        }
        else if(Mode == WALK || Mode == SPRINT)
        {
            if(direction == FORWARD)
                Position += cross(WorldUp, Right) * velocity;
            if(direction == BACKWARD)
                Position -= cross(WorldUp, Right) * velocity;
            if(direction == LEFT)
                Position += cross(WorldUp, Front) * velocity;
            if(direction == RIGHT)
                Position -= cross(WorldUp, Front) * velocity;    
        }
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if(constrainPitch)
        {
            if(Pitch > 89.0f)
                Pitch = 89.0f;
            if(Pitch < -89.0f)
                Pitch = -89.0f;
        }
        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if(Zoom < 1.0f)
            Zoom = 1.0f;
        if(Zoom > 45.0f)
            Zoom = 45.0f;
    }
private:
    void updateCameraVectors()
    {
        vec3 front;
        front.x = cos(radians(Yaw)) * cos(radians(Pitch));
        front.y = sin(radians(Pitch));
        front.z = sin(radians(Yaw)) * cos(radians(Pitch));
        Front = normalize(front); 

        Right = normalize(cross(Front, WorldUp));
        Up = normalize(cross(Right, Front));
    }
};
#endif
