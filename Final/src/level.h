
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
#include "object.h"

using namespace std;
using namespace glm;

class level {
    public:
        vector<Object> Objects;
        Camera camera;
        string terrain;

    vector<pair<string, Model>> modelList;
    // Adding new Models
    modelList.push_back("TREE", new Model("../resources/22-trees_9_obj/trees9.obj"));

    void LoadLevel(std::string Filename)
{

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

                if (Type == "OBJ"){
                    
                }
                else if (Type == "POV"){

                }
                else if (Type == "POV"){

                }
                else{
                    cout << "LoadLevel ERROR\n";
                }
                
            }
        }
    }
    else
    {
        cout << "LoadLevel ERROR\n";
    }

    fp.close();
    }
};