
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
#include "manager.h"

using namespace std;
using namespace glm;

class level {
public:
    level()
    {
    }

    Camera camera;

    void LoadLevel(string Filename, vector<Object> &objects, Manager* m)
    {

        string Line;
        string Type;
        string cont;
        vector<string> conStr;
        vector<const char*> conPrt;

        ifstream fp;
        fp.open(Filename);
        if(fp.is_open())
        {
            while(getline(fp, Line))
            {
                if(!Line.empty())
                {
                    int id;
                    vec3 pos;
                    float angleX;
                    float angleY;
                    float angleZ;
                    vec3 vel;
                    float rad_h;
                    float rad_w;
                    float scaleFactor;
     
                    Type = Line.substr(0, 3);
                    cont = Line.substr(4);

                    conStr = getCont(cont);

                    for (int i = 0; i < conStr.size(); i++){
                        const char* char_array = conStr[i].c_str();
                        conPrt.push_back(char_array);    
                    }

                    if (Type == "OBJ"){ 
                        // get the id and other data
                        id = (int)atof(conPrt[0]);
                        pos = vec3((float)atof(conPrt[1]), (float)atof(conPrt[2]), (float)atof(conPrt[3]));
                        angleX = (float)atof(conPrt[4]);
                        angleY = (float)atof(conPrt[5]);
                        angleZ = (float)atof(conPrt[6]);
                        vel = vec3((float)atof(conPrt[7]), (float)atof(conPrt[8]), (float)atof(conPrt[9]));
                        rad_h = (float)atof(conPrt[10]);
                        rad_w = (float)atof(conPrt[11]);
                        scaleFactor = (float)atof(conPrt[12]);
                        objects.push_back(Object(id, pos, angleX, angleY, angleZ, vel, rad_h, rad_w, scaleFactor, m));
                    }
                    /*
                    else if (Type == "POV"){
                        cout << conPrt[0] << "\n";
                        pos = vec3((float)atof(conPrt[0]), 0.0f, (float)atof(conPrt[1]));
                        camera.Position = pos;
                    }
                    else if (Type == "TER"){
                        const char* terrName = conPrt[0];
                        for (int i = 0; i < TerrList.size(); i++){
                            if (strcmp(terrName, TerrList[i].terrName) == 0){
                                // terr exist
                                terrain = TerrList[i].terr;
                            }
                        }
                    }
                    else{
                        cout << "inside COM\n";
                    }*/
                    conPrt.clear();
                }
            }
        }
        else
        {
            cout << "Error: Level file not found.\n";
        }

        fp.close();
    }

    void SaveLevel(string Filename, vector<Object> &objects)
    {
        ofstream fp;
        fp.open(Filename);
        fp << "COM <type id pos.x pos.y pos.z angleX angleY angleZ vel.x vel.y vel.z rad_h rad_w scale>\n";
        for(int i = 0; i < objects.size(); ++i)
        {
            fp << "OBJ ";
            fp << objects[i].id << " ";
            fp << objects[i].position.x << " ";
            fp << objects[i].position.y << " ";
            fp << objects[i].position.z << " ";
            fp << objects[i].angleX << " ";
            fp << objects[i].angleY << " ";
            fp << objects[i].angleZ << " ";
            fp << objects[i].velocity.x << " ";
            fp << objects[i].velocity.y << " ";
            fp << objects[i].velocity.z << " ";
            fp << objects[i].height_radius << " ";
            fp << objects[i].width_radius << " ";
            fp << objects[i].scaleFactor;

            fp << "\n";
        }

        fp.close();
    }

    vector<string> getCont(string cont)
    {
        vector<string> res;
        string delimiter = " ";
        while(cont.find(delimiter) != -1){
            char* prt;
            string new_string = cont.substr(0, cont.find(delimiter)); 
            cont = cont.substr(cont.find(delimiter) + 1);
            prt = &new_string[0];
            res.push_back(new_string);
        }
        char* prt;
        string new_string = cont.substr(0, cont.find(delimiter)); 
        cont = cont.substr(cont.find(delimiter) + 1);
        prt = &new_string[0];
        res.push_back(new_string);

        return res; 
    }
};
