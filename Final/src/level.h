// Author: Alex Hartford
// Program: Emblem
// File: Level Loader
// Date: February 2022

#include <iostream>
#include <fstream>
#include <assert.h>
#include <vector>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "model.h"
#include "camera.h"

using namespace std;
using namespace glm;

class level {
    public:
        vector<Object> Objects;
        Camera camera;
        String terrain;
}

void LoadLevel(std::string Filename)
{
    std::shared_ptr<Unit> Current;

    string Line;
    string Type;
    string Cont;

    int Col = 0;
    int Row = 0;

    ifstream fp;
    fp.open(Filename);
    if(fp.is_open())
    {
        while(getline(fp, Line))
        {
            if(!Line.empty())
            {
                vec3 pos;
                float angle;
                vec3 rot;
                vec3 vel;
                float rad_h;
                float rad_w;
                float scale;

                Type = Line.substr(0, 3);
                Cont = Line.substr(4);

                switch (Type)
                {
                case 'OBJ':
                    
                    break;
                case 'POV':
                    /* code */
                    break;
                case 'COM':
                    /* code */
                    break;
                default:
                    break;
                }
            }
        }
    }
    else
    {
        cout << "LoadLevel ERROR\n";
    }

    for(std::shared_ptr<Unit> unt : CurrentLevel.Units)
    {
        CurrentLevel.Tiles[unt->Position.x][unt->Position.y].Occupant = unt;
        CurrentLevel.Tiles[unt->Position.x][unt->Position.y].Occupied = true;
    }
    for(std::shared_ptr<Unit> unt : CurrentLevel.Enemies)
    {
        CurrentLevel.Tiles[unt->Position.x][unt->Position.y].Occupant = unt;
        CurrentLevel.Tiles[unt->Position.x][unt->Position.y].Occupied = true;
    }

    fp.close();
}

