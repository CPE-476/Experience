// Author: Alex Hartford, Lucas Li
// Program: Experience
// File: Level Loader
// Date: May 2022

#ifndef LEVEL_H
#define LEVEL_H

#include <iostream>
#include <fstream>
#include <assert.h>
#include <vector>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "manager.h"
#include "shader.h"
#include "camera.h"
#include "object.h"
#include "light.h"
#include "particles.h"
#include "skybox.h"
#include "terrain.h"
#include "boundary.h"
#include "sound.h"
#include "sun.h"

using namespace std;
using namespace glm;

struct Level
{
    string currentLevel = "../levels/base.txt";

    vec3 startPosition = vec3(0.0f, 0.0f, 0.0f);
    string nextLevel = "../levels/base.txt";

    Level()
    {

    }

    void LoadLevel(string Filename, vector<Object> *objects, 
            vector<Light> *lights, Sun *sun,
            vector<Emitter> *emitters, FogSystem *fog, 
            Skybox *skybox, Terrain *terrain,
            Boundary *bound)
    {
        currentLevel = Filename;

        // Clear the current level.
        objects->clear();
        lights->clear();
        emitters->clear();

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
                    Type = Line.substr(0, 3);
                    cont = Line.substr(4);

                    conStr = getCont(cont);

                    for (int i = 0; i < conStr.size(); i++)
                    {
                        const char* char_array = conStr[i].c_str();
                        conPrt.push_back(char_array);    
                    }

                    if(Type == "NXT")
                    {
                        cout << "Next level found.\n";

                        nextLevel = conPrt[0];
                    }
                    else if(Type == "OBJ")
                    {
                        int id;

                        vec3 pos;
                        float angleX;
                        float angleY;
                        float angleZ;
                        vec3 vel;
                        float rad_v;
                        float rad_c;
                        float rad_s;
                        float scaleFactor;

                        bool inter;
                        bool disap;
                        int noteN;
                        int snd;

                        // get the id and other data
                        id = (int)atof(conPrt[0]);
                        pos = vec3((float)atof(conPrt[1]), (float)atof(conPrt[2]), (float)atof(conPrt[3]));
                        angleX = (float)atof(conPrt[4]);
                        angleY = (float)atof(conPrt[5]);
                        angleZ = (float)atof(conPrt[6]);
                        vel = vec3((float)atof(conPrt[7]), (float)atof(conPrt[8]), (float)atof(conPrt[9]));
                        rad_v = (float)atof(conPrt[10]);
                        rad_c = (float)atof(conPrt[11]);
                        rad_s = (float)atof(conPrt[12]);
                        scaleFactor = (float)atof(conPrt[13]);
                        inter = (bool)atoi(conPrt[14]);
                        disap = (bool)atoi(conPrt[15]);
                        noteN = atoi(conPrt[16]);
                        snd = atoi(conPrt[17]);

                        objects->push_back(Object(id, pos, angleX, angleY, angleZ, vel, rad_v, rad_c, rad_s, scaleFactor, inter, disap, noteN, snd));
                    }
                    else if (Type == "LGT")
                    {
                        int id;
                        vec3 pos;

                        vec3 ambient;
                        vec3 diffuse;
                        vec3 specular;

                        float constant;
                        float linear;
                        float quadratic;
     
                        pos = vec3((float)atof(conPrt[0]), (float)atof(conPrt[1]), (float)atof(conPrt[2]));
                        ambient = vec3((float)atof(conPrt[3]), (float)atof(conPrt[4]), (float)atof(conPrt[5]));
                        diffuse = vec3((float)atof(conPrt[6]), (float)atof(conPrt[7]), (float)atof(conPrt[8]));
                        specular = vec3((float)atof(conPrt[9]), (float)atof(conPrt[10]), (float)atof(conPrt[11]));
                        constant = (float)atof(conPrt[12]);
                        linear = (float)atof(conPrt[13]);
                        quadratic = (float)atof(conPrt[14]);
                        lights->push_back(Light(pos, ambient, diffuse, specular, constant, linear, quadratic));
                    }
                    else if (Type == "SUN")
                    {
                        cout << "Sun loaded from file\n";
                        float scl;
                        vec3 pos;
                        vec3 col;

                        vec3 ambient;
                        vec3 diffuse;
                        vec3 specular;

                        scl = (float)atof(conPrt[0]);
                        pos = vec3((float)atof(conPrt[1]), (float)atof(conPrt[2]), (float)atof(conPrt[3]));
                        col = vec3((float)atof(conPrt[4]), (float)atof(conPrt[5]), (float)atof(conPrt[6]));
                        ambient = vec3((float)atof(conPrt[7]), (float)atof(conPrt[8]), (float)atof(conPrt[9]));
                        diffuse = vec3((float)atof(conPrt[10]), (float)atof(conPrt[11]), (float)atof(conPrt[12]));
                        specular = vec3((float)atof(conPrt[13]), (float)atof(conPrt[14]), (float)atof(conPrt[15]));

                        sun->init(scl, pos, col, ambient, diffuse, specular);
                    }
                    else if (Type == "PAR")
                    {
                        string path;
                        int partAmt;
                        vec3 pos;
                        float rad1;
                        float rad2;
                        float height;
                        vec3 vel;
                        float life;
                        float grav;
                        vec4 startCol;
                        vec4 endCol;
                        float startScl;
                        float endScl;
                        int bugMode;
                        int fogMode;

                        // get the id and other data
                        path = conPrt[0];
                        partAmt = (int)atof(conPrt[1]);
                        pos = vec3((float)atof(conPrt[2]), (float)atof(conPrt[3]), (float)atof(conPrt[4]));
                        rad1 = (float)atof(conPrt[5]);
                        rad2 = (float)atof(conPrt[6]);
                        height = (float)atof(conPrt[7]);
                        vel = vec3((float)atof(conPrt[8]), (float)atof(conPrt[9]), (float)atof(conPrt[10]));
                        life = (float)atof(conPrt[11]);
                        grav = (float)atof(conPrt[12]);
                        startCol = vec4((float)atof(conPrt[13]), (float)atof(conPrt[14]), (float)atof(conPrt[15]), (float)atof(conPrt[16]));
                        endCol = vec4((float)atof(conPrt[17]), (float)atof(conPrt[18]), (float)atof(conPrt[19]), (float)atof(conPrt[20]));
                        startScl = (float)atof(conPrt[21]);
                        endScl = (float)atof(conPrt[22]);
                        bugMode = atoi(conPrt[23]);
                        fogMode = atoi(conPrt[24]);
                        Emitter em = Emitter(path, partAmt, pos, rad1, rad2, height, vel, life, grav, startCol, endCol, startScl, endScl);
                        if(bugMode)
                        {
                            em.bugMode = 1;
                        }
                        if(fogMode)
                        {
                            em.fogMode = 1;
                        }
                        emitters->push_back(em);
                    }
                    else if(Type == "FOG")
                    {
                        cout << "Fog loaded from file\n";
                        fog->maxDistance = (float)atof(conPrt[0]);
                        fog->minDistance = (float)atof(conPrt[1]);
                        fog->color = vec4((float)atof(conPrt[2]), (float)atof(conPrt[3]), (float)atof(conPrt[4]), (float)atof(conPrt[5]));
                    }
                    else if (Type == "TER")
                    {
                        if (Filename != "../levels/credit.txt") {
                            
                        }
                        cout << "Terrain loaded from file\n";
                        string path;
                        float y_scale;
                        float y_shift;
                        vec3 bottom;
                        vec3 top;
                        vec3 dirt;

                        path = conPrt[0];
                        y_scale = (float)atof(conPrt[1]);
                        y_shift = (float)atof(conPrt[2]);
                        bottom = vec3((float)atof(conPrt[3]), (float)atof(conPrt[4]), (float)atof(conPrt[5]));
                        top = vec3((float)atof(conPrt[6]), (float)atof(conPrt[7]), (float)atof(conPrt[8]));
                        dirt = vec3((float)atof(conPrt[9]), (float)atof(conPrt[10]), (float)atof(conPrt[11]));
                        
                        terrain->init(path, y_scale, y_shift, bottom, top, dirt);
                    }
                    else if (Type == "BND")
                    {
                        cout << "Boundary loaded from file\n";
                        vec3 color;
                        float y;
                        float wid;
                        float hgt;
                        
                        color = vec3((float)atof(conPrt[0]), (float)atof(conPrt[1]), (float)atof(conPrt[2]));
                        y = (float)atof(conPrt[3]);
                        wid = (float)atof(conPrt[4]);
                        hgt = (float)atof(conPrt[5]);
                        
                        bound->init(color, y, wid, hgt);
                    }
                    else if (Type == "SKY")
                    {
                        cout << "Skybox loaded from file\n";
                        string path;
                        
                        path = conPrt[0];
                        skybox->init(path, false);
                    }

                    else if (Type == "POV")
                    {
                        cout << "Camera Position loaded from file\n";
                        vec3 pos;
                        pos = vec3((float)atof(conPrt[0]), (float)atof(conPrt[1]), (float)atof(conPrt[2]));

                        this->startPosition = pos;
                        camera.Position = pos;
                    }
                    else
                    {
                        //cout << "inside COM\n";
                    }
                    conPrt.clear();
                }
            }
        }
        else
        {
            cout << "Error: Level file not found.\n";
        }
        cout << "Finished Loading\n";
        fp.close();
    }

    void SaveLevel(string Filename, vector<Object> *objects, 
            vector<Light> *lights, Sun *sun, 
            vector<Emitter> *emitters, FogSystem *fog,
            Skybox *skybox, Terrain *terrain,
            Boundary *bound)
    {
        ofstream fp;
        fp.open(Filename);

        fp << "COM Level saved by the Experience Level Loader\n";
        fp << "COM By Brett Hickman, Lucas Li, and Alex Hartford\n";
        fp << "COM Spring 2022\n\n";

        // Save Next Level data
        fp << "\nCOM Next Level: <NXT path>\n";
        fp << "NXT ";
        fp << nextLevel;
        fp << "\n";

        // Save Desired Camera Position
        fp << "\nCOM Camera Setup: <POV pos.x pos.y pos.z>\n";
        fp << "POV ";
        fp << startPosition.x << " ";
        fp << startPosition.y << " ";
        fp << startPosition.z << " ";
        fp << "\n";

	// Save Object Data
	fp << "\nCOM Object: <OBJ id pos.x pos.y pos.z angleX angleY angleZ vel.x vel.y vel.z rad_v rad_c rad_s scale inter? disap? noteN sound>\n";
	for(int i = 0; i < objects->size(); ++i)
	{
	    fp << "OBJ ";
	    fp << objects->at(i).id << " ";
	    fp << objects->at(i).position.x << " ";
	    fp << objects->at(i).position.y << " ";
	    fp << objects->at(i).position.z << " ";
	    fp << objects->at(i).angleX << " ";
	    fp << objects->at(i).angleY << " ";
	    fp << objects->at(i).angleZ << " ";
	    fp << objects->at(i).velocity.x << " ";
	    fp << objects->at(i).velocity.y << " ";
	    fp << objects->at(i).velocity.z << " ";
	    fp << objects->at(i).view_radius << " ";
	    fp << objects->at(i).collision_radius << " ";
	    fp << objects->at(i).selection_radius << " ";
	    fp << objects->at(i).scaleFactor << " ";
	    fp << objects->at(i).interactible << " ";
	    fp << objects->at(i).disappearing << " ";
	    fp << objects->at(i).noteNum << " ";
	    fp << objects->at(i).sound << " ";
	    fp << "\n";
	}

	// Save Point Light Data
	fp << "\nCOM Light: <LGT pos.x pos.y pos.z amb.x amb.y amb.z dif.x dif.y dif.z spec.x spec.y spec.z constant linear quadratic>\n";
	for(int i = 0; i < lights->size(); ++i)
	{
	    fp << "LGT ";
	    fp << lights->at(i).position.x << " ";
	    fp << lights->at(i).position.y << " ";
	    fp << lights->at(i).position.z << " ";
	    fp << lights->at(i).ambient.x << " ";
	    fp << lights->at(i).ambient.y << " ";
	    fp << lights->at(i).ambient.z << " ";
	    fp << lights->at(i).diffuse.x << " ";
	    fp << lights->at(i).diffuse.y << " ";
	    fp << lights->at(i).diffuse.z << " ";
	    fp << lights->at(i).specular.x << " ";
	    fp << lights->at(i).specular.y << " ";
	    fp << lights->at(i).specular.z << " ";
	    fp << lights->at(i).constant << " ";
	    fp << lights->at(i).linear << " ";
	    fp << lights->at(i).quadratic << " ";
	    fp << "\n";
	}

	// Save Directional Light Data
	fp << "\nCOM Sun: <SUN scl pos.x pos.y pos.z col.r col.g col.b dir.x dir.y dir.z amb.x amb.y amb.z dif.x dif.y dif.z spec.x spec.y spec.z>\n";
	fp << "SUN ";
	fp << sun->scale_factor << " ";
	fp << sun->position.x << " ";
	fp << sun->position.y << " ";
	fp << sun->position.z << " ";
	fp << sun->color.r << " ";
	fp << sun->color.g << " ";
	fp << sun->color.b << " ";
	fp << sun->dirLight.ambient.x << " ";
	fp << sun->dirLight.ambient.y << " ";
	fp << sun->dirLight.ambient.z << " ";
	fp << sun->dirLight.diffuse.x << " ";
	fp << sun->dirLight.diffuse.y << " ";
	fp << sun->dirLight.diffuse.z << " ";
	fp << sun->dirLight.specular.x << " ";
	fp << sun->dirLight.specular.y << " ";
	fp << sun->dirLight.specular.z << " ";
	fp << "\n";

	// Save Particle System Data
	fp << "\nCOM Emitter: <PAR path partAmt pos.x pos.y pos.z rad1 rad2 height vel.x vel.y vel.z life grav startCol.x startCol.y startCol.z startCol.a endCol.x endCol.y endCol.z endCol.a startScl endScl Bugmode Fogmode>\n";
	for(int i = 0; i < emitters->size(); ++i){
	    fp << "PAR ";
	    fp << emitters->at(i).path << " ";
	    fp << emitters->at(i).particleAmount << " ";
	    fp << emitters->at(i).startPosition.x << " ";
	    fp << emitters->at(i).startPosition.y << " ";
	    fp << emitters->at(i).startPosition.z << " ";
	    fp << emitters->at(i).radius << " ";
	    fp << emitters->at(i).radiusTop << " ";
	    fp << emitters->at(i).height << " ";
	    fp << emitters->at(i).startVelocity.x << " ";
	    fp << emitters->at(i).startVelocity.y << " ";
	    fp << emitters->at(i).startVelocity.z << " ";
	    fp << emitters->at(i).lifeSpan << " ";
	    fp << emitters->at(i).gravity << " ";
	    fp << emitters->at(i).startColor.x << " ";
	    fp << emitters->at(i).startColor.y << " ";
	    fp << emitters->at(i).startColor.z << " ";
	    fp << emitters->at(i).startColor.a << " ";
	    fp << emitters->at(i).endColor.x << " ";
	    fp << emitters->at(i).endColor.y << " ";
	    fp << emitters->at(i).endColor.z << " ";
	    fp << emitters->at(i).endColor.a << " ";
	    fp << emitters->at(i).startScale << " ";
	    fp << emitters->at(i).endScale << " ";
            fp << emitters->at(i).bugMode << " ";
            fp << emitters->at(i).fogMode << " ";
	    fp << "\n";
	}

	fp << "\nCOM Fog: <FOG max min col.x col.y col.z col.a>\n";
	fp << "FOG ";
	fp << fog->maxDistance << " ";
	fp << fog->minDistance << " ";
	fp << fog->color.x << " ";
	fp << fog->color.y << " ";
	fp << fog->color.z << " ";
	fp << fog->color.a;
	fp << "\n";

	fp << "\nCOM Terrain: <TER path yScale yShift amb.x amb.y amb.z dif.x dif.y dif.z spec.x spec.y spec.z shine>\n";
	fp << "TER ";
	fp << terrain->path << " ";
	fp << terrain->yScale << " ";
	fp << terrain->yShift << " ";
	fp << terrain->bottom.x << " ";
	fp << terrain->bottom.y << " ";
	fp << terrain->bottom.z << " ";
	fp << terrain->top.x << " ";
	fp << terrain->top.y << " ";
	fp << terrain->top.z << " ";
	fp << terrain->dirt.x << " ";
	fp << terrain->dirt.y << " ";
	fp << terrain->dirt.z << " ";
	fp << "\n";

	fp << "\nCOM Skybox: <SKY dir>\n";
	fp << "SKY ";
	fp << skybox->dir;
	fp << "\n";

	fp << "\nCOM Boundary: <BND col.r, col.g, col.b>\n";
	fp << "BND ";
	fp << bound->color.x << " ";
	fp << bound->color.y << " ";
	fp << bound->color.z << " ";
        fp << bound->boundY << " ";
        fp << bound->width << " ";
        fp << bound->height << " ";
	fp << "\n";

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

#endif
