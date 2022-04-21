// Author: Alex Hartford
// Program: Experience
// File: Main
// Date: April 2022

/* TODO
 *
 * ID System With one format specifier in level loader.
 *
 * Ground Geometry
 *  - Movement
 *  - Heightmap
 * 
 * Particles
 *
 * Cave Transition
 *  - Plane Transition
 *
 * Gamepad Support
 *
 * Level Editor
 *  - Palette
 *  - Mouse Placement?
 *
 * Instanced Rendering
 *
 * Create main GetProjection and GetView Matrix function, 
 * so we don't have to pass camera around.
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
#include "object.h"
#include "light.h"
#include "text.h"
#include "skybox.h"
#include "frustum.h"
#include "model.h"
#include "heightmap.h"
#include "level.h"

using namespace std;
using namespace glm;

enum EditorModes
{
    MOVEMENT,
    GUI
};

int EditorMode = MOVEMENT;

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void RenderDebugText(TextRenderer Text);

Camera camera(vec3(0.0f, 0.0f, 3.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_WIDTH / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
unsigned int frameCount = 0;

const float MusicVolume = 0.1f;
const float SFXVolume = 0.1f;

int drawnObjects;

float randFloat(){
    float r = rand() / static_cast<float>(RAND_MAX);
    return r;
}

int main(void)
{
    srand(time(NULL));
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Experience", NULL, NULL);
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD.\n";
        return -1;
    }

    /* Manage OpenGL State */
    glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE); glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Text Rendering */
    TextRenderer Text = TextRenderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    Text.Load("../resources/verdanab.ttf", TEXT_SIZE);

    /* Miniaudio */
    ma_result result;
    ma_engine musicEngine;
    if (ma_engine_init(NULL, &musicEngine) != MA_SUCCESS)
    {
        cout << "Failed to initialize audio engine.\n";
        return -1;
    }
    ma_engine sfxEngine;
    if (ma_engine_init(NULL, &sfxEngine) != MA_SUCCESS)
    {
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
    stbi_set_flip_vertically_on_load(true);
    Model backpack("../resources/backpack/backpack.obj");
    stbi_set_flip_vertically_on_load(false);
    Model bonfire("../resources/dark_souls_bonfire/scene.gltf");
    Model box("../resources/cube.obj");
    Model skull("../resources/skull.obj");
    Model tree("../resources/low-poly-tree/source/Tree3.fbx");
    Model rock("../resources/stylized-lowpoly-rock/source/Rock.fbx");

    /* Populating Object List */
    vector<Object> objects;
    level lvl = level(&dunes, &backpack, &skull);
    lvl.LoadLevel("../levels/level1.txt", objects);

    Frustum frustum;

    /* Sound and Lighting */
    ma_engine_play_sound(&musicEngine, "../resources/bach.mp3", NULL);
    LightSystem lightSystem = LightSystem(camera);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ++frameCount;

        drawnObjects = 0;

        processInput(window);
        ma_engine_set_volume(&sfxEngine, SFXVolume);
        ma_engine_set_volume(&musicEngine, MusicVolume);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mat4 projection = perspective(radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
        mat4 view = camera.GetViewMatrix();
        mat4 model;

        frustum.ExtractVFPlanes(projection, view);

        /* Render Terrain */
        //dunes.Draw(heightShader, camera);
	
        /* Render Light Positions (DEBUG) */
        lightShader.bind();
        {
            lightShader.setMat4("projection", projection);
            lightShader.setMat4("view", view);

            for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
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

            lightSystem.Render(materialShader);

            for(int i = 0; i < objects.size(); i++)
            {
                if(objects[i].shader_type == MATERIAL)
                {
                    if(!frustum.ViewFrustCull(objects[i].position, objects[i].width_radius))
                    {
                        objects[i].Draw();
                        drawnObjects++;
                    }
                }
            }

            materialShader.setVec3("material.ambient", 0.5f, 0.8f, 0.5f);
            materialShader.setVec3("material.diffuse", 0.5f, 0.8f, 0.5f);
            materialShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
            materialShader.setFloat("material.shine", 1.0f); 

	    model = mat4(1.0f);
	    model = translate(model, vec3(0.0f, -51.0f, 0.0f));
	    model = scale(model, vec3(200.0f, 100.0f, 200.0f));
	    materialShader.setMat4("model", model);
	    box.Draw(materialShader);
        }
        materialShader.unbind();

        /* Render Textured Objects */
        textureShader.bind();
        {
            textureShader.setMat4("projection", projection);
            textureShader.setMat4("view", view);
            textureShader.setVec3("viewPos", camera.Position);

            lightSystem.Render(textureShader);

            for(int i = 0; i < objects.size(); i++)
            {
                if(objects[i].shader_type == TEXTURE)
                {
                    if(!frustum.ViewFrustCull(objects[i].position, objects[i].width_radius))
                    {
                        objects[i].Draw();
                        drawnObjects++;
                    }
                }
            }
        }
        textureShader.unbind();


        /* Render Skybox */
        blueSkybox.Draw(skyboxShader, camera);

        /* Render Text */
        Text.RenderText("You will die.", typeShader, 25.0f, 25.0f, 2.0f, vec3(0.5, 0.8, 0.2));
        RenderDebugText(Text);

        if(EditorMode == GUI)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                static float f = 0.0f;
                static int objectPointer = 0;

                ImGui::Begin("Experience Level Editor"); // Create a window and append into it

                    ImGui::ColorEdit3("Ambient", (float *)&objects[objectPointer].material.ambient);
                    ImGui::ColorEdit3("Diffuse", (float *)&objects[objectPointer].material.diffuse);
                    ImGui::ColorEdit3("Specular", (float *)&objects[objectPointer].material.specular);
                    ImGui::SliderFloat("Shine", (float *)&objects[objectPointer].material.shine, 0.0f, 32.0f);

                    ImGui::SliderFloat("Pos.x", (float *)&objects[objectPointer].position.x, -256.0f, 256.0f);
                    ImGui::SliderFloat("Pos.z", (float *)&objects[objectPointer].position.z, -256.0f, 256.0f);

                    if(ImGui::Button("X Rotation"))
                    {
                        objects[objectPointer].rotation = vec3(1.0f, 0.0f, 0.0f);
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Y Rotation"))
                    {
                        objects[objectPointer].rotation = vec3(0.0f, 1.0f, 0.0f);
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Z Rotation"))
                    {
                        objects[objectPointer].rotation = vec3(0.0f, 0.0f, 1.0f);
                    }
                    ImGui::SliderFloat("Angle", (float *)&objects[objectPointer].angle, 0.0f, 1.0f);

                    ImGui::SliderFloat("X Scale", (float *)&objects[objectPointer].Scale.x, 0.0f, 5.0f);
                    ImGui::SliderFloat("Y Scale", (float *)&objects[objectPointer].Scale.y, 0.0f, 5.0f);
                    ImGui::SliderFloat("Z Scale", (float *)&objects[objectPointer].Scale.z, 0.0f, 5.0f);

                    for (int n = 0; n < objects.size(); ++n)
                    {
                        char buffer[256];
                        sprintf(buffer, "%d", n);
                        if(ImGui::Button(buffer))
                            objectPointer = n;
                        ImGui::SameLine();
                    }
                    ImGui::NewLine();

                    ImGui::Text("Object = %d. Position = (%f %f %f)", objectPointer, objects[objectPointer].position.x, objects[objectPointer].position.y, objects[objectPointer].position.z);

                    if(ImGui::Button("Delete Object"))
                    {
                        objects.erase(objects.begin() + objectPointer);
                        objectPointer--;
                        if(objectPointer > objects.size())
                            objectPointer = objects.size() - 2;
                    }
                    
                    if(ImGui::Button("Create Tree"))
                    {
                        objects.push_back(Object(&tree, &textureShader, TEXTURE,
                                                 vec3(0.0f), 
                                                 -1.6f, vec3(1.0f, 0.0f, 0.0f), 
                                                 vec3(1), 1, 20, vec3(1.0f), "SKL", "TEX"));
                        objectPointer = objects.size() - 1;
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Create Rock"))
                    {
                        objects.push_back(Object(&rock, &materialShader, MATERIAL,
                                                 vec3(0.0f), 
                                                 0.0f, vec3(1.0f), 
                                                 vec3(1), 1, 1, vec3(1.0f), "BPK", "MAT"));
                        objectPointer = objects.size() - 1;
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Create Forest"))
                    {
			// DO NOT REMOVE. ESSENTIAL TO PROGRAM INTEGRITY.
			ma_engine_play_sound(&musicEngine, "../resources/beam.wav", NULL);
                        for(int i=0;i<100;i++){
                        objects.push_back(Object(&tree, &textureShader, TEXTURE,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, vec3(1.0f, 0.0f, 0.0f), 
                                                 vec3(1), 1, 20, vec3((randFloat())*2.0f), "SKL", "TEX"));
                        objectPointer = objects.size() - 1;
                        }
                        for(int i=0;i<100;i++){
                        objects.push_back(Object(&rock, &materialShader, MATERIAL,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 0.0f, vec3(1.0f), 
                                                 vec3(1), 1, 1, vec3((randFloat()) * 5.0f), "BPK", "MAT"));
                        objectPointer = objects.size() - 1;
                        }
                    }

                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate); 

                    if(ImGui::Button("Save")) 
                        lvl.SaveLevel("../levels/level1.txt", objects);

/*
                    ImGui::ColorEdit3("Light Direction", (float *)&lightSystem.dirLight.direction);
                    ImGui::ColorEdit3("LAmbient", (float *)&lightSystem.dirLight.ambient);
                    ImGui::ColorEdit3("LAmbient", (float *)&lightSystem.dirLight.specular);
                    ImGui::ColorEdit3("LAmbient", (float *)&lightSystem.dirLight.diffuse);
*/

                ImGui::End();

                ImGui::Render();
                glViewport(0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2); // TODO(Alex): Investigate why this is required.

                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
        }

        /* Present Render */
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);

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

    sprintf(buffer, "Drawn Objects: %d", drawnObjects);
    Text.RenderText(buffer, typeShader, 0.0f, SCREEN_HEIGHT - (TEXT_SIZE * lineNumber), 1.0f, vec3(0.5, 0.8, 0.2));
    lineNumber++;

    sprintf(buffer, "This is a debug message");
    Text.RenderText(buffer, typeShader, 0.0f, SCREEN_HEIGHT - (TEXT_SIZE * lineNumber), 1.0f, vec3(0.5, 0.8, 0.2));
    lineNumber++;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && camera.Mode == FREE)
        camera.Mode = FAST;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && camera.Mode == FAST)
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.Mode = FAST;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        camera.Mode = FREE;

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        EditorMode = MOVEMENT;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if(glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
    {
        EditorMode = GUI;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        camera.Mode = WALK;
    if(glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        camera.Mode = FREE;
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    if(EditorMode == MOVEMENT)
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
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if(EditorMode == MOVEMENT)
    {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
