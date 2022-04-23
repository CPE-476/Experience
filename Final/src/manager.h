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

/* TODO
 *
 * PROCESS:
 * See ID
 * Get
 *  - Model Ref
 *  - Shader Ref
 *  - Shader ID
 * Create Object
 *
 * Ideally, we only load an ID, and we only Save an ID too.
 *
 * NOTE: If any objects are dynamically allocated, we need to have a destructor.
 */

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
    Model tree;
    Model rock;
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

    Manager()
    {
        /* Shader Compilation */
        this->shaders.textureShader.init("../shaders/texture_vert.glsl", "../shaders/texture_frag.glsl");
        this->shaders.materialShader.init("../shaders/material_vert.glsl", "../shaders/material_frag.glsl");
        this->shaders.typeShader.init("../shaders/type_vert.glsl", "../shaders/type_frag.glsl");
        this->shaders.skyboxShader.init("../shaders/cubemap_vert.glsl", "../shaders/cubemap_frag.glsl");
        this->shaders.lightShader.init("../shaders/light_vert.glsl", "../shaders/light_frag.glsl");

        /* Geometry Loading */
        this->skyboxes.daySkybox.init("../resources/daysky/", false);
        this->skyboxes.nightSkybox.init("../resources/nightsky/", false);

        this->terrains.dunes.init("../resources/heightmap.png");

        stbi_set_flip_vertically_on_load(true);
        this->models.backpack.init("../resources/backpack/backpack.obj");
        stbi_set_flip_vertically_on_load(false);
        this->models.bonfire.init("../resources/dark_souls_bonfire/scene.gltf");
        this->models.box.init("../resources/cube.obj");
        this->models.skull.init("../resources/skull.obj");
        this->models.tree.init("../resources/low-poly-tree/source/Tree3.fbx");
        this->models.rock.init("../resources/stylized-lowpoly-rock/source/Rock.fbx");
    }

    ID_Entry Lookup[100];

    void Populate()
    {
        Lookup[0] = {0, &this->models.backpack, &this->shaders.textureShader, TEXTURE};
        Lookup[1] = {1, &this->models.skull, &this->shaders.materialShader, MATERIAL};
        Lookup[2] = {2, &this->models.tree, &this->shaders.textureShader, TEXTURE};
        Lookup[3] = {3, &this->models.rock, &this->shaders.materialShader, MATERIAL};
    }
};

#endif
