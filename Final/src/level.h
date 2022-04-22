
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
    Model *sk;
    Heightmap *dunes;

    level(Heightmap *dunes, Model *backpack, Model *skull)
    {
        this->dunes = dunes;
        this->bp = backpack;
        this->sk = skull;
    }

    Camera camera;
    Heightmap* terrain;

    void LoadLevel(string Filename, vector<Object> &objects)
    {
        // Adding new Models
        vector<ModelPair> modelList {
            {strdup("BPK"), bp},
            {strdup("SKL"), sk}
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
                    int shad_t;
                    vec3 pos;
                    float angle;
                    vec3 rot;
                    vec3 vel;
                    float rad_h;
                    float rad_w;
                    vec3 scale;

                    string m;
                    string s;
     
                    Type = Line.substr(0, 3);
                    cont = Line.substr(4);

                    conStr = getCont(cont);

                    for (int i = 0; i < conStr.size(); i++){
                        const char* char_array = conStr[i].c_str();
                        conPrt.push_back(char_array);    
                    }

                    if (Type == "OBJ"){ 
                        const char* objectName = conPrt[0];
                        m = conPrt[0];
                        for (int i = 0; i < modelList.size(); i++){
                            if (strcmp(objectName, modelList[i].modelName) == 0){
                                // model exist
                                const char* shaderName = conPrt[1];
                                s = conPrt[1];
                                for (int j = 0; j < ShaderList.size(); j++){
                                    if (strcmp(shaderName, ShaderList[j].shaderName) == 0){
                                        if(j == 0)
                                            shad_t = TEXTURE;
                                        else
                                            shad_t = MATERIAL;
                                        pos = vec3((float)atof(conPrt[2]), 0.0f, (float)atof(conPrt[3]));
                                        angle = (float) atof(conPrt[4]);
                                        rot = vec3((float)atof(conPrt[5]), (float)atof(conPrt[6]), (float)atof(conPrt[7]));
                                        vel = vec3((float)atof(conPrt[8]), (float)atof(conPrt[9]), (float)atof(conPrt[10]));
                                        rad_h = (float) atof(conPrt[11]);
                                        rad_w = (float) atof(conPrt[12]);
                                        scale = vec3((float)atof(conPrt[13]), (float)atof(conPrt[14]), (float)atof(conPrt[15]));
                                        objects.push_back(Object(modelList[i].model, ShaderList[j].shader, shad_t, pos, angle, rot, vel, rad_h, rad_w, scale, m, s));
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
            cout << "Error: Level file not found.\n";
        }

        fp.close();
    }

    void SaveLevel(string Filename, vector<Object> &objects)
    {
        ofstream fp;
        fp.open(Filename);
        fp << "COM <type model shader pos.x pos.z angle rot.x rot.y rot.z vel.x vel.y vel.z rad_h rad_w scale>\n";
        for(int i = 0; i < objects.size(); ++i)
        {
            fp << "OBJ ";
            fp << objects[i].MODEL_ID << " ";
            fp << objects[i].SHADER_ID << " ";
            fp << objects[i].position.x << " ";
            fp << objects[i].position.z << " ";
            fp << objects[i].angle << " ";
            fp << objects[i].rotation.x << " ";
            fp << objects[i].rotation.y << " ";
            fp << objects[i].rotation.z << " ";
            fp << objects[i].velocity.x << " ";
            fp << objects[i].velocity.y << " ";
            fp << objects[i].velocity.z << " ";
            fp << objects[i].height_radius << " ";
            fp << objects[i].width_radius << " ";
            fp << objects[i].Scale.x << " ";
            fp << objects[i].Scale.y << " ";
            fp << objects[i].Scale.z;

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
