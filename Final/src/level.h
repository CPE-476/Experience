
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

struct ModelPair {
    char* modelName;
    Model* model;
};

struct ShaderPair {
    char* shaderName;
    Shader* shader;
};

struct TerrPair {
    char* terrName;
    Heightmap* terr;
};

class level {

    public:
    Model *bp;
    Heightmap *dunes;
    level()
    {
        this->dunes = new Heightmap("../resources/heightmap.png");
        this->bp = new Model("../resources/backpack/backpack.obj");
    }
        vector<Object*> Objects;
        Camera camera;
        Heightmap* terrain;

    void LoadLevel(std::string Filename)
{
    // Adding new Models
    vector<ModelPair> modelList {
        {strdup("BPK"), bp}
    };

    // Adding new Shaders
    vector<ShaderPair> ShaderList {
        {strdup("TEX"), &textureShader},
        {strdup("MAT"), &materialShader}
    };

    vector<TerrPair> TerrList {
        {strdup("DUNES"), dunes}
    };

    string Line;
    string Type;
    string cont;
    char* prt;
    size_t len;
    vector<string> conStr;
    vector<const char*> conPrt;

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
                vec3 scale;

 
                Type = Line.substr(0, 3);
                cont = Line.substr(4);

                conStr = getCont(cont);

                for (int i = 0; i < conStr.size(); i++){
                    const char* char_array = conStr[i].c_str();
                    conPrt.push_back(char_array);    
                }

                if (Type == "OBJ"){
                    
                    const char* objectName = conPrt[0];
                    for (int i = 0; i < modelList.size(); i++){
                        if (strcmp(objectName, modelList[i].modelName) == 0){
                            // model exist
                            const char* shaderName = conPrt[1];
                            for (int j = 0; j < ShaderList.size(); j++){
                                if (strcmp(shaderName, ShaderList[j].shaderName) == 0){
                                    // shader exist
                                    cout << "here1\n" << conPrt.size() << "\n";
                                    pos = vec3((float)atof(conPrt[2]), 0.0f, (float)atof(conPrt[3]));
                                    angle = (float) atof(conPrt[4]);
                                    rot = vec3((float)atof(conPrt[5]), (float)atof(conPrt[6]), (float)atof(conPrt[7]));
                                    vel = vec3((float)atof(conPrt[8]), (float)atof(conPrt[9]), (float)atof(conPrt[10]));
                                    rad_h = (float) atof(conPrt[11]);
                                    rad_w = (float) atof(conPrt[12]);
                                    scale = vec3((float)atof(conPrt[13]), (float)atof(conPrt[14]), (float)atof(conPrt[15]));
                                    cout << "here2\n";
                                    Object* newObject = new Object(modelList[i].model, ShaderList[j].shader, pos, angle, rot, vel, rad_h, rad_w, scale);
                                    cout << "here3\n";
                                    Objects.push_back(newObject);

                                    cout << Objects[0]->position.x << "\n";
                                }
                            }
                        }
                    }
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
        cout << "LoadLevel ERROR\n";
    }

    fp.close();
    }

    vector<string> getCont(string cont){
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