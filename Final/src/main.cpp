// Author: Alex Hartford, Brett Hickman, Lucas Li
// Program: Experience
// File: Main
// Date: April 2022

/* TODO
 * Gamepad Support
 *
 * Editor
 *  - Compass
 *  - Point Lights Presets in ID System
 *  - Be able to see which object you are editing.
 *  - Color Picker
 *  - DirLight in Level Loader
 *  - Top down view for quick editing.
 *  - Mass Delete
 *  - Undo/Redo
 *  - Raycasting - Point and Click.
 *    IDEAS
 *    - Outline object with some light color.
 *    - Show object type in GUI.
 *
 * Distance Fog
 * Fog Clouds
 * Water
 *
 * Particles
 *
 * Level Transitions
 *  - Fog at the edges.
 *  - Fade to White, then load other level, then fade back in.
 *
 * Instanced Rendering
 *  - Grass
 *  - Flowers with Noise
 *
 * Collisions
 *
 * Note Pickup Render Text to Screen.
 *
 * Soundtrack
 * NOTE: Look into MiniAudio's Extended Functionality
 *  - Ambient Sounds.
 *  - Spatial Sounds.
 *    - Birds
 *    - Water
 *    - Note Makes a directional Sound.
 *    - Fog Wall sound.
 *
 * Ideas
 *  - Dunes Walking Trail
 *  - Trail on the Heightmap.
 *    - Start in the Woods, no Path.
 *  - Wind Waker Wind
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

using namespace std;
using namespace glm;

// NOTE(Alex): Global State!
#define PI 3.1415

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 800;

const unsigned int TEXT_SIZE = 16;

enum EditorModes { MOVEMENT, GUI };

int EditorMode = MOVEMENT;

#include "camera.h"

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

// My Headers
#include "shader.h"
#include "manager.h"
#include "object.h"
#include "light.h"
#include "text.h"
#include "skybox.h"
#include "frustum.h"
#include "model.h"
#include "terrain.h"
#include "level.h"

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void RenderDebugText(TextRenderer Text, Manager m);

float randFloat()
{
    float r = rand() / static_cast<float>(RAND_MAX);
    return r;
}

float lerp(float a, float b, float x)
{
    return a + (b - a) * x;
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

#if 0
    // Full Screen Mode
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Experience", glfwGetPrimaryMonitor(), NULL);
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




    // Manager Object. Loads all Shaders, Models, Geometry.
    Manager m;

    // Populating Object List
    vector<Object> objects;

    vector<Light> lights;

    // Default value.
    DirLight dirLight = DirLight(vec3(0.0f, 0.0f, 1.0f),   // Direction
                                 vec3(0.4f, 0.2f, 0.2f),   // Ambient
                                 vec3(0.8f, 0.6f, 0.6f),   // Diffuse
                                 vec3(0.5f, 0.3f, 0.3f));  // Specular

    level lvl;
    lvl.LoadLevel("../levels/level1.txt", &objects, &lights, &dirLight, &m);

    Frustum frustum;

    // Sound System
    ma_engine_play_sound(&musicEngine, "../resources/audio/bach.mp3", NULL);

    bool showLightEditor = true;
    bool showDirLightEditor = true;
    bool showObjectEditor = true;

    bool snapToTerrain = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ++frameCount;

        drawnObjects = 0;

        processInput(window);

        // TODO(Alex): Interpolate this value for smooth movement.
        if(camera.Mode == WALK || camera.Mode == SPRINT)
            camera.Position.y = m.terrains.dunes.heightAt(camera.Position.x + 128.0f, camera.Position.z + 128.0f) + 5.0f;

        ma_engine_set_volume(&sfxEngine, SFXVolume);
        ma_engine_set_volume(&musicEngine, MusicVolume);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mat4 projection = perspective(radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
        mat4 view = camera.GetViewMatrix();
        mat4 model;

        frustum.ExtractVFPlanes(projection, view);
        
        // Render Light Positions (DEBUG)
        m.shaders.lightShader.bind();
        {
            m.shaders.lightShader.setMat4("projection", projection);
            m.shaders.lightShader.setMat4("view", view);

            for (int i = 0; i < lights.size(); ++i)
            {
                model = mat4(1.0f);
                model = scale(model, vec3(0.5f, 0.5f, 0.5f));
                model = translate(model, lights[i].position);
                m.shaders.lightShader.setMat4("model", model);
                m.models.box.Draw(m.shaders.lightShader);
            }
        }
        m.shaders.lightShader.unbind();

        // Render Material Objects
        m.shaders.materialShader.bind();
        {
            m.shaders.materialShader.setMat4("projection", projection);
            m.shaders.materialShader.setMat4("view", view);
            m.shaders.materialShader.setVec3("viewPos", camera.Position);

            dirLight.Render(m.shaders.materialShader);

            m.shaders.materialShader.setInt("size", lights.size());
            for(int i = 0; i < lights.size(); ++i)
            {
                lights[i].Render(m.shaders.materialShader, i);
            }

            for(int i = 0; i < objects.size(); ++i)
            {
                int id = objects[i].id;
                if(m.findbyId(id).shader_type == MATERIAL)
                {
                    if(!frustum.ViewFrustCull(objects[i].position, objects[i].width_radius))
                    {
                        objects[i].Draw();
                        drawnObjects++;
                    }
                }
            }

            m.shaders.materialShader.setVec3("material.ambient", 0.5f, 0.8f, 0.5f);
            m.shaders.materialShader.setVec3("material.diffuse", 0.5f, 0.8f, 0.5f);
            m.shaders.materialShader.setVec3("material.specular", 1.0f, 1.0f, 1.0f);
            m.shaders.materialShader.setFloat("material.shine", 1.0f); 

            // Render Terrain
            //m.terrains.dunes.Draw(m.shaders.materialShader, camera);
        }
        m.shaders.materialShader.unbind();

        // Render Textured Objects
        m.shaders.textureShader.bind();
        {
            m.shaders.textureShader.setMat4("projection", projection);
            m.shaders.textureShader.setMat4("view", view);
            m.shaders.textureShader.setVec3("viewPos", camera.Position);

            dirLight.Render(m.shaders.textureShader);

            m.shaders.textureShader.setInt("size", lights.size());
            for(int i = 0; i < lights.size(); ++i)
            {
                lights[i].Render(m.shaders.textureShader, i);
            }

            for(int i = 0; i < objects.size(); i++)
            {
                int id = objects[i].id;
                if(m.findbyId(id).shader_type == TEXTURE)
                {
                    if(!frustum.ViewFrustCull(objects[i].position, objects[i].width_radius))
                    {
                        objects[i].Draw();
                        drawnObjects++;
                    }
                }
            }
        }
        m.shaders.textureShader.unbind();


        // Render Skybox
        m.skyboxes.daySkybox.Draw(m.shaders.skyboxShader);

        // Render Text
        Text.RenderText("You will die.", m.shaders.typeShader, 25.0f, 25.0f, 2.0f, vec3(0.5, 0.8, 0.2));
        RenderDebugText(Text, m);

        if(EditorMode == GUI)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            static int objectPointer = 0;
            static int lightPointer = 0;

            if(showLightEditor)
            {
                ImGui::Begin("Light Editor");
                for (int n = 0; n < lights.size(); ++n)
                {
                    char buffer[256];
                    sprintf(buffer, "%d", n);
                    if(ImGui::Button(buffer))
                        lightPointer = n;
                    ImGui::SameLine();
                }
                ImGui::NewLine();
                ImGui::Text("Light = %d. Position = (%f %f %f)", lightPointer, lights[lightPointer].position.x, lights[lightPointer].position.y, lights[lightPointer].position.z);

                ImGui::SliderFloat3("Position", (float *)&lights[lightPointer].position, -128.0f, 128.0f);

                ImGui::SliderFloat3("Ambient", (float *)&lights[lightPointer].ambient, 0.0f, 1.0f);
                ImGui::SliderFloat3("Diffuse", (float *)&lights[lightPointer].diffuse, 0.0f, 1.0f);
                ImGui::SliderFloat3("Specular", (float *)&lights[lightPointer].specular, 0.0f, 1.0f);

                ImGui::SliderFloat("Constant", (float *)&lights[lightPointer].constant, 0.0f, 1.0f);
                ImGui::SliderFloat("Linear", (float *)&lights[lightPointer].linear, 0.0f, 1.0f);
                ImGui::SliderFloat("Quadratic", (float *)&lights[lightPointer].quadratic, 0.0f, 1.0f);

                if(ImGui::Button("Delete Light"))
                {
                    lights.erase(lights.begin() + lightPointer);
                    lightPointer--;
                    if(lightPointer > lights.size())
                        lightPointer = lights.size() - 2;
                }
                
                if(ImGui::Button("Create Light"))
                {
                    lights.push_back(Light(0, vec3(0.0f), vec3(0.05f), vec3(1.0f), vec3(0.4f),
                                           1.0f, 0.09f, 0.032f));
                    lightPointer = lights.size() - 1;
                }
                ImGui::End();
            }

            if(showDirLightEditor)
            {
                ImGui::Begin("DirLight Editor");
                ImGui::SliderFloat3("Direction", (float *)&dirLight.direction, -1.0f, 1.0f);

                ImGui::SliderFloat3("Ambient", (float *)&dirLight.ambient, 0.0f, 1.0f);
                ImGui::SliderFloat3("Diffuse", (float *)&dirLight.diffuse, 0.0f, 1.0f);
                ImGui::SliderFloat3("Specular", (float *)&dirLight.specular, 0.0f, 1.0f);
                ImGui::End();
            }

            if(showObjectEditor)
            {
                ImGui::Begin("Object Editor");
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

		ImGui::Checkbox("Terrain Snap", &snapToTerrain);

                if(ImGui::SliderFloat("Pos.x", (float *)&objects[objectPointer].position.x, -128.0f, 128.0f))
		{
		    if(snapToTerrain)
		    {
			objects[objectPointer].UpdateY(&m.terrains.dunes);
		    }
		}
                ImGui::SliderFloat("Pos.y", (float *)&objects[objectPointer].position.y, -128.0f, 128.0f);
                if(ImGui::SliderFloat("Pos.z", (float *)&objects[objectPointer].position.z, -128.0f, 128.0f))
		{
		    if(snapToTerrain)
		    {
			objects[objectPointer].UpdateY(&m.terrains.dunes);
		    }
		}

                ImGui::SliderFloat("AngleX", (float *)&objects[objectPointer].angleX, -PI, PI);
                ImGui::SliderFloat("AngleY", (float *)&objects[objectPointer].angleY, -PI, PI);
                ImGui::SliderFloat("AngleZ", (float *)&objects[objectPointer].angleZ, -PI, PI);

                ImGui::SliderFloat("Scale", (float *)&objects[objectPointer].scaleFactor, 0.0f, 5.0f);

                if(ImGui::Button("Delete Object"))
                {
                    objects.erase(objects.begin() + objectPointer);
                    objectPointer--;
                    if(objectPointer > objects.size())
                        objectPointer = objects.size() - 2;
                }
                
                if(ImGui::Button("Create Tree"))
                {
                    objects.push_back(Object(0,
                                             vec3(0.0f), -1.6f, 0.0f, 0.0f, 
                                             vec3(1), 1, 20, 1.0f, &m));
                    objectPointer = objects.size() - 1;
                }
                ImGui::SameLine();
                if(ImGui::Button("Create Rock"))
                {
                    objects.push_back(Object(3,
                                             vec3(0.0f), 0.0f, 0.0f, 0.0f, 
                                             vec3(1), 1, 1, 1.0f, &m));
                    objectPointer = objects.size() - 1;
                }
                ImGui::SameLine();
                if(ImGui::Button("Create Forest"))
                {
                    for(int i=0;i<10;i++){
                        objects.push_back(Object(0,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, 0.0f, 0.0f, 
                                                 vec3(1), 1, 20, randFloat()*1.5f,  &m));
                        objectPointer = objects.size() - 1;
                    }
                    for(int i=0;i<10;i++){
                        objects.push_back(Object(1,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, 0.0f, 0.0f, 
                                                 vec3(1), 1, 20, randFloat()*1.5f,  &m));
                        objectPointer = objects.size() - 1;
                    }
                    for(int i=0;i<10;i++){
                        objects.push_back(Object(2,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, 0.0f, 0.0f, 
                                                 vec3(1), 1, 20, randFloat()*1.5f,  &m));
                        objectPointer = objects.size() - 1;
                    }
                    for(int i=0;i<10;i++){
                        objects.push_back(Object(3,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, 0.0f, 0.0f, 
                                                 vec3(1), 1, 20, randFloat()*1.5f,  &m));
                        objectPointer = objects.size() - 1;
                    }
                    for(int i=0;i<10;i++){
                        objects.push_back(Object(4,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, 0.0f, 0.0f, 
                                                 vec3(1), 1, 20, randFloat()*1.5f,  &m));
                        objectPointer = objects.size() - 1;
                    }
                    for(int i=0;i<10;i++){
                        objects.push_back(Object(5,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, 0.0f, 0.0f, 
                                                 vec3(1), 1, 20, randFloat()*1.5f,  &m));
                        objectPointer = objects.size() - 1;
                    }
                    for(int i=0;i<10;i++){
                        objects.push_back(Object(6,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, 0.0f, 0.0f, 
                                                 vec3(1), 1, 20, randFloat()*1.5f,  &m));
                        objectPointer = objects.size() - 1;
                    }
                    for(int i=0;i<10;i++){
                        objects.push_back(Object(7,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, 0.0f, 0.0f, 
                                                 vec3(1), 1, 20, randFloat()*1.5f,  &m));
                        objectPointer = objects.size() - 1;
                    }
                    for(int i=0;i<10;i++){
                        objects.push_back(Object(8,
                                                 vec3((randFloat()*200.0f)-100.0f, 0.0f, (randFloat()*200.0f)-100.0f), 
                                                 -1.6f, 0.0f, 0.0f, 
                                                 vec3(1), 1, 20, randFloat()*1.5f,  &m));
                        objectPointer = objects.size() - 1;
                    }
                }

                ImGui::End();
            }

            ImGui::Begin("Level Editor");
                ImGui::Checkbox("Light Editor", &showLightEditor);                
                ImGui::Checkbox("DirLight Editor", &showDirLightEditor);                
                ImGui::Checkbox("Object Editor", &showObjectEditor);                

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate); 

                if(ImGui::Button("Save")) 
                {
                    lvl.SaveLevel("../levels/level1.txt", &objects, &lights, &dirLight);
                    ImGui::Text("Level saved.");
                }

                if(ImGui::Button("Load"))
                {
                    lvl.LoadLevel("../levels/level1.txt", &objects, &lights, &dirLight, &m);
                    ImGui::Text("Level loaded."); 
                }

            ImGui::End();

            ImGui::Render();
            glViewport(0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

void RenderDebugText(TextRenderer Text, Manager m)
{
    unsigned int lineNumber = 1;
    char buffer[256];
    sprintf(buffer, "%d ms (%d FPS)", (int)(1000 * deltaTime), (int)(1.0f / deltaTime));
    Text.RenderText(buffer, m.shaders.typeShader, 0.0f, SCREEN_HEIGHT - (TEXT_SIZE * lineNumber), 1.0f, vec3(0.5, 0.8, 0.2));
    lineNumber++;

    sprintf(buffer, "Drawn Objects: %d", drawnObjects);
    Text.RenderText(buffer, m.shaders.typeShader, 0.0f, SCREEN_HEIGHT - (TEXT_SIZE * lineNumber), 1.0f, vec3(0.5, 0.8, 0.2));
    lineNumber++;

    sprintf(buffer, "This is a debug message");
    Text.RenderText(buffer, m.shaders.typeShader, 0.0f, SCREEN_HEIGHT - (TEXT_SIZE * lineNumber), 1.0f, vec3(0.5, 0.8, 0.2));
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

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && camera.Mode == FREE)
        camera.Mode = FAST;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && camera.Mode == FAST)
        camera.Mode = FREE;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && camera.Mode == WALK)
        camera.Mode = SPRINT;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && camera.Mode == SPRINT)
        camera.Mode = WALK;

    if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        camera.Mode = WALK;
    if(glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
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
        firstMouse = true;
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
