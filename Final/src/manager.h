// Author: Alex Hartford, Lucas Li
// Program: Experience
// File: Manager
// Date: April 2022

#ifndef MANAGER_H
#define MANAGER_H

/*
 * NOTE(Alex):
 * The purpose of the manager is to abstract away the compilation of shaders
 * and the loading of geometry, including the following things:
 *  - Models
 *  - Notes
 */

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

#include "shader.h"
#include "model.h"
#include "note.h"
#include "object.h"
#include "light.h"

using namespace std;
using namespace glm;

struct ID_Entry
{
    int ID;
    Model *model;

    Shader *shader;
    int shader_type;
    float collision_radius;
};

struct Shader_Container
{
    Shader textureShader;
    Shader materialShader;
    Shader typeShader;
    Shader skyboxShader;
    Shader lightShader;
    Shader particleShader;
    Shader noteShader;
    Shader terrainShader;
    Shader transShader;
    Shader waterShader;
    Shader boundaryShader;
    Shader depthShader;
    Shader shadowShader;
    Shader debugShader;
};

struct Model_Container
{
    // testing
    Model cylinder;
    Model cube;
    Model sphere;
    Model skull;

    Model note;

    // forest
    Model tree_1;
    Model tree_2;
    Model tree_3;
    Model tree_4;
    Model tree_5;
    Model rock_1;
    Model rock_2;
    Model rock_3;
    Model rock_4;
    Model campfire;
    Model snail;
    Model fern;
    Model grass_1;
    Model grass_2;
    Model grass_3;
    Model grass_4;
    Model grass_5;
    Model grass_6;
    Model grass_7;
    Model grass_8;
    Model grass_9;
    Model bear;
    Model boar;
    Model deer_1;
    Model deer_2;
    Model fox;
    Model hedhog;
    Model owl;
    Model rabbit;
    Model squirrel;
    Model wolf;

    // desert
    Model rock_5;
    Model rock_6;
    Model rock_7;
    Model rock_8;
    Model rock_9;
    Model rock_10;
    Model rock_11;    
    Model cactus_1;
    Model cactus_2;
    Model cactus_3;
    Model trumbleweed;

    // street
    Model road;
};

struct Note_Container
{
    Note aurelius1;
};

struct Manager
{
    Shader_Container shaders;
    Model_Container models;
    Note_Container notes;
    ID_Entry Lookup[100] = {};
    unsigned int bufferIds[100] = {};

    Manager()
    {
        /* Shader Compilation */
        this->shaders.textureShader.init("../shaders/texture_vert.glsl", "../shaders/texture_frag.glsl");
        this->shaders.materialShader.init("../shaders/material_vert.glsl", "../shaders/material_frag.glsl");
        this->shaders.typeShader.init("../shaders/type_vert.glsl", "../shaders/type_frag.glsl");
        this->shaders.skyboxShader.init("../shaders/cubemap_vert.glsl", "../shaders/cubemap_frag.glsl");
        this->shaders.lightShader.init("../shaders/light_vert.glsl", "../shaders/light_frag.glsl");
        this->shaders.particleShader.init("../shaders/part_vert.glsl", "../shaders/part_frag.glsl");
        this->shaders.noteShader.init("../shaders/note_vert.glsl", "../shaders/note_frag.glsl");
        this->shaders.terrainShader.init("../shaders/terrain_vert.glsl", "../shaders/terrain_frag.glsl");
        this->shaders.transShader.init("../shaders/trans_vert.glsl", "../shaders/trans_frag.glsl");
        this->shaders.waterShader.init("../shaders/water_vert.glsl", "../shaders/water_frag.glsl");
        this->shaders.boundaryShader.init("../shaders/bound_vert.glsl", "../shaders/bound_frag.glsl");
        this->shaders.depthShader.init("../shaders/depth_vert.glsl", "../shaders/depth_frag.glsl");
        this->shaders.shadowShader.init("../shaders/shadow_vert.glsl", "../shaders/shadow_frag.glsl");
        this->shaders.debugShader.init("../shaders/debug_vert.glsl", "../shaders/debug_frag.glsl");



        this->notes.aurelius1.init("../resources/notes/aurelius1.png");

        stbi_set_flip_vertically_on_load(false);
        this->models.cylinder.init("../resources/testing/cylinder.obj");
        this->models.cube.init("../resources/testing/cube.obj");
        this->models.sphere.init("../resources/testing/sphere.obj");
        this->models.skull.init("../resources/testing/skull.obj");
        this->models.tree_1.init("../resources/models/trees/tree_1.fbx");
        this->models.tree_2.init("../resources/models/trees/tree_2.fbx");
        this->models.tree_3.init("../resources/models/trees/tree_3.fbx");
        this->models.tree_4.init("../resources/models/trees/tree_4.fbx");
        this->models.tree_5.init("../resources/models/trees/tree_5.fbx");
        this->models.rock_1.init("../resources/models/rocks/forest_rocks/rock_1.fbx");
        this->models.rock_2.init("../resources/models/rocks/forest_rocks/rock_2.fbx");
        this->models.rock_3.init("../resources/models/rocks/forest_rocks/rock_3.fbx");
        this->models.rock_4.init("../resources/models/rocks/forest_rocks/rock_4.fbx");
        this->models.rock_5.init("../resources/models/rocks/desert_rocks/rock_13.fbx");
        this->models.rock_6.init("../resources/models/rocks/desert_rocks/rock_9.fbx");
        this->models.rock_7.init("../resources/models/rocks/desert_rocks/rock_8.fbx");
        this->models.rock_8.init("../resources/models/rocks/desert_rocks/rock_6.fbx");
        this->models.rock_9.init("../resources/models/rocks/desert_rocks/rock_3.fbx");
        this->models.rock_10.init("../resources/models/rocks/desert_rocks/rock_2.fbx");
        this->models.rock_11.init("../resources/models/rocks/desert_rocks/rock_1.fbx");
        this->models.campfire.init("../resources/models/environment/campfire/Campfire.fbx");
        this->models.snail.init("../resources/models/environment/snail/snail.fbx");
        this->models.cactus_1.init("../resources/models/environment/cactus/cactus_1.fbx");
        this->models.cactus_2.init("../resources/models/environment/cactus/cactus_2.fbx");
        this->models.cactus_3.init("../resources/models/environment/cactus/cactus_3.fbx");
        this->models.trumbleweed.init("../resources/models/environment/tumbleweed/Tumbleweed.fbx");
        this->models.fern.init("../resources/models/environment/fern/fern.fbx");
        this->models.grass_1.init("../resources/models/grass/grass_1.fbx");
        this->models.grass_2.init("../resources/models/grass/grass_2.fbx");
        this->models.grass_3.init("../resources/models/grass/grass_3.fbx");
        this->models.grass_4.init("../resources/models/grass/grass_4.fbx");
        this->models.grass_5.init("../resources/models/grass/grass_5.fbx");
        this->models.grass_6.init("../resources/models/grass/grass_6.fbx");
        this->models.grass_7.init("../resources/models/grass/grass_7.fbx");
        this->models.grass_8.init("../resources/models/grass/grass_8.fbx");
        this->models.grass_9.init("../resources/models/grass/grass_9.fbx");
        this->models.road.init("../resources/models/environment/road/road.fbx");
        // this->models.bear.init("../resources/models/animals/bear.fbx");
        // this->models.boar.init("../resources/models/animals/boar.fbx");
        // this->models.deer_1.init("../resources/models/animals/deer_1.fbx");
        // this->models.deer_2.init("../resources/models/animals/deer_2.fbx");
        // this->models.fox.init("../resources/models/animals/fox.fbx");
        // this->models.hedhog.init("../resources/models/animals/hedhog.fbx");
        // this->models.owl.init("../resources/models/animals/owl.fbx");
        // this->models.rabbit.init("../resources/models/animals/rabbit.fbx");
        // this->models.squirrel.init("../resources/models/animals/squirrel.fbx");
        // this->models.wolf.init("../resources/models/animals/wolf.fbx");

        

        this->models.note.init("../resources/models/environment/note/scroll2.fbx");

        this->Populate();
        this->genInstanceBuffers();
    }

    void Populate()
    {
        Lookup[0] = {0, &this->models.tree_1, &this->shaders.textureShader, TEXTURE, 1.3f};
        Lookup[1] = {1, &this->models.tree_2, &this->shaders.textureShader, TEXTURE, 2.8f};
        Lookup[2] = {2, &this->models.tree_3, &this->shaders.textureShader, TEXTURE, 2.8f};
        Lookup[3] = {3, &this->models.tree_4, &this->shaders.textureShader, TEXTURE, 2.8f};
        Lookup[4] = {4, &this->models.tree_5, &this->shaders.textureShader, TEXTURE, 2.0f};
        Lookup[5] = {5, &this->models.rock_1, &this->shaders.textureShader, TEXTURE, 1.1f};
        Lookup[6] = {6, &this->models.rock_2, &this->shaders.textureShader, TEXTURE, 1.1f};
        Lookup[7] = {7, &this->models.rock_3, &this->shaders.textureShader, TEXTURE, 1.1f};
        Lookup[8] = {8, &this->models.rock_4, &this->shaders.textureShader, TEXTURE, 1.1f};
        Lookup[9] = {9, &this->models.rock_5, &this->shaders.textureShader, TEXTURE, 1.9f};
        Lookup[10] = {10, &this->models.rock_6, &this->shaders.textureShader, TEXTURE, 1.3f};
        Lookup[11] = {11, &this->models.rock_7, &this->shaders.textureShader, TEXTURE, 1.0f};
        Lookup[12] = {12, &this->models.rock_8, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[13] = {13, &this->models.rock_9, &this->shaders.textureShader, TEXTURE, 2.8f};
        Lookup[14] = {14, &this->models.rock_10, &this->shaders.textureShader, TEXTURE, 2.0f};
        Lookup[15] = {15, &this->models.rock_11, &this->shaders.textureShader, TEXTURE, 13.5f};
        Lookup[16] = {16, &this->models.campfire, &this->shaders.textureShader, TEXTURE, 50.0f};
        Lookup[17] = {17, &this->models.snail, &this->shaders.textureShader, TEXTURE, 1.7f};
        Lookup[18] = {18, &this->models.fern, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[19] = {19, &this->models.cactus_1, &this->shaders.textureShader, TEXTURE, 1.0f};
        Lookup[20] = {20, &this->models.cactus_2, &this->shaders.textureShader, TEXTURE, 1.0f};
        Lookup[21] = {21, &this->models.cactus_3, &this->shaders.textureShader, TEXTURE, 1.0f};
        Lookup[22] = {22, &this->models.trumbleweed, &this->shaders.textureShader, TEXTURE, 3.6f};
        Lookup[23] = {23, &this->models.grass_1, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[24] = {24, &this->models.grass_2, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[25] = {25, &this->models.grass_3, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[26] = {26, &this->models.grass_4, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[27] = {27, &this->models.grass_5, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[28] = {28, &this->models.grass_6, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[29] = {29, &this->models.grass_7, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[30] = {30, &this->models.grass_8, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[31] = {31, &this->models.grass_9, &this->shaders.textureShader, TEXTURE, 0.0f};
        Lookup[32] = {32, &this->models.road, &this->shaders.textureShader, MATERIAL, 0.0f};
        // Lookup[33] = {33, &this->models.bear, &this->shaders.materialShader, MATERIAL, 0.0f};
        // Lookup[34] = {34, &this->models.boar, &this->shaders.materialShader, MATERIAL, 0.0f};
        // Lookup[35] = {35, &this->models.deer_1, &this->shaders.materialShader, MATERIAL, 0.0f};
        // Lookup[36] = {36, &this->models.deer_2, &this->shaders.materialShader, MATERIAL, 0.0f};
        // Lookup[37] = {37, &this->models.fox, &this->shaders.materialShader, MATERIAL, 0.0f};
        // Lookup[38] = {38, &this->models.hedhog, &this->shaders.materialShader, MATERIAL, 0.0f};
        // Lookup[39] = {39, &this->models.owl, &this->shaders.materialShader, MATERIAL, 0.0f};
        // Lookup[40] = {40, &this->models.rabbit, &this->shaders.materialShader, MATERIAL, 0.0f};
        // Lookup[41] = {41, &this->models.squirrel, &this->shaders.materialShader, MATERIAL, 0.0f};
        // Lookup[42] = {42, &this->models.wolf, &this->shaders.materialShader, MATERIAL, 0.0f};



        Lookup[99] = {99, &this->models.note, &this->shaders.textureShader, TEXTURE, 0.0f};
    }

    void genInstanceBuffers()
    {
        for(int i=0; i < 100; i++)
        {
            if(Lookup[i].model == NULL)
            {
                break;
            }
            unsigned int instanceBuffer;
            glGenBuffers(1, &instanceBuffer);
            bufferIds[i] = instanceBuffer;
        }
    }

    void DrawAllModels(Shader &shader, vector<Object> *objects, vector<Light> *lights, DirLight *dirLight, FogSystem *fog)
    {
        for(int i = 0; i < 100; ++i)
        {
            if(Lookup[i].model == NULL)
            {
                break;
            }

            ID_Entry entry = Lookup[i];
            vector<mat4> modelMatrices;
            for(int objInd = 0; objInd < objects->size(); ++objInd)
            {
                if(objects->at(objInd).id == entry.ID)
                {
                    modelMatrices.push_back(objects->at(objInd).matrix);
                }
            }
            glBindBuffer(GL_ARRAY_BUFFER, bufferIds[i]);
            glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0], GL_STREAM_DRAW);

            for(int i = 0; i < entry.model->meshes.size(); i++)
            {
                unsigned int instanceVAO = entry.model->meshes[i].VAO;
                glBindVertexArray(instanceVAO);
                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
                glEnableVertexAttribArray(4);
                glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
                glEnableVertexAttribArray(5);
                glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
                glEnableVertexAttribArray(6);
                glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

                glVertexAttribDivisor(3, 1);
                glVertexAttribDivisor(4, 1);
                glVertexAttribDivisor(5, 1);
                glVertexAttribDivisor(6, 1);

                glBindVertexArray(0); 
            }

            // Drawing
            shader.bind();
            {
                mat4 projection = camera.GetProjectionMatrix();
                mat4 view = camera.GetViewMatrix();
                shader.setMat4("projection", projection);
                shader.setMat4("view", view);
                shader.setVec3("viewPos", camera.Position);

                shader.setInt("texture_diffuse1", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, entry.model->textures_loaded[0].id);

                shader.setFloat("maxFogDistance", fog->maxDistance);
                shader.setFloat("minFogDistance", fog->minDistance);
                shader.setVec4("fogColor", fog->color);

                dirLight->Render(shader);

                shader.setInt("size", lights->size());
                for (int i = 0; i < lights->size(); ++i)
                {
                    lights->at(i).Render(shader, i);
                }

                for(int i = 0; i < entry.model->meshes.size(); i++)
                {
                    //entry.model->meshes[i].SetTextureParams(shader);
                    glBindVertexArray(entry.model->meshes[i].VAO);
                    glDrawElementsInstanced(GL_TRIANGLES, 
                            static_cast<unsigned int>(entry.model->meshes[i].indices.size()),
                            GL_UNSIGNED_INT, 0,
                            modelMatrices.size());
                    glBindVertexArray(0);
                }
                glActiveTexture(GL_TEXTURE0);
            }
            shader.unbind();
        }
    }

    ID_Entry findbyId(int id) {
        return Lookup[id];
    }
};

#endif
