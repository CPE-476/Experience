// Author: Alex Hartford, Brett Hickman, Lucas Li
// Program: Experience
// File: Main
// Date: May 2022

#include <iostream>
#include <time.h>
#include <unordered_map>

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

#define PI 3.14159265

// NOTE(Alex): Global State!
const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 800;
const unsigned int TEXT_SIZE = 16;
const float PLAYER_HEIGHT = 2.0f;
const float default_scale = 1.0f;

#include "camera.h"
Camera camera(vec3(25.0f, 25.0f, 25.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_WIDTH / 2.0f;
bool  firstMouse = true;

float        deltaTime = 0.0f;
float        lastFrame = 0.0f;
unsigned int frameCount = 0;

// For Selector.
vec3 selectorRay = vec3(0.0f);
bool checkInteraction = false;
bool drawNote = false;
bool pauseNote = false;

bool drawCollection = false;

// NOTE(Lucas) For collsion detection
vector<int> ignore_objects = {18, 23, 24, 25, 26, 27, 28, 29, 30, 31};

enum EditorModes
{
    MOVEMENT,
    GUI
};

int EditorMode = MOVEMENT;

int drawnObjects;

enum ShaderTypes
{
    MATERIAL,
    TEXTURE
};

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shine;
};

struct FogSystem
{
    float maxDistance;
    float minDistance;
    vec4 color;
};

// Project Headers
#include "shader.h"
#include "model.h"
#include "manager.h"
#include "object.h"
#include "light.h"
#include "level.h"
#include "text.h"
#include "skybox.h"
#include "terrain.h"
#include "frustum.h"
#include "particles.h"
#include "note.h"
#include "boundary.h"
#include "water.h"
#include "spline.h"
#include "sound.h"

using namespace std;
using namespace glm;

void processInput(GLFWwindow *window, vector<Object> *objects, vector<Sound *> *sounds);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

float randFloat()
{
    float r = rand() / static_cast<float>(RAND_MAX);
    return r;
}

float randCoord()
{
    return (randFloat() * 220.0f) - 100.0f;
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
    if (window == NULL)
    {
        cout << "Failed to create GLFW window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
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

    // Manager Object. Loads Shaders, Models.
    Manager m;

    vector<Object> objects;
    vector<Light> lights;
    vector<Emitter> emitters;
    vector<Sound *> sounds;
    vector<Note> notes;
    vector<bool> discoveredNotes;

    // Notes
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/testing/grass.jpg"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/aurelius1.png"));
    discoveredNotes.push_back(false);

    // Sounds
    Sound whistle = Sound("../resources/audio/whistle.wav", 1.0f, false);
    Sound rock = Sound("../resources/audio/desert.wav", vec3(25, 0, 0), 1.0f, 5.0f, 2.0f, 50.0f, true, false);
    Sound welcome = Sound("../resources/audio/welcome.wav", vec3(-50, 0, 0), 1.0f, 50.0f, 2.0f, 10.0f, true, false);
    Sound music = Sound("../resources/audio/BGM/愛にできることはまだあるかい.mp3", 0.1f, true);
    Sound alert = Sound("../resources/audio/alert.wav", 1.0f, false);
    Sound walk = Sound("../resources/audio/step.wav", 0.5f, false);

    // DEBUG
    sounds.push_back(&walk);
    sounds.push_back(&whistle);
    sounds.push_back(&rock);
    sounds.push_back(&welcome);
    sounds.push_back(&music);
    sounds.push_back(&alert);

    Skybox skybox;
    stbi_set_flip_vertically_on_load(false);
    // Default value.
    skybox.init("../resources/skyboxes/sunsky/", false);

    Terrain terrain;
    // Default value.
    terrain.init("../resources/heightmaps/lake.jpeg", 16.0f,
                 vec3(0.676, 0.691, 0.484),
                 vec3(0.459, 0.525, 0.275),
                 vec3(0.25, 0.129, 0.000));

    // Default value.
    DirLight dirLight = DirLight(vec3(0.0f, 0.0f, 1.0f),  // Direction
                                 vec3(0.4f, 0.2f, 0.2f),  // Ambient
                                 vec3(0.8f, 0.6f, 0.6f),  // Diffuse
                                 vec3(0.5f, 0.3f, 0.3f)); // Specular

    // Default value
    FogSystem fog = {200.0f, 15.0f, vec4(0.4f, 0.4f, 0.4f, 1.0f)};

    Level lvl;

    Boundary bound;
    bound.init(vec3(1.0f, 1.0f, 1.0f));

    lvl.LoadLevel("../levels/base.txt", &objects, &lights,
                  &dirLight, &emitters, &fog, &skybox, &terrain, 
		  &bound);

    Frustum frustum;

    Spline spline;

    Water water;
    water.gpuSetup();

    // Editor Settings
    bool showParticleEditor = false;
    bool showLightEditor = false;
    bool showDirLightEditor = false;
    bool showFogEditor = false;
    bool showTerrainEditor = false;
    bool showSkyboxEditor = false;
    bool showObjectEditor = false;
    bool showBoundaryEditor = false;
    bool showNoteEditor = false;
    bool showSoundEditor = false;

    bool snapToTerrain = true;

    bool drawTerrain = true;
    bool drawSkybox = false;
    bool drawBoundingSpheres = true;
    bool drawCollisionSpheres = false;
    bool drawPointLights = false;
    bool drawParticles = false;

    char levelName[128] = "";
    char skyboxPath[128] = "";
    char terrainPath[128] = "";
    char object_id[3] = "";

    int selectedObject = 0;
    int interactingObject = 0;
    int selectedLight = 0;
    int selectedParticle = 0;
    int selectedNote = 0;
    int selectedSound = 0;

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        {
            whistle.setPitch(randFloat()*0.5 + 0.75);
            whistle.startSound();
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
	{
            whistle.reset();
	}

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ++frameCount;

        drawnObjects = 0;

        // Input Resolution
        glfwPollEvents();
        processInput(window, &objects, &sounds);
        if (camera.Mode == WALK)
        {
            float dist = sqrt((abs(camera.Position.x) * abs(camera.Position.x)) + (abs(camera.Position.z) * abs(camera.Position.z)));
            if(dist > terrain.widthExtent - 2)
            {
                cout << "Boundary Collision. Loading Next Level.\n";
                lvl.LoadLevel(lvl.nextLevel, &objects, &lights, &dirLight,
                              &emitters, &fog, &skybox, &terrain, 
			      &bound);
                bound.counter = 157;
                bound.active = true;
            }
            camera.Position.y = terrain.heightAt(camera.Position.x, camera.Position.z) + PLAYER_HEIGHT;
        }

        // Spline
        spline.update(deltaTime);
        if(spline.active)
        {
            camera.Position = spline.getPosition();
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 projection = perspective(radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
        mat4 view = camera.GetViewMatrix();
        mat4 model;

        frustum.ExtractVFPlanes(projection, view);

        m.DrawAllModels(&objects, &lights, &dirLight, &fog);
 
        // Render Skybox
        if (drawSkybox)
        {
            skybox.Draw(m.shaders.skyboxShader);
        }

        // Render Light Positions (DEBUG)
        m.shaders.lightShader.bind();
        {
            m.shaders.lightShader.setMat4("projection", projection);
            m.shaders.lightShader.setMat4("view", view);

            model = mat4(1.0f);
            m.shaders.lightShader.setMat4("model", model);

            if (drawPointLights)
            {
                for (int i = 0; i < lights.size(); ++i)
                {
                    model = mat4(1.0f);
                    model = scale(model, vec3(0.5f, 0.5f, 0.5f));
                    model = translate(model, lights[i].position);
                    m.shaders.lightShader.setMat4("model", model);
                    m.models.cube.Draw(m.shaders.lightShader);
                }
            }

            if (drawBoundingSpheres)
            {
                for (int i = 0; i < objects.size(); ++i)
                {
                    model = mat4(1.0f);
                    model = translate(model, objects[i].position);
                    model = scale(model, vec3(objects[i].view_radius));
                    m.shaders.lightShader.setMat4("model", model);
                    m.models.sphere.Draw(m.shaders.lightShader);
                }
            }

            if (drawCollisionSpheres)
            {
                for (int i = 0; i < objects.size(); ++i)
                {
                    model = mat4(1.0f);
                    model = translate(model, objects[i].position);
                    model = scale(model, vec3(objects[i].collision_radius));
                    m.shaders.lightShader.setMat4("model", model);
                    m.models.sphere.Draw(m.shaders.lightShader);
                }
            }

            m.shaders.lightShader.setFloat("time", glfwGetTime() * 5);
            objects[selectedObject].Draw(&m.shaders.lightShader, m.findbyId(objects[selectedObject].id).model, m.findbyId(objects[selectedObject].id).shader_type);
        }
        m.shaders.lightShader.unbind();

        // Render Terrain
        if (drawTerrain)
        {
            terrain.Draw(m.shaders.terrainShader, &lights, &dirLight, &fog);
        }

        water.Draw(m.shaders.waterShader, deltaTime);

        if(drawParticles)
        {
            for (int i = 0; i < emitters.size(); ++i)
            {
                emitters[i].Draw(m.shaders.particleShader, deltaTime, terrain.width);
            }
        }

        if(bound.active)
        {
            bound.Draw(m.shaders.transShader);
            if(bound.counter == 314)
            {
                bound.active = false;
            }
            bound.counter++;
        }
        bound.DrawWall(m.shaders.boundaryShader, terrain.width / 2.0f, 8.0f, &m.models.cylinder);

        // Render Note
        if(checkInteraction)
        {
            interactingObject = 0;
            for (int i = 0; i < objects.size(); ++i)
            {
                // Ray collision detection.
                vec3 p = camera.Position - objects[i].position; // The vector pointing from us to an object.
                float rSquared = objects[i].view_radius * objects[i].view_radius;
                float p_d = dot(p, selectorRay); // Calculated to see if the object is behind us.

                if (p_d > 0 || dot(p, p) < rSquared) // If the object is behind us or surrounding the starting point:
                    continue;                        // No collision.

                vec3 a = p - p_d * selectorRay; // Treat a as a plane passing through the object's center perpendicular to the ray.

                float aSquared = dot(a, a);

                if (aSquared > rSquared) // If our closest approach is outside the sphere:
                    continue;            // No collision.

                if(EditorMode == MOVEMENT)
                {
                    interactingObject = i;
                }
                else
                {
                    selectedObject = i;
                }
            }
            if(objects[interactingObject].interactible)
            {
                drawNote = true;
                selectedNote = objects[interactingObject].noteNum;
                discoveredNotes[selectedNote] = true;
            }
            checkInteraction = false;
        }

        if(drawNote)
        {
            notes[selectedNote].Update();
            notes[selectedNote].Draw(m.shaders.noteShader);
        }

        if(drawCollection)
        {
            int xInd = 0;
            int yInd = 0;
            for(int i = 0; i < notes.size(); ++i)
            {
                xInd = i % 4;
                yInd = i / 4;
                if(discoveredNotes[i]) // Draw only discovered notes.
                {
                    notes[i].DrawSmall(m.shaders.noteShader, xInd, yInd);
                }
            }
        }

        if (EditorMode == GUI)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (showParticleEditor)
            {
                ImGui::Begin("Particle Editor");
                for (int n = 0; n < emitters.size(); ++n)
                {
                    char buffer[256];
                    sprintf(buffer, "%d", n);
                    if (ImGui::Button(buffer))
                        selectedParticle = n;
                    ImGui::SameLine();
                }
                ImGui::NewLine();
                ImGui::Text("Particle = %d. Position = (%.02f %.02f %.02f)", selectedParticle, emitters[selectedParticle].startPosition.x, emitters[selectedParticle].startPosition.y, emitters[selectedParticle].startPosition.z);

                ImGui::SliderFloat3("Position", (float *)&emitters[selectedParticle].startPosition, -128.0f, 128.0f);
                ImGui::SliderFloat3("Velocity", (float *)&emitters[selectedParticle].startVelocity, -50.0f, 50.0f);
                ImGui::SliderFloat("Gravity", (float *)&emitters[selectedParticle].gravity, -100.0f, 100.0f);
                ImGui::SliderFloat("Bottom Radius", (float *)&emitters[selectedParticle].radius, 0.0f, 10.0f);
                ImGui::SliderFloat("Top Radius", (float *)&emitters[selectedParticle].radiusTop, 0.0f, 10.0f);
                ImGui::ColorEdit4("Start Color", (float *)&emitters[selectedParticle].startColor);
                ImGui::ColorEdit4("End Color", (float *)&emitters[selectedParticle].endColor);
                ImGui::SliderFloat("Start Scale", (float *)&emitters[selectedParticle].startScale, 0.0f, 20.0f);
                ImGui::SliderFloat("End Scale", (float *)&emitters[selectedParticle].endScale, 0.0f, 20.0f);
                // NOTE(Alex): Broken, for some reason.
                // ImGui::SliderInt("Amount", (int *)&emitters[selectedParticle].particleAmount, 0, 9999);
                ImGui::SliderInt("Bug Mode", (int *)&emitters[selectedParticle].bugMode, 0, 1);
                ImGui::SliderInt("Fog Mode", (int *)&emitters[selectedParticle].fogMode, 0, 1);


                if (ImGui::Button("Create Emitter"))
                {
                    emitters.push_back(
                        Emitter("../resources/models/particle/part.png", 20000, vec3(0, 10, 0), 0.2, 3, 7.0f, vec3(3, 10, 3), 2.0f, -9.81f, vec4(1.0f, 0.0f, 0, 1), vec4(0.0f, 0.0f, 1.0f, 1.0f), 1, 0));
                    selectedParticle = emitters.size() - 1;
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete Emitter"))
                {
                    emitters.erase(emitters.begin() + selectedParticle);
                    selectedParticle--;
                    if (selectedParticle > emitters.size())
                        selectedParticle = emitters.size() - 2;
                }

                if (ImGui::Button("Fire"))
                {
                    emitters.push_back(
                        Emitter("../resources/models/particle/part.png", 1000, vec3(0, 0, 0), 1.4, 0.5, 4.5f, vec3(0, 5, 0), 1.4f, 0.0f, vec4(1.0, 1.0f, 0.7, 0.7), vec4(1.0, 0.4, 0, 0.9), 1, 0));
                    selectedParticle = emitters.size() - 1;
                }
                ImGui::SameLine();
                if (ImGui::Button("Smoke"))
                {
                    emitters.push_back(
                        Emitter("../resources/models/particle/part.png", 200, vec3(0, 0, 0), 1, 1, 4.5f, vec3(1, 5, 1), 4.0f, 0.0f, vec4(0.5, 0.5, 0.5, 1), vec4(1, 1, 1, 1), 0, 5));
                    selectedParticle = emitters.size() - 1;
                }
                ImGui::SameLine();
                if (ImGui::Button("Bugs"))
                {
                    emitters.push_back(
                        Emitter("../resources/models/particle/part.png", 1000, vec3(0, 0, 0), 100, 100, 4.5f, vec3(0, 0.1, 0), 1.0f, -0.81f, vec4(1.0f, 0.8f, 0, 1), vec4(0.8, 1.0, 0.0, 0), 0, 0.5));
                    selectedParticle = emitters.size() - 1;
                    emitters[selectedParticle].bugMode = 1;
                }
                ImGui::SameLine();
                if (ImGui::Button("Rain"))
                {
                    emitters.push_back(
                        Emitter("../resources/models/particle/part.png", 10000, vec3(0, 0, 0), 100, 10, 4.5f, vec3(5, 0, 5), 7.0f, -9.81f, vec4(0.5f, 0.5f, 1.0, 1), vec4(0.0f, 0.0f, 1.0f, 1.0f), 0, 0.5));
                    selectedParticle = emitters.size() - 1;
                }
                ImGui::End();
            }

            if (showLightEditor)
            {
                ImGui::Begin("Light Editor");
                for (int n = 0; n < lights.size(); ++n)
                {
                    char buffer[256];
                    sprintf(buffer, "%d", n);
                    if (ImGui::Button(buffer))
                        selectedLight = n;
                    ImGui::SameLine();
                }
                ImGui::NewLine();
                ImGui::Text("Light = %d. Position = (%.02f %.02f %.02f)", selectedLight, lights[selectedLight].position.x, lights[selectedLight].position.y, lights[selectedLight].position.z);

                ImGui::SliderFloat3("Position", (float *)&lights[selectedLight].position, -128.0f, 128.0f);

                ImGui::ColorEdit3("Ambient", (float *)&lights[selectedLight].ambient);
                ImGui::ColorEdit3("Diffuse", (float *)&lights[selectedLight].diffuse);
                ImGui::ColorEdit3("Specular", (float *)&lights[selectedLight].specular);

                ImGui::SliderFloat("Constant", (float *)&lights[selectedLight].constant, 0.0f, 1.0f);
                ImGui::SliderFloat("Linear", (float *)&lights[selectedLight].linear, 0.0f, 1.0f);
                ImGui::SliderFloat("Quadratic", (float *)&lights[selectedLight].quadratic, 0.0f, 1.0f);

                if (ImGui::Button("Delete Light"))
                {
                    lights.erase(lights.begin() + selectedLight);
                    selectedLight--;
                    if (selectedLight > lights.size())
                        selectedLight = lights.size() - 2;
                }
                ImGui::SameLine();
                if (ImGui::Button("Create Light"))
                {
                    lights.push_back(Light(vec3(0.0f), vec3(0.05f), vec3(1.0f), vec3(0.4f),
                                           1.0f, 0.09f, 0.032f));
                    selectedLight = lights.size() - 1;
                }

                if (ImGui::Button("Firelight"))
                {
                    lights.push_back(Light(vec3(0.0f), vec3(0.05f),
                                           vec3(1.0f, 0.3f, 0.0f), vec3(0.4f, 0.2f, 0.0f),
                                           0.5f, 1.0f, 1.0f));
                    selectedLight = lights.size() - 1;
                }
                ImGui::End();
            }

            if (showDirLightEditor)
            {
                ImGui::Begin("DirLight Editor");
                ImGui::SliderFloat3("Direction", (float *)&dirLight.direction, -1.0f, 1.0f);

                ImGui::ColorEdit3("Ambient", (float *)&dirLight.ambient);
                ImGui::ColorEdit3("Diffuse", (float *)&dirLight.diffuse);
                ImGui::ColorEdit3("Specular", (float *)&dirLight.specular);
                ImGui::End();
            }

            if (showFogEditor)
            {
                ImGui::Begin("Fog Editor");

                ImGui::SliderFloat("Max", (float *)&fog.maxDistance, 1.0f, 1000.0f);
                ImGui::SliderFloat("Min", (float *)&fog.minDistance, 0.0f, 1000.0f);
                ImGui::ColorEdit3("Color", (float *)&fog.color);
                ImGui::End();
            }

            if (showSkyboxEditor)
            {
                ImGui::Begin("Skybox Editor");

                ImGui::InputText("Name", skyboxPath, IM_ARRAYSIZE(skyboxPath));
                if (ImGui::Button("Update"))
                {
                    skybox.init("../resources/skyboxes/" + string(skyboxPath) + "/");
                }
                ImGui::End();
            }

            if (showTerrainEditor)
            {
                ImGui::Begin("Terrain Editor");

                ImGui::InputText("Name", terrainPath, IM_ARRAYSIZE(terrainPath));
                ImGui::SliderFloat("Y Scale", &terrain.yScale, 0.0f, 100.0f);

                if (ImGui::Button("Update"))
                {
                    terrain.init("../resources/heightmaps/" + string(terrainPath), terrain.yScale,
                            terrain.bottom, terrain.top, terrain.dirt);
                }

                ImGui::ColorEdit3("Bottom", (float *)&terrain.bottom);
                ImGui::ColorEdit3("Top", (float *)&terrain.top);
                ImGui::ColorEdit3("Dirt", (float *)&terrain.dirt);
                ImGui::SliderFloat("Water Level", (float *)&water.height, -6.0f, 6.0f);
                ImGui::End();
            }

            if (showBoundaryEditor)
            {
                ImGui::Begin("Boundary Editor");
                ImGui::ColorEdit3("Color", (float *)&bound.color);
                ImGui::End();
            }

            if (showNoteEditor)
            {
                ImGui::Begin("Note Editor");
                for (int n = 0; n < notes.size(); ++n)
                {
                    char buffer[256];
                    sprintf(buffer, "%d", n);
                    if (ImGui::Button(buffer))
                        selectedNote = n;
                    ImGui::SameLine();
                }
                ImGui::NewLine();
                ImGui::Text("Note = %d.", selectedNote);

                ImGui::SliderFloat("Scale", (float *)&notes[selectedNote].scl, 0.0f, 5.0f);
                ImGui::SliderFloat2("Position", (float *)&notes[selectedNote].pos, -1.0f, 1.0f);

                ImGui::End();
            }

	    if (showSoundEditor)
	    {
		ImGui::Begin("Sound Editor");
                for (int n = 0; n < sounds.size(); ++n)
                {
                    char buffer[256];
                    sprintf(buffer, "%d", n);
                    if (ImGui::Button(buffer))
                        selectedSound = n;
                    ImGui::SameLine();
                }
                ImGui::NewLine();
                ImGui::Text("%s\nSound = %d. Position = (%.02f %.02f %.02f)", sounds[selectedSound]->path.c_str(), selectedSound, sounds[selectedSound]->pos.x, sounds[selectedSound]->pos.y, sounds[selectedSound]->pos.z);

		if(ImGui::Button("Play"))
		{
		    cout << "In play button\n";
		    sounds[selectedSound]->startSound();
		}
		if(ImGui::Button("Pause"))
		{
		    sounds[selectedSound]->stopSound();
		}
		if(ImGui::Button("Reset"))
		{
		    sounds[selectedSound]->reset();
		}

                ImGui::SliderFloat3("Position", (float *)&sounds[selectedSound]->pos, -128.0f, 128.0f);
                ImGui::SliderFloat("Volume", (float *)&sounds[selectedSound]->volume, 0.0f, 1.0f);
                ImGui::SliderFloat("Rolloff", (float *)&sounds[selectedSound]->rolloff, 0.0f, 100.0f);
                ImGui::SliderFloat("Min", (float *)&sounds[selectedSound]->minDistance, 0.0f, 100.0f);
                ImGui::SliderFloat("Max", (float *)&sounds[selectedSound]->maxDistance, 0.0f, 100.0f);
		ImGui::Checkbox("Looping?", &sounds[selectedSound]->isLooping);
		ImGui::Checkbox("Music?", &sounds[selectedSound]->isMusic);
		ImGui::Checkbox("Has Played?", &sounds[selectedSound]->hasPlayed);

		ImGui::End();
	    }

            if (showObjectEditor)
            {
                ImGui::Begin("Object Editor");
                ImGui::Text("Object = %d/%lu. Position = (%.02f %.02f %.02f)", selectedObject, objects.size(),
                            objects[selectedObject].position.x, objects[selectedObject].position.y, objects[selectedObject].position.z);

                ImGui::Checkbox("Terrain Snap", &snapToTerrain);

                if (ImGui::SliderFloat("Pos.x", (float *)&objects[selectedObject].position.x, -128.0f, 128.0f))
                {
                    if (snapToTerrain)
                    {
                        objects[selectedObject].position.y = terrain.heightAt(objects[selectedObject].position.x,
                                                                              objects[selectedObject].position.z);
                    }
                    objects[selectedObject].UpdateModel();
                }
                if (ImGui::SliderFloat("Pos.y", (float *)&objects[selectedObject].position.y, -128.0f, 128.0f))
                {
                    objects[selectedObject].UpdateModel();
                }
                if (ImGui::SliderFloat("Pos.z", (float *)&objects[selectedObject].position.z, -128.0f, 128.0f))
                {
                    if (snapToTerrain)
                    {
                        objects[selectedObject].position.y = terrain.heightAt(objects[selectedObject].position.x,
                                                                              objects[selectedObject].position.z);
                    }
                    objects[selectedObject].UpdateModel();
                }

                if (ImGui::SliderFloat("AngleX", (float *)&objects[selectedObject].angleX, -PI, PI))
                    objects[selectedObject].UpdateModel();
                if (ImGui::SliderFloat("AngleY", (float *)&objects[selectedObject].angleY, -PI, PI))
                    objects[selectedObject].UpdateModel();
                if (ImGui::SliderFloat("AngleZ", (float *)&objects[selectedObject].angleZ, -PI, PI))
                    objects[selectedObject].UpdateModel();

                if (ImGui::SliderFloat("Scale", (float *)&objects[selectedObject].scaleFactor, 0.0f, 5.0f)){
                    objects[selectedObject].UpdateModel();
                    objects[selectedObject].collision_radius = m.findbyId(objects[selectedObject].id).collision_radius * objects[selectedObject].scaleFactor;
                }

                ImGui::Checkbox("Interactible?", &objects[selectedObject].interactible);

                ImGui::SliderInt("Note", &objects[selectedObject].noteNum, 0, 20);

                if (ImGui::SliderFloat("Collison Radius", (float *)&objects[selectedObject].collision_radius, 0.0f, 50.0f))
                    objects[selectedObject].UpdateModel();

                if (ImGui::Button("Delete Object"))
                {
                    objects.erase(objects.begin() + selectedObject);
                }

                if (ImGui::Button("Delete All"))
                {
                    while (objects.size() > 1)
                    {
                        objects.erase(objects.begin() + objects.size() - 1);
                    }
                }

                ImGui::InputText("Object", object_id, IM_ARRAYSIZE(object_id));
                ImGui::SameLine();
                if (ImGui::Button("Add"))
                {
                    int id = atof(object_id);
                    float cr = m.findbyId(id).collision_radius;
                    objects.push_back(Object(id,
                                             vec3(camera.Position.x,
                                                  terrain.heightAt(camera.Position.x, camera.Position.z),
                                                  camera.Position.z),
                                             -1.6f, 0.0f, 0.0f,
                                             vec3(1), 1, cr * default_scale, 
                                             default_scale, false, 0));
                    selectedObject = objects.size() - 1;
                }

                if (ImGui::Button("Copy"))
                {
                    int id = objects[selectedObject].id;
                    float cr = m.findbyId(id).collision_radius;
                    objects.push_back(Object(id,
                                             vec3(camera.Position.x,
                                                  terrain.heightAt(camera.Position.x, camera.Position.z),
                                                  camera.Position.z),
                                             -1.6f, 0.0f, 0.0f,
                                             vec3(1), 1, cr * default_scale, 
                                             default_scale, false, 0));
                    selectedObject = objects.size() - 1;
                }
                if (ImGui::Button("Forest"))
                {
                    float pos_y = 0.0f;
                    float small_scale = 0.05f;
                    float grass_scale = (randFloat()* 0.5) + 0.03;

                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);
                        
                        objects.push_back(Object(0,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), m.findbyId(0).model->MaximumExtent, 
                                                 m.findbyId(0).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(1,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(1).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(2,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(2).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(3,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(3).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(4,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(4).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(5,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(5).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(6,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(6).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(7,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(7).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(8,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(8).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(16,
                                                 pos,
                                                 0.0f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(16).collision_radius * small_scale, 
                                                 small_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(17,
                                                 pos,
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(8).collision_radius * default_scale, 
                                                 default_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        float pos_x = randCoord();
                        float pos_z = randCoord();
                        if (snapToTerrain)
                            pos_y = terrain.heightAt(pos_x, pos_z);
                        vec3 pos = vec3(pos_x, pos_y, pos_z);

                        objects.push_back(Object(18,
                                                 pos,
                                                 0.0f, 0.0f, 0.0f,
                                                 vec3(1), 1, m.findbyId(18).collision_radius * small_scale, 
                                                 small_scale, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    // for (int i = 0; i < 5; i++){
                    //     for (int j = 33; j < 43; j++){
                    //         float pos_x = randCoord();
                    //         float pos_z = randCoord();
                    //         if (snapToTerrain)
                    //             pos_y = terrain.heightAt(pos_x, pos_z);
                    //         vec3 pos = vec3(pos_x, pos_y, pos_z);
                    //         objects.push_back(Object(j,
                    //                                  pos,
                    //                                  -1.6f, 0.0f, 0.0f,
                    //                                  vec3(1), 1, m.findbyId(j).collision_radius * default_scale, 
                    //                                  default_scale));
                    //         selectedObject = objects.size() - 1;
                    //     }
                    // }
                    for (int i = 0; i < 2000; i++)
                    {
                        for (int j = 23; j < 32; j++)
                        {
                            int goodVal = 0;
                            float pos_x;
                            float pos_z;
                            while(!goodVal)
                            {
                            pos_x = randCoord();
                            pos_z = randCoord();
                            if (snapToTerrain){
                                pos_y = terrain.heightAt(pos_x, pos_z);
                                if(pos_y > (water.height + 0.6))
                                    goodVal = 1;
                            }
                            else
                                goodVal = 1;
                            }

                            vec3 pos = vec3(pos_x, pos_y, pos_z);

                            objects.push_back(Object(j,
                                                     pos,
                                                     -1.6f, 0.0f, 0.0f,
                                                     vec3(1), 0.03, m.findbyId(j).collision_radius * grass_scale, 
                                                     default_scale, false, 0));
                            selectedObject = objects.size() - 1;
                        }
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Desert"))
                {
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(9,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 1, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(10,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 1, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(11,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 1, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(12,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 1, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(13,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 1, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(14,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 1, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(15,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 1, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(3,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 1, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(4,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 1, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(19,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 20, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(20,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 20, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        objects.push_back(Object(21,
                                                 vec3(randCoord(), 0.0f, randCoord()),
                                                 -1.6f, 0.0f, 0.0f,
                                                 vec3(1), 1, 20, 
                                                 randFloat() * 1.5f, false, 0));
                        selectedObject = objects.size() - 1;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Street"))
                {
                    for (int i = 0; i < 3; i++)
                    {
                        objects.push_back(Object(32,
                                                 vec3(0.0f, 0.0f, 0.0f + i * 250.0f),
                                                 0.0f, 0.0f, 0.0f,
                                                 vec3(1), 1, 20, 1.0f,
                                                 false, 0));
                        selectedObject = objects.size() - 1;
                    }
                }
                ImGui::End();
            }

            ImGui::Begin("Level Editor");

            ImGui::InputText("Name", levelName, IM_ARRAYSIZE(levelName));

            ImGui::SliderFloat3("Starting Position", (float *)&lvl.startPosition, -128.0f, 128.0f);
            ImGui::SliderFloat3("Starting Direction", (float *)&lvl.startDirection, -1.0f, 1.0f);

            if (ImGui::Button("Save"))
            {
                string str = "../levels/";
                str.append(levelName);
                lvl.SaveLevel(str, &objects, &lights, &dirLight,
                    &emitters, &fog, &skybox, &terrain, &bound);
                cout << "Level saved: " << str << "\n";
            }
            ImGui::SameLine();
            if (ImGui::Button("Load"))
            {
                string str = "../levels/";
                str.append(levelName);
                lvl.LoadLevel(str, &objects, &lights, &dirLight,
                              &emitters, &fog, &skybox, &terrain, 
			      &bound);
                cout << "Level loaded: " << str << "\n";
            }
            ImGui::SameLine();
            if (ImGui::Button("Set Next"))
            {
                string str = "../levels/";
                str.append(levelName);
                lvl.nextLevel = str;
            }
            ImGui::SameLine();
            if (ImGui::Button("Next"))
            {
                lvl.LoadLevel(lvl.nextLevel, &objects, &lights, &dirLight,
                              &emitters, &fog, &skybox, &terrain, 
			      &bound);
            }

            // Editors
            ImGui::Text("Editors");
            ImGui::Checkbox("Object", &showObjectEditor);
            ImGui::SameLine();
            ImGui::Checkbox("Particle", &showParticleEditor);
            ImGui::SameLine();
            ImGui::Checkbox("Light", &showLightEditor);
            ImGui::SameLine();
            ImGui::Checkbox("DirLight", &showDirLightEditor);

            ImGui::Checkbox("Skybox", &showSkyboxEditor);
            ImGui::SameLine();
            ImGui::Checkbox("Terrain", &showTerrainEditor);
            ImGui::SameLine();
            ImGui::Checkbox("Boundary", &showBoundaryEditor);
            ImGui::SameLine();
            ImGui::Checkbox("Fog", &showFogEditor);
            ImGui::SameLine();
            ImGui::Checkbox("Note", &showNoteEditor);

            ImGui::Checkbox("Sound", &showSoundEditor);

            ImGui::NewLine();

            // Settings
            ImGui::Text("Settings");
            ImGui::Checkbox("Draw Terrain", &drawTerrain);
            ImGui::SameLine();
            ImGui::Checkbox("Draw Skybox", &drawSkybox);
            ImGui::SameLine();
            ImGui::Checkbox("Draw Point Lights", &drawPointLights);

            ImGui::Checkbox("Draw Bounding Spheres", &drawBoundingSpheres);
            ImGui::SameLine();
            ImGui::Checkbox("Draw Collision Spheres", &drawCollisionSpheres);

            ImGui::Checkbox("Draw Particles", &drawParticles);

            ImGui::Text("%d ms (%d FPS)", (int)(1000 * deltaTime), (int)(1.0f / deltaTime));

            ImGui::Text("Drawn Objects: %d", drawnObjects);
            ImGui::Text("Location: (%.02f %.02f %.02f)", camera.Position.x, camera.Position.y, camera.Position.z);
            ImGui::Text("Orientation: (%.02f %.02f %.02f)", camera.Front.x, camera.Front.y, camera.Front.z);
            ImGui::Text("Level: %s | Next: %s", lvl.currentLevel.c_str(), lvl.nextLevel.c_str());

            if(ImGui::Button("SPLINE!"))
            {
                spline.init(camera.Position, vec3(0.0f, 3.0f, 0.0f), 1.0f);
                spline.active = true;
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

float objectDis(vec3 curPos, vec3 objectPos)
{
    return sqrt(pow(curPos.x - objectPos.x, 2) + pow(curPos.z - objectPos.z, 2));
}

bool Colliding(vector<Object> *objects)
{
    for (int i = 0; i < objects->size(); i++)
    {
        if (objectDis(camera.Position, objects->at(i).position) < objects->at(i).collision_radius)
        {
            return true;
        }
    }
    return false;
}

void processInput(GLFWwindow *window, vector<Object> *objects, vector<Sound *> *sounds)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
	drawCollection = true;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(FORWARD, deltaTime);
        if (camera.Mode == WALK && Colliding(objects))
        {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (camera.Mode == WALK && Colliding(objects))
        {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
        if (camera.Mode == WALK && Colliding(objects))
        {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
        if (camera.Mode == WALK && Colliding(objects))
        {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        }
    }

    if (camera.Mode == WALK && (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || 
                                glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || 
                                glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || 
                                glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS))
    {
        if(!ma_sound_is_playing(&(sounds->at(0)->sound)))
            ma_sound_start(&(sounds->at(0)->sound));
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && 
        glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && 
        glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE && 
        glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
    {
        ma_sound_stop(&(sounds->at(0)->sound));
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.Fast = true;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        camera.Fast = false;

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        camera.Mode = WALK;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        camera.Mode = FREE;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        EditorMode = MOVEMENT;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
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
    float xpos;
    float ypos;

    if (EditorMode == MOVEMENT)
    {
        xpos = static_cast<float>(xposIn);
        ypos = static_cast<float>(yposIn);

        if (firstMouse)
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

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos;
        double ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if(EditorMode == MOVEMENT)
        {
            xpos = SCREEN_WIDTH / 2.0f;
            ypos = SCREEN_HEIGHT / 2.0f;
        }

        GLbyte color[4];
        GLfloat depth;
        GLuint index;

        glReadPixels(xpos, SCREEN_HEIGHT - ypos - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
        glReadPixels(xpos, SCREEN_HEIGHT - ypos - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
        glReadPixels(xpos, SCREEN_HEIGHT - ypos - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

        vec4 viewport = vec4(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        vec3 wincoord = vec3(xpos, SCREEN_HEIGHT - ypos - 1, depth);
        vec3 objcoord = unProject(wincoord, camera.GetViewMatrix(), camera.GetProjectionMatrix(), viewport);

        selectorRay = normalize(objcoord - camera.Position);    
        if(!drawNote)
        {
            checkInteraction = true;
        }
        else
        {
            pauseNote = false;
        }
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (EditorMode == MOVEMENT)
    {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
