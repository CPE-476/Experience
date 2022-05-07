// Author: Alex Hartford, Lucas Li
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
#include "skybox.h"
#include "terrain.h"
#include "note.h"

using namespace std;
using namespace glm;

// NOTE(Alex): If any objects are dynamically allocated, we need to have a destructor.

/* TODO(Alex):
 *
 * Objects should have some predefined radii that we define.
 * (Culling Radius, Collision Radius)
 * Should we put this information in the Editor or in the Manager?
 * - Should be modified by scale.
 */

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
    Shader particleShader;
    Shader noteShader;
};

struct Skybox_Container
{
    Skybox daySkybox;
    Skybox nightSkybox;
};

struct Model_Container
{
    // testing
    Model backpack;
    Model cube;
    Model sphere;
    Model skull;
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

struct Terrain_Container
{
    Terrain dunes;
};

struct Note_Container
{
    Note aurelius1;
};

struct Manager
{
    Shader_Container shaders;
    Skybox_Container skyboxes;
    Model_Container models;
    Terrain_Container terrains;
    Note_Container notes;
    ID_Entry Lookup[100];

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

        this->notes.aurelius1.init("../resources/notes/aurelius1.png");

        /* Geometry Loading */
        this->skyboxes.daySkybox.init("../resources/skyboxes/daysky/", false);
        this->skyboxes.nightSkybox.init("../resources/skyboxes/nightsky/", false);

        this->terrains.dunes.init("../resources/testing/final-dunes-first-try.png");

        stbi_set_flip_vertically_on_load(true);
        // this->models.backpack.init("../resources/models/backpack/backpack.obj");
        stbi_set_flip_vertically_on_load(false);
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
        Lookup[9] = {9, &this->models.rock_5, &this->shaders.textureShader, TEXTURE};
        Lookup[10] = {10, &this->models.rock_6, &this->shaders.textureShader, TEXTURE};
        Lookup[11] = {11, &this->models.rock_7, &this->shaders.textureShader, TEXTURE};
        Lookup[12] = {12, &this->models.rock_8, &this->shaders.textureShader, TEXTURE};
        Lookup[13] = {13, &this->models.rock_9, &this->shaders.textureShader, TEXTURE};
        Lookup[14] = {14, &this->models.rock_10, &this->shaders.textureShader, TEXTURE};
        Lookup[15] = {15, &this->models.rock_11, &this->shaders.textureShader, TEXTURE};
        Lookup[16] = {16, &this->models.campfire, &this->shaders.textureShader, TEXTURE};
        Lookup[17] = {17, &this->models.snail, &this->shaders.textureShader, TEXTURE};
        Lookup[18] = {18, &this->models.fern, &this->shaders.textureShader, TEXTURE};
        Lookup[19] = {19, &this->models.cactus_1, &this->shaders.textureShader, TEXTURE};
        Lookup[20] = {20, &this->models.cactus_2, &this->shaders.textureShader, TEXTURE};
        Lookup[21] = {21, &this->models.cactus_3, &this->shaders.textureShader, TEXTURE};
        Lookup[22] = {22, &this->models.trumbleweed, &this->shaders.textureShader, TEXTURE};
        Lookup[23] = {23, &this->models.grass_1, &this->shaders.textureShader, TEXTURE};
        Lookup[24] = {24, &this->models.grass_2, &this->shaders.textureShader, TEXTURE};
        Lookup[25] = {25, &this->models.grass_3, &this->shaders.textureShader, TEXTURE};
        Lookup[26] = {26, &this->models.grass_4, &this->shaders.textureShader, TEXTURE};
        Lookup[27] = {27, &this->models.grass_5, &this->shaders.textureShader, TEXTURE};
        Lookup[28] = {28, &this->models.grass_6, &this->shaders.textureShader, TEXTURE};
        Lookup[29] = {29, &this->models.grass_7, &this->shaders.textureShader, TEXTURE};
        Lookup[30] = {30, &this->models.grass_8, &this->shaders.textureShader, TEXTURE};
        Lookup[31] = {31, &this->models.grass_9, &this->shaders.textureShader, TEXTURE};
        Lookup[32] = {32, &this->models.road, &this->shaders.textureShader, TEXTURE};


    }

    ID_Entry findbyId(int id) {
        return Lookup[id];
    }
};

#endif
