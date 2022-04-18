// Author: Alex Hartford
// Program: Experience
// File: Main
// Date: March 2022

/* TODO
 * Ground Geometry
 *  - Walking on it.
 *  - Trees/rocks rendering at proper height
 *
 * Gamepad Support
 *
 * Particles
 *
 * GUI Library
 *
 * Level Editor
 *  - Level Saving
 *  - Level Loading
 *  - Loading
 *  - Geometry Modification
 *    - Palette
 *    - Placement
 *    - Translation
 *    - Scale
 *    - Rotation
 *    - Color Editing
 *
 * Instanced Rendering
 */

#include <iostream>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 800;

const unsigned int TEXT_SIZE = 16;

#include "shader.h"
Shader textureShader;  // Render Textured Meshes
Shader materialShader; // Render Material Meshes
Shader typeShader;     // Render Text on Screen
Shader skyboxShader;   // Render a Cubemap Skybox
Shader lightShader;    // <DEBUG> Render the physical locations of lights
Shader heightShader;   // Render a Heightmap as Terrain

// My Headers
#include "camera.h"
#include "heightmap.h"
#include "model.h"
#include "object.h"
#include "light.h"
#include "text.h"
#include "skybox.h"
#include "frustum.h"

using namespace std;
using namespace glm;

void processInput(GLFWwindow *window, Heightmap *map);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void RenderDebugText(TextRenderer Text);

Camera camera(vec3(0.0f, 0.0f, 3.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_WIDTH / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
unsigned int frameCount = 0;

const float MusicVolume = 1.0f;
const float SFXVolume = 0.1f;

int main(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Base", NULL, NULL);
    if(window == NULL)
    {
    cout << "Failed to create GLFW window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD.\n";
        return -1;
    }

    /* Manage OpenGL State */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Text Rendering */
    TextRenderer Text = TextRenderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    Text.Load("../resources/verdanab.ttf", TEXT_SIZE);

    /* Miniaudio */
    ma_result result;
    ma_engine musicEngine;
    if(ma_engine_init(NULL, &musicEngine) != MA_SUCCESS) {
        cout << "Failed to initialize audio engine.\n";
        return -1;
    }
    ma_engine sfxEngine;
    if(ma_engine_init(NULL, &sfxEngine) != MA_SUCCESS) {
        cout << "Failed to initialize audio engine.\n";
        return -1;
    }

    /* Shader Compilation */
    textureShader.init("../shaders/texture_vert.glsl", "../shaders/texture_frag.glsl");
    materialShader.init("../shaders/material_vert.glsl", "../shaders/material_frag.glsl");
    typeShader.init("../shaders/type_vert.glsl", "../shaders/type_frag.glsl");
    skyboxShader.init("../shaders/cubemap_vert.glsl", "../shaders/cubemap_frag.glsl");
    lightShader.init("../shaders/light_vert.glsl", "../shaders/light_frag.glsl");
    heightShader.init("../shaders/height_vert.glsl", "../shaders/height_frag.glsl");

    /* Geometry Loading */
    Skybox blueSkybox("../resources/daysky/", false);
    Skybox nightSkybox("../resources/nightsky/", false);

    Heightmap dunes("../resources/heightmap.png");

    // For textures that are wonky.
    stbi_set_flip_vertically_on_load(true);
    Model backpack("../resources/backpack/backpack.obj");
    stbi_set_flip_vertically_on_load(false);

    Model bonfire("../resources/dark_souls_bonfire/scene.gltf");

    Model box("../resources/cube.obj");

    Model skullModel("../resources/skull.obj");

    /* Object List */
    vector<Object> objects;
    for(int i = 0; i<20;i++)
    objects.push_back(Object(&skullModel, &materialShader, 
                vec3(-20+(i*2), 1.0f, -4.0f), 0.0f, vec3(1.0f), vec3(1), 1, 1, vec3(1.0f)));          

    /* Sound and Lighting */
    ma_engine_play_sound(&musicEngine, "../resources/bach.mp3", NULL);
    LightSystem lightSystem = LightSystem(camera);
    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ++frameCount;

        processInput(window, &dunes);
        ma_engine_set_volume(&sfxEngine, SFXVolume);
        ma_engine_set_volume(&musicEngine, MusicVolume);


        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mat4 projection = perspective(radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
        mat4 view = camera.GetViewMatrix();
        mat4 model;

        Frustum frustum(projection, view);

        /* Render Terrain */
        dunes.Draw(heightShader, camera);

        /* Render Light Positions (DEBUG) */
        lightShader.bind();
        {
            lightShader.setMat4("projection", projection);
            lightShader.setMat4("view", view);

            for(int i = 0; i < NUM_POINT_LIGHTS; ++i)
            {
                model = mat4(1.0f);
                model = scale(model, vec3(0.5f, 0.5f, 0.5f));
                model = translate(model, pointLightPositions[i]);
                lightShader.setMat4("model", model);
                box.Draw(lightShader);
            }
        }
        lightShader.unbind();


        /* Render Material Objects */
        materialShader.bind();
        {
            materialShader.setMat4("projection", projection);
            materialShader.setMat4("view", view);
            materialShader.setVec3("viewPos", camera.Position);

            materialShader.setVec3("material.ambient", 0.31f, 0.1f, 1.0f);
            materialShader.setVec3("material.diffuse", 0.31f, 0.1f, 1.0f);
            materialShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
            materialShader.setFloat("material.shine", 32.0f); 

            lightSystem.Render(materialShader);
            
            for(int i=0; i<objects.size();i++)
            {
                objects[i].Draw();
            }
        }
        materialShader.unbind();


        /* Render Textured Objects */
        textureShader.bind();
        {
            textureShader.setMat4("projection", projection);
            textureShader.setMat4("view", view);
            textureShader.setVec3("viewPos", camera.Position);

            lightSystem.Render(textureShader);

            model = mat4(1.0f);
            model = translate(model, vec3(0.0f, 1.0f, 0.0f));
            textureShader.setMat4("model", model);
            backpack.Draw(textureShader);

        }    
        textureShader.unbind();


        /* Render Skybox */
        //nightSkybox.Draw(skyboxShader, camera);

        /* Render Text */
        Text.RenderText("You will die.", typeShader, 25.0f, 25.0f, 2.0f, vec3(0.5, 0.8, 0.2));
        RenderDebugText(Text);

        /* Present Render */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

void RenderDebugText(TextRenderer Text)
{
    unsigned int lineNumber = 1;
    char buffer[256];
    sprintf(buffer, "%d ms (%d FPS)", (int)(1000 * deltaTime), (int)(1.0f / deltaTime));
    Text.RenderText(buffer, typeShader, 0.0f, SCREEN_HEIGHT - (TEXT_SIZE * lineNumber), 1.0f, vec3(0.5, 0.8, 0.2));
    lineNumber++;

    sprintf(buffer, "This is a debug message.");
    Text.RenderText(buffer, typeShader, 0.0f, SCREEN_HEIGHT - (TEXT_SIZE * lineNumber), 1.0f, vec3(0.5, 0.8, 0.2));
    lineNumber++;
}

void processInput(GLFWwindow *window, Heightmap *map)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.Mode = FAST;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        camera.Mode = FREE;

    camera.Position.y = map->heightAt(camera.Position.x, camera.Position.y);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
