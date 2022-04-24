// Author: Alex Hartford
// Program: Experience
// File: Manager
// Date: April 2022

#ifndef MANAGER_H
#define MANAGER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

#include "shader.h"
#include "model.h"
#include "camera.h"
#include "skybox.h"
#include "terrain.h"

using namespace std;
using namespace glm;

// NOTE(Alex): If any objects are dynamically allocated, we need to have a destructor.

enum ShaderTypes {
    MATERIAL,
    TEXTURE
};

struct ID_Entry
{
    int ID;
    Model *model;

    Shader *shader;
    int shader_type;
};

struct Shader_Container
{
    Shader textureShader;
    Shader materialShader;
    Shader typeShader;
    Shader skyboxShader;
    Shader lightShader;
};

struct Skybox_Container
{
    Skybox daySkybox;
    Skybox nightSkybox;
};

struct Model_Container
{
    Model backpack;
    Model bonfire;
    Model box;
    Model skull;
    Model tree_1;
    Model tree_2;
    Model tree_3;
    Model tree_4;
    Model tree_5;
    Model rock_1;
    Model rock_2;
    Model rock_3;
    Model rock_4;
};

struct Terrain_Container
{
    Terrain dunes;
};

struct Manager
{
    Shader_Container shaders;
    Skybox_Container skyboxes;
    Model_Container models;
    Terrain_Container terrains;
    ID_Entry Lookup[100];

    Manager()
    {
        /* Shader Compilation */
        this->shaders.textureShader.init("../shaders/texture_vert.glsl", "../shaders/texture_frag.glsl");
        this->shaders.materialShader.init("../shaders/material_vert.glsl", "../shaders/material_frag.glsl");
        this->shaders.typeShader.init("../shaders/type_vert.glsl", "../shaders/type_frag.glsl");
        this->shaders.skyboxShader.init("../shaders/cubemap_vert.glsl", "../shaders/cubemap_frag.glsl");
        this->shaders.lightShader.init("../shaders/light_vert.glsl", "../shaders/light_frag.glsl");

        /* Geometry Loading */

        this->skyboxes.daySkybox.init("../resources/skyboxes/daysky/", false);

        this->skyboxes.nightSkybox.init("../resources/skyboxes/nightsky/", false);

        this->terrains.dunes.init("../resources/testing/heightmap.png");

        stbi_set_flip_vertically_on_load(true);
        this->models.backpack.init("../resources/models/backpack/backpack.obj");
        stbi_set_flip_vertically_on_load(false);
        this->models.box.init("../resources/testing/cube.obj");
        this->models.skull.init("../resources/testing/skull.obj");
        this->models.bonfire.init("../resources/models/dark_souls_bonfire/scene.gltf");
        this->models.tree_1.init("../resources/models/trees/tree_1.fbx");
        this->models.tree_2.init("../resources/models/trees/tree_2.fbx");
        this->models.tree_3.init("../resources/models/trees/tree_3.fbx");
        this->models.tree_4.init("../resources/models/trees/tree_4.fbx");
        this->models.tree_5.init("../resources/models/trees/tree_5.fbx");
        this->models.rock_1.init("../resources/models/rocks/rock_1.fbx");
        this->models.rock_2.init("../resources/models/rocks/rock_2.fbx");
        this->models.rock_3.init("../resources/models/rocks/rock_3.fbx");
        this->models.rock_4.init("../resources/models/rocks/rock_4.fbx");

        this->Populate();
    }

    void Populate()
    {
        Lookup[0] = {0, &this->models.tree_1, &this->shaders.textureShader, TEXTURE};
        Lookup[1] = {1, &this->models.tree_2, &this->shaders.textureShader, TEXTURE};
        Lookup[2] = {2, &this->models.tree_3, &this->shaders.textureShader, TEXTURE};
        Lookup[3] = {3, &this->models.tree_4, &this->shaders.textureShader, TEXTURE};
        Lookup[4] = {4, &this->models.tree_5, &this->shaders.textureShader, TEXTURE};
        Lookup[5] = {5, &this->models.rock_1, &this->shaders.textureShader, TEXTURE};
        Lookup[6] = {6, &this->models.rock_2, &this->shaders.textureShader, TEXTURE};
        Lookup[7] = {7, &this->models.rock_3, &this->shaders.textureShader, TEXTURE};
        Lookup[8] = {8, &this->models.rock_4, &this->shaders.textureShader, TEXTURE};

    }

    ID_Entry findbyId(int id) {
        return Lookup[id];
    }
};

#endif
