// Author: Alex Hartford, Brett Hickman, Lucas Li
// Program: Experience
// File: Main
// Date: June 2022

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
const unsigned int RETINA_SCREEN_WIDTH = 1280;
const unsigned int RETINA_SCREEN_HEIGHT = 800;
const unsigned int SCREEN_WIDTH = 2560;
const unsigned int SCREEN_HEIGHT = 1600;
const unsigned int TEXT_SIZE = 16;
const float PLAYER_HEIGHT = 1.4f;
const float default_scale = 1.0f;
const float default_view = 1.414f;

#include "camera.h"
Camera camera(vec3(25.0f, 25.0f, 25.0f));
float lastX = RETINA_SCREEN_WIDTH / 2.0f;
float lastY = RETINA_SCREEN_HEIGHT / 2.0f;
bool  firstMouse = true;
char  levelName[128] = "";

float        deltaTime = 0.0f;
float        lastFrame = 0.0f;
unsigned int frameCount = 0;
const float  y_offset = 1.0f;

int   bobbingCounter = 0;
int   bobbingSpeed = 7;
float bobbingAmount = 0.03;
float road_width = 3.0f;

// For Selector.
vec3  selectorRay = vec3(0.0f);
bool  checkInteraction = false;
bool  drawNote = false;
bool  drawCollection = false;
float collectionScroll = 0.0f;
bool  pauseNote = false;
float fog_offset = 7.5f;

// NOTE(Lucas) For collsion detection
vector<int> ignore_objects = {18, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};

// For FBO Stuff
bool  bloom = true;
float exposure = 1.0f;
float gBloomThreshold = 0.5f;

bool  gDONTCULL = false;

bool  gSelecting = false;

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
#include "particles.h"
#include "note.h"
#include "boundary.h"
#include "water.h"
#include "spline.h"
#include "sound.h"
#include "sun.h"

using namespace std;
using namespace glm;

void processInput(GLFWwindow *window, vector<Object> *objects, vector<Sound *> *sounds);

FloatSpline fspline;
Level lvl;

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

float randFloat()
{
    float r = rand() / static_cast<float>(RAND_MAX);
    return r;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;

void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions         // texture Coords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

float randCoord()
{
    return (randFloat() * 220.0f) - 100.0f;
}

float randCoordDes()
{
    return (randFloat() * 440.0f) - 200.0f;
}

float randRange(float min, float max)
{
    return ((randFloat() * (max - min)) + min);
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
    GLFWwindow *window = glfwCreateWindow(RETINA_SCREEN_WIDTH, RETINA_SCREEN_HEIGHT, "Experience", NULL, NULL);
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
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Text Rendering */
    TextRenderer Text = TextRenderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    Text.Load("../resources/verdanab.ttf", TEXT_SIZE);

    // Manager Object. Loads Shaders, Models.
    Manager m;

    vector<Object>  objects;
    vector<Light>   lights;
    vector<Emitter> emitters;
    vector<Sound *> sounds;
    vector<Note>    notes;
    vector<bool>    discoveredNotes;

    // Notes
    notes.push_back(Note("../resources/notes/note1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/note2.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/note3.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/note4.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/note5.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/note6.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/note7.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/note8.png"));
    discoveredNotes.push_back(false);

    notes.push_back(Note("../resources/notes/box1.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/box2.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/box3.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/box4.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/box5.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/box6.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/box7.png"));
    discoveredNotes.push_back(false);
    notes.push_back(Note("../resources/notes/box8.png"));
    discoveredNotes.push_back(false);

    // Sounds
    Sound whistle = Sound("../resources/audio/whistle.wav", 1.0f, false);
    Sound pickup = Sound("../resources/audio/pickup2.mp3", 1.0f, false);
    Sound hmm = Sound("../resources/audio/hmm.wav", 1.0f, false);
    Sound rock = Sound("../resources/audio/desert.wav", vec3(25, 0, 0), 1.0f, 5.0f, 2.0f, 50.0f, true, false);
    Sound fogAmb = Sound("../resources/audio/wind.wav", vec3(0, 0, 0), 0.3f, 0.0f, 50.0f, 100.0f, true, true);
    Sound music = Sound("../resources/audio/BGM/愛にできることはまだあるかい.mp3", 0.1f, true);
    Sound streetMusic = Sound("../resources/audio/BGM/Sunrise.mp3", 0.1f, true);
    Sound forestMusic1 = Sound("../resources/audio/BGM/Forest_1_calm.mp3", 0.1f, true);
    Sound desertMusic = Sound("../resources/audio/BGM/Desert.mp3", 0.1f, true);
    Sound alert = Sound("../resources/audio/alert.wav", 1.0f, false);
    Sound walk = Sound("../resources/audio/step.wav", 0.3f, false);
    Sound EVA = Sound("../resources/audio/EVA おめでとう最終話.mp3", 1.0f, false);
    Sound fire = Sound("../resources/audio/fire.mp3", vec3(-13.6, -3.799, 10.2), 1.0f, 7.0f, 1.0f, 10.0f, true, false);

    //Ambient Sounds
    Sound forestAmb = Sound("../resources/audio/bird.wav", vec3(0, 0, 0), 1.0f, 1.0f, 2.0f, 50.0f, true, false);

    sounds.push_back(&walk);
    sounds.push_back(&whistle);
    sounds.push_back(&pickup);
    sounds.push_back(&hmm);
    sounds.push_back(&rock);
    sounds.push_back(&fogAmb);
    sounds.push_back(&music);
    sounds.push_back(&alert);
    sounds.push_back(&EVA);
    sounds.push_back(&forestAmb);
    sounds.push_back(&fire);

    Skybox skybox;
    stbi_set_flip_vertically_on_load(false);
    // Default value.
    skybox.init("../resources/skyboxes/sunsky/", false);

    Terrain terrain;
    // Default value.
    terrain.init("../resources/heightmaps/lake.jpeg", 16.0f, 16.0f,
                 vec3(0.676, 0.691, 0.484),
                 vec3(0.459, 0.525, 0.275),
                 vec3(0.25, 0.129, 0.000));

    Sun sun = Sun(&m.models.sphere); 

    // Default value
    FogSystem fog = {200.0f, 15.0f, vec4(0.4f, 0.4f, 0.4f, 1.0f)};

    Boundary bound;
    bound.init(vec3(1.0f, 1.0f, 1.0f), -5.0f, terrain.width / 2.0f, 0.0f);

    lvl.LoadLevel("../levels/forest.txt", &objects, &lights,
                  &sun, &emitters, &fog, &skybox, &terrain, &bound);
    Frustum frustum;

    Water water;
    water.gpuSetup();

    Spline sunspline;
    Spline ambspline;


    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 4024, SHADOW_HEIGHT = 4024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // FBO shader configuration
    // --------------------
    m.shaders.shadowShader.bind();
    m.shaders.shadowShader.setInt("diffuseTexture", 0);
    m.shaders.shadowShader.setInt("shadowMap", 1);
    m.shaders.debugShader.bind();
    m.shaders.debugShader.setInt("depthMap", 0);

    m.shaders.blurShader.bind();
    m.shaders.blurShader.setInt("image", 0);
    m.shaders.bloomShader.bind();
    m.shaders.bloomShader.setInt("scene", 0);
    m.shaders.bloomShader.setInt("bloomBlur", 1);

    // Shadow info
    // -------------
    glm::vec3 lightPos(126.0f, 76.0f, -1.0F);
    float near_plane = 0.0f, far_plane = 345.0f;
    float shadow_frustum = 111.0f;

    // Color buffers to identify high brightness locations
    // -----------------------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for(int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Attach texture to FBO.
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);   // Depth Buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "Framebuffer Error!\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cout << "Framebuffer not complete!\n";
    }

    // Editor Settings
    bool showObjectEditor = true;
    bool showParticleEditor = false;
    bool showLightEditor = false;
    bool showFogEditor = false;
    bool showShadowEditor = false;
    bool showTerrainEditor = false;
    bool showSkyboxEditor = false;
    bool showBoundaryEditor = false;
    bool showNoteEditor = false;
    bool showSoundEditor = false;
    bool showSunEditor = false;

    bool snapToTerrain = true;

    bool drawTerrain = true;
    bool drawSkybox = true;
    bool drawBoundingSpheres = false;
    bool drawCollisionSpheres = false;
    bool drawSelectionSpheres = false;
    bool drawPointLights = true;
    bool drawParticles = true;

    bool toggleRenderEffects = false;

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

         if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
        {
            desertMusic.stopSound();
            forestMusic1.stopSound();
            streetMusic.startSound();
        }
        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_RELEASE)
        {
            streetMusic.reset();
        }
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
        {
            streetMusic.stopSound();
            forestMusic1.stopSound();
            desertMusic.startSound();
        }
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE)
        {
            desertMusic.reset();
        }
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
        {
            desertMusic.stopSound();
            streetMusic.stopSound();
            forestMusic1.startSound();
        }
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE)
        {
            forestMusic1.reset();
        }
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
        {
            desertMusic.stopSound();
            forestMusic1.stopSound();
            streetMusic.stopSound();
        }

        forestAmb.updateSound();
        fire.updateSound();
        fogAmb.updateSound();
        

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
            if(dist > bound.width - 2)
            {
                cout << "Boundary Collision. Loading Next Level.\n";
                lvl.LoadLevel(lvl.nextLevel, &objects, &lights, &sun,
                              &emitters, &fog, &skybox, &terrain, 
                              &bound);
                bound.counter = 157; 
                bound.active = true;
            }
            if(dist > bound.width - 50)
                fogAmb.volume = (dist-(bound.width-50))/(bound.width-50);
            else
                fogAmb.volume = 0.0f;
            camera.Position.y = terrain.heightAt(camera.Position.x, camera.Position.z) + PLAYER_HEIGHT + bobbingAmount * sin((float)bobbingCounter / (float)bobbingSpeed);
        }

        // Spline
        fspline.update(deltaTime);
        sunspline.update(deltaTime);
        ambspline.update(deltaTime);

        if(fspline.active)
        {
            camera.Zoom = fspline.getPosition();
        }
        if(sunspline.active)
        {
            sun.dirLight.direction = sunspline.getPosition();
        } 
        if(ambspline.active)
        {
            sun.dirLight.ambient = ambspline.getPosition();
        }
        sun.updateLight();



        if(toggleRenderEffects || EditorMode == MOVEMENT)
        {
            // Render Depth Map to its own FBO
            // -------------------------------------------------------------------
            glm::mat4 lightProjection, lightView;
            glm::mat4 lightSpaceMatrix;
            lightProjection = glm::ortho(-shadow_frustum, shadow_frustum, -shadow_frustum, shadow_frustum, near_plane, far_plane);
            lightView = glm::lookAt(vec3(sun.position.x, sun.position.y + sun.scale_factor, sun.position.z), 
                    glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjection * lightView;
            // render scene from light's point of view
            m.shaders.depthShader.bind();
            {
                m.shaders.depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
                m.shaders.depthShader.setBool("isT", false);

                glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
                glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
                glClear(GL_DEPTH_BUFFER_BIT);

                m.DrawAllModels(m.shaders.depthShader, &objects, &lights, &sun.dirLight, &fog, &frustum);

                m.shaders.depthShader.setBool("isT", true);
                // // Render Terrain
                // if (drawTerrain)
                // {
                //     terrain.Draw(m.shaders.depthShader, &lights, &sun.dirLight, &fog);
                // }
            }
            m.shaders.depthShader.unbind();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Let's start drawing actual geometry.
            // =====================================
            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            glEnable(GL_DEPTH_TEST);

            // reset viewport for actual Drawing.
            glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

            mat4 projection = camera.GetProjectionMatrix();
            mat4 view = camera.GetViewMatrix();
            mat4 model;

            frustum.ExtractVFPlanes(projection, view);

            m.shaders.shadowShader.bind();
            {
                m.shaders.shadowShader.setMat4("projection", projection);
                m.shaders.shadowShader.setMat4("view", view);
                m.shaders.shadowShader.setVec3("viewPos", camera.Position);
                m.shaders.shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
                m.shaders.shadowShader.setBool("isT", false);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, depthMap);
                m.DrawAllModels(m.shaders.shadowShader, &objects, &lights, 
                    &sun.dirLight, &fog, &frustum);
            }
            m.shaders.shadowShader.unbind();

            // NOTE(alex): Some reason this needs to be rebound to work? Who fucking knows.
            m.shaders.shadowShader.bind();
            {
                m.shaders.shadowShader.setMat4("projection", projection);
                m.shaders.shadowShader.setMat4("view", view);
                m.shaders.shadowShader.setVec3("viewPos", camera.Position);
                m.shaders.shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
                m.shaders.shadowShader.setBool("isT", false);

                // Render Terrain
                if (drawTerrain && strcmp(lvl.nextLevel.c_str(), "../levels/forest.txt") != 0)
                {
                    m.shaders.shadowShader.setBool("isT", true);
                    terrain.Draw(m.shaders.shadowShader, &lights, &sun.dirLight, &fog);
                }
            }
            m.shaders.shadowShader.unbind();

            // render Depth map to quad for visual debugging
            // ---------------------------------------------
            m.shaders.debugShader.bind();
            m.shaders.debugShader.setFloat("near_plane", near_plane);
            m.shaders.debugShader.setFloat("far_plane", far_plane);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            {
                renderQuad();
            }

            // Render Skybox
            if (drawSkybox)
            {
                skybox.Draw(m.shaders.skyboxShader);
            }

            if (strcmp(lvl.nextLevel.c_str(), "../levels/credit.txt") == 0)
            {
                sun.position.y = -camera.Position.z + 10;
                float rate = pow((-(camera.Position.z - 110.0f) / 200.0f) /2.0, 2);
                sun.dirLight.ambient = vec3(rate * 3.0f,rate * 2.0f,rate);
            }
            // Render Sun
            sun.Draw(m.shaders.sunShader);

            // Render Point Lights
            if (drawPointLights)
            {
                m.shaders.lightShader.bind();
                {
                    m.shaders.lightShader.setMat4("projection", projection);
                    m.shaders.lightShader.setMat4("view", view);
                    vec3 selectedColor = vec3(1.0f, 0.0f, 0.0f);

                    for (int i = 0; i < lights.size(); ++i)
                    {
                        if(selectedLight == i)
                        {
                            m.shaders.lightShader.setVec3("lightColor", selectedColor);
                        }
                        else
                        {
                            m.shaders.lightShader.setVec3("lightColor", lights[i].color);
                        }
                        model = mat4(1.0f);
                        float z_adj = lights[i].position.z;
                        if (lights[i].position.x < 0) z_adj -= 0.05f;
                        else z_adj += 0.1f;
                        model = translate(model, vec3(lights[i].position.x, lights[i].position.y, z_adj));
                        model = scale(model, vec3(0.15f, 0.075f, 0.075f));
                        m.shaders.lightShader.setMat4("model", model);
                        m.models.cube.Draw(m.shaders.lightShader);
                    }
                }
            }
            m.shaders.lightShader.unbind();

            if (strcmp(lvl.nextLevel.c_str(), "../levels/desert.txt") == 0) {
                water.Draw(m.shaders.waterShader, deltaTime);
            }

            if (strcmp(lvl.nextLevel.c_str(), "../levels/credit.txt") == 0) {
                water.height = -18.5f;
                water.color = vec4(0.15f, 0.15, 0.10f, 0.7f);
                water.Draw(m.shaders.waterShader, deltaTime);
            }

            if(drawParticles)
            {
                bound.height = -7.0f;
                if(strcmp(lvl.nextLevel.c_str(), "../levels/credit.txt") == 0) {
                    bound.height = -35.0f;
                    fog_offset = 40.0f;
                }
                if(strcmp(lvl.nextLevel.c_str(), "../levels/street.txt") == 0) {
                    bound.height = 25.0f;
                }
                for (int i = 0; i < emitters.size(); ++i)
                {
                    emitters[i].Draw(m.shaders.particleShader, deltaTime, bound.width, bound.height, fog_offset);
                }
            }

            // Render Note
            if(checkInteraction)
            {
                float minDistance = FLT_MAX;
                interactingObject = 0;
                for (int i = 0; i < objects.size(); ++i)
                {
                    // Ray collision detection.
                    vec3 p = camera.Position - objects[i].position; // The vector pointing from us to an object.
                    float rSquared = objects[i].selection_radius * objects[i].selection_radius;
                    float p_d = dot(p, selectorRay); // Calculated to see if the object is behind us.

                    if (p_d > 0 || dot(p, p) < rSquared) // If the object is behind us or surrounding the starting point:
                        continue;                        // No collision.

                    vec3 a = p - p_d * selectorRay; // Treat a as a plane passing through the object's center perpendicular to the ray.

                    float aSquared = dot(a, a);

                    if (aSquared > rSquared) // If our closest approach is outside the sphere:
                        continue;            // No collision.

                    if(length(p) < minDistance)
                    {
                        if(EditorMode == MOVEMENT)
                        {
			    if(length(p) < 10.0f)
			    {
				minDistance = length(p);
				interactingObject = i;
			    }
                        }
                        else
                        {
                            minDistance = length(p);
                            selectedObject = i;
                        }
                    }
                }
                if(objects[interactingObject].interactible)
                {
                    drawNote = true;
                    selectedNote = objects[interactingObject].noteNum;
                    discoveredNotes[selectedNote] = true;
                    if(objects[interactingObject].disappearing)
                    {
                        objects.erase(objects.begin() + interactingObject);
                        sounds[2]->startSound();
                    }
                    else
                    {
                        fspline.init(camera.Zoom, 20.0f, 0.5f);
                        fspline.active = true;
                        sounds[objects[interactingObject].sound]->startSound();
                    }
                }
                checkInteraction = false;
            }

            if(drawNote)
            {
                notes[selectedNote].Update(&fspline);
                notes[selectedNote].Draw(m.shaders.noteShader);
            }

            if(drawCollection)
            {
                int xInd = 0;
                int yInd = 0;
                for(int i = 0; i < notes.size(); ++i)
                {
                    xInd = i / 4;
                    yInd = i % 4;
                    if(discoveredNotes[i]) // Draw only discovered notes.
                    {
                        notes[i].DrawSmall(m.shaders.noteShader, xInd, yInd);
                    }
                }
            }


            // FRAMEBUFFER Render Normal scene plus bright portions
            // ----------------------------------------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            glEnable(GL_DEPTH_TEST);
          
            if(bound.active)
            {
                bound.Draw(m.shaders.transShader);
                if(bound.counter == 314)
                {
                    bound.active = false;
                }
                bound.counter++;
            }
            bound.DrawWall(m.shaders.boundaryShader, &m.models.cylinder);

            // FBO Time!
            // blur bright fragments with two-pass Gaussian Blur 
            bool horizontal = true, first_iteration = true;
            unsigned int amount = 10;
            m.shaders.blurShader.bind();
            {
                for (unsigned int i = 0; i < amount; i++)
                {
                    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
                    m.shaders.blurShader.setInt("horizontal", horizontal);

                    glActiveTexture(GL_TEXTURE0);
                    // bind texture of other framebuffer (or scene if first iteration)
                    glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1]
                                                                 : pingpongColorbuffers[!horizontal]);
                    renderQuad();
                    horizontal = !horizontal;
                    if (first_iteration)
                        first_iteration = false;
                }
            }
            m.shaders.blurShader.unbind();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);   

            // Render floating point color buffer to 2D quad and 
            // tonemap HDR colors to default framebuffer's (clamped) color range
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m.shaders.bloomShader.bind();
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
                m.shaders.bloomShader.setInt("bloom", bloom);
                m.shaders.bloomShader.setFloat("exposure", exposure);
                renderQuad();
            }
            m.shaders.bloomShader.unbind();
        } // End of Rendering Effects code.

        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glEnable(GL_DEPTH_TEST);

            // reset viewport for actual Drawing.
            glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

            mat4 projection = camera.GetProjectionMatrix();
            mat4 view = camera.GetViewMatrix();
            mat4 model;

            frustum.ExtractVFPlanes(projection, view);

            m.DrawAllModels(m.shaders.textureShader, &objects, &lights, &sun.dirLight,
                &fog, &frustum);

            if(drawTerrain)
            {
                terrain.Draw(m.shaders.terrainShader, &lights, &sun.dirLight, &fog);
            }

            // Render Skybox
            if (drawSkybox)
            {
                skybox.Draw(m.shaders.skyboxShader);
            }

            if (strcmp(lvl.nextLevel.c_str(), "../levels/credit.txt") == 0)
            {
                sun.position.y = -camera.Position.z + 10;
                float rate = pow((-(camera.Position.z - 110.0f) / 200.0f) /2.0, 2);
                sun.dirLight.ambient = vec3(rate * 3.0f,rate * 2.0f,rate);
            }
            // Render Sun
            sun.Draw(m.shaders.sunShader);

            // Render Point Lights
            m.shaders.lightShader.bind();
            {
                m.shaders.lightShader.setMat4("projection", projection);
                m.shaders.lightShader.setMat4("view", view);
                vec3 selectedColor = vec3(1.0f, 0.0f, 0.0f);

                if (drawPointLights)
                {
                    for (int i = 0; i < lights.size(); ++i)
                    {
                        if(selectedLight == i)
                        {
                            m.shaders.lightShader.setVec3("lightColor", selectedColor);
                        }
                        else
                        {
                            m.shaders.lightShader.setVec3("lightColor", lights[i].color);
                        }
                        model = mat4(1.0f);
                        float z_adj = lights[i].position.z;
                        if (lights[i].position.x < 0) z_adj -= 0.05f;
                        else z_adj += 0.1f;
                        model = translate(model, vec3(lights[i].position.x, lights[i].position.y, z_adj));
                        model = scale(model, vec3(0.15f, 0.075f, 0.075f));
                        m.shaders.lightShader.setMat4("model", model);
                        m.models.cube.Draw(m.shaders.lightShader);
                    }
                }

                // Render Selected Objects, Bounding Spheres, etc.
                m.shaders.lightShader.setVec3("lightColor", selectedColor);
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

                if (drawSelectionSpheres)
                {
                    for (int i = 0; i < objects.size(); ++i)
                    {
                        model = mat4(1.0f);
                        model = translate(model, objects[i].position);
                        model = scale(model, vec3(objects[i].selection_radius));
                        m.shaders.lightShader.setMat4("model", model);
                        m.models.sphere.Draw(m.shaders.lightShader);
                    }
                }

                if (EditorMode == GUI)
                {
                    objects[selectedObject].Draw(&m.shaders.lightShader, m.findbyId(objects[selectedObject].id).model, m.findbyId(objects[selectedObject].id).shader_type);
                }
            }
            m.shaders.lightShader.unbind();

            if (strcmp(lvl.nextLevel.c_str(), "../levels/desert.txt") == 0) {
                water.Draw(m.shaders.waterShader, deltaTime);
            }

            if (strcmp(lvl.nextLevel.c_str(), "../levels/credit.txt") == 0) {
                water.height = -18.5f;
                water.color = vec4(0.15f, 0.15, 0.10f, 0.7f);
                water.Draw(m.shaders.waterShader, deltaTime);
            }

            if(drawParticles)
            {
                bound.height = -7.0f;
                if(strcmp(lvl.nextLevel.c_str(), "../levels/credit.txt") == 0) {
                    bound.height = -35.0f;
                    fog_offset = 40.0f;
                }
                if(strcmp(lvl.nextLevel.c_str(), "../levels/street.txt") == 0) {
                    bound.height = 25.0f;
                }
                for (int i = 0; i < emitters.size(); ++i)
                {
                    emitters[i].Draw(m.shaders.particleShader, deltaTime, bound.width, bound.height, fog_offset);
                }
            }

            float minDistance = FLT_MAX;
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

                    if(length(p) < minDistance)
                    {
                        minDistance = length(p);
                        selectedObject = i;
                    }
                }
                checkInteraction = false;
            }

            if(drawNote)
            {
                notes[selectedNote].Update(&fspline);
                notes[selectedNote].Draw(m.shaders.noteShader);
            }

            if(drawCollection)
            {
                int xInd = 0;
                int yInd = 0;
                for(int i = 0; i < notes.size(); ++i)
                {
                    xInd = i / 4;
                    yInd = i % 4;
                    if(discoveredNotes[i]) // Draw only discovered notes.
                    {
                        notes[i].DrawSmall(m.shaders.noteShader, xInd, yInd);
                    }
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
            bound.DrawWall(m.shaders.boundaryShader, &m.models.cylinder);
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
                ImGui::Text("Particle = %d. Position = (%.02f %.02f %.02f)", 
                selectedParticle, 
                emitters[selectedParticle].startPosition.x, 
                emitters[selectedParticle].startPosition.y, 
                emitters[selectedParticle].startPosition.z);

                ImGui::SliderFloat3("Position", (float *)&emitters[selectedParticle].startPosition, -512.0f, 512.0f);
                ImGui::SliderFloat3("Velocity", (float *)&emitters[selectedParticle].startVelocity, -50.0f, 50.0f);
                ImGui::SliderFloat("Gravity", (float *)&emitters[selectedParticle].gravity, -100.0f, 100.0f);
                ImGui::SliderFloat("Bottom Radius", (float *)&emitters[selectedParticle].radius, 0.0f, 10.0f);
                ImGui::SliderFloat("Top Radius", (float *)&emitters[selectedParticle].radiusTop, 0.0f, 10.0f);
                ImGui::SliderFloat("Height Between Rads", (float *)&emitters[selectedParticle].height, -10.0f, 10.0f);
                ImGui::SliderFloat("Life Span", (float *)&emitters[selectedParticle].lifeSpan, 0.0f, 10.0f);
                ImGui::ColorEdit4("Start Color", (float *)&emitters[selectedParticle].startColor);
                ImGui::ColorEdit4("End Color", (float *)&emitters[selectedParticle].endColor);
                ImGui::SliderFloat("Start Scale", (float *)&emitters[selectedParticle].startScale, 0.0f, 20.0f);
                ImGui::SliderFloat("End Scale", (float *)&emitters[selectedParticle].endScale, 0.0f, 20.0f);
                ImGui::SliderInt("Bug Mode", (int *)&emitters[selectedParticle].bugMode, 0, 1);
                ImGui::SliderInt("Fog Mode", (int *)&emitters[selectedParticle].fogMode, 0, 1);
                
                if (ImGui::Button("Create Emitter"))
                {
                    emitters.push_back(
                        Emitter("../resources/models/particle/part.png", 10000, vec3(0, 10, 0), 0.2, 3, 7.0f, vec3(3, 10, 3), 2.0f, -9.81f, vec4(1.0f, 0.0f, 0, 1), vec4(0.0f, 0.0f, 1.0f, 1.0f), 1, 0));
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

                ImGui::SliderFloat3("Position", (float *)&lights[selectedLight].position, -512.0f, 512.0f);

                ImGui::ColorEdit3("Ambient", (float *)&lights[selectedLight].ambient);
                ImGui::ColorEdit3("Diffuse", (float *)&lights[selectedLight].diffuse);
                ImGui::ColorEdit3("Specular", (float *)&lights[selectedLight].specular);

                ImGui::SliderFloat3("Color", (float *)&lights[selectedLight].color, 0.0f, 20.0f);

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

            if (showFogEditor)
            {
                ImGui::Begin("Fog Editor");

                ImGui::SliderFloat("Min", (float *)&fog.minDistance, 0.0f, 1000.0f);
                ImGui::SliderFloat("Max", (float *)&fog.maxDistance, 0.0f, 1000.0f);
                ImGui::ColorEdit3("Color", (float *)&fog.color);
                ImGui::End();
            }

            if (showShadowEditor)
            {
                ImGui::Begin("Shadow Editor");

                ImGui::SliderFloat("near", (float *)&near_plane, -10.0f, 10.0f);
                ImGui::SliderFloat("far", (float *)&far_plane, 0.0f, 1000.0f);
                ImGui::SliderFloat("frustum", (float *)&shadow_frustum, 0.0f, 300.0f);

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
                ImGui::SliderFloat("Y Scale", &terrain.yScale, 0.0f, 200.0f);
                ImGui::SliderFloat("Y Shift", &terrain.yShift, 0.0f, 200.0f);

                if (ImGui::Button("Update"))
                {
                    terrain.init("../resources/heightmaps/" + string(terrainPath), terrain.yScale, terrain.yShift,
                            terrain.bottom, terrain.top, terrain.dirt);
                }

                ImGui::ColorEdit3("Bottom", (float *)&terrain.bottom);
                ImGui::ColorEdit3("Top", (float *)&terrain.top);
                ImGui::ColorEdit3("Dirt", (float *)&terrain.dirt);
                ImGui::SliderFloat("Water Level", (float *)&water.height, -40.0f, 5.0f);
                ImGui::End();
            }

            if (showBoundaryEditor)
            {
                ImGui::Begin("Boundary Editor");
                ImGui::ColorEdit3("Color", (float *)&bound.color);
                ImGui::SliderFloat("Y", (float *)&bound.boundY, -50.0f, 50.0f);
                ImGui::SliderFloat("Width", (float *)&bound.width, 0.0f, 500.0f);
                ImGui::SliderFloat("Height", (float *)&bound.height, 0.0f, 50.0f);
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

                ImGui::SliderFloat3("Position", (float *)&sounds[selectedSound]->pos, -512.0f, 512.0f);
                ImGui::SliderFloat("Volume", (float *)&sounds[selectedSound]->volume, 0.0f, 1.0f);
                ImGui::SliderFloat("Rolloff", (float *)&sounds[selectedSound]->rolloff, 0.0f, 100.0f);
                ImGui::SliderFloat("Min", (float *)&sounds[selectedSound]->minDistance, 0.0f, 100.0f);
                ImGui::SliderFloat("Max", (float *)&sounds[selectedSound]->maxDistance, 0.0f, 100.0f);
                ImGui::Checkbox("Looping?", &sounds[selectedSound]->isLooping);
                ImGui::Checkbox("Music?", &sounds[selectedSound]->isMusic);
                ImGui::Checkbox("Has Played?", &sounds[selectedSound]->hasPlayed);

                ImGui::End();
            }

            if (showSunEditor)
            {
                ImGui::Begin("Sun Editor");

                ImGui::SliderFloat("Scale", (float *)&sun.scale_factor, 0.0f, 100.0f);
                ImGui::SliderFloat3("Position", (float *)&sun.position, -512.0f, 512.0f);
                ImGui::SliderFloat3("Color", (float *)&sun.color, 0.0f, 20.0f);

                ImGui::SliderFloat3("Direction", (float *)&sun.dirLight.direction, -1.0f, 1.0f);

                ImGui::ColorEdit3("Ambient", (float *)&sun.dirLight.ambient);
                ImGui::ColorEdit3("Diffuse", (float *)&sun.dirLight.diffuse);
                ImGui::ColorEdit3("Specular", (float *)&sun.dirLight.specular);

                ImGui::End();
            }

            if (showObjectEditor)
            {
                ImGui::Begin("Object Editor");
                ImGui::Text("Object = %d/%lu. ID = %d. Position = (%.02f %.02f %.02f)", selectedObject, objects.size(),
                            objects[selectedObject].id,
                            objects[selectedObject].position.x, objects[selectedObject].position.y, objects[selectedObject].position.z);

                ImGui::Checkbox("Terrain Snap", &snapToTerrain);

                if (ImGui::SliderFloat("Pos.x", (float *)&objects[selectedObject].position.x, -512.0f, 512.0f))
                {
                    if (snapToTerrain)
                    {
                        objects[selectedObject].position.y = terrain.heightAt(objects[selectedObject].position.x,
                                                                              objects[selectedObject].position.z) + objects[selectedObject].scaleFactor * m.findbyId(objects[selectedObject].id).y_offset;
                    }
                    objects[selectedObject].UpdateModel();
                }
                if (ImGui::SliderFloat("Pos.y", (float *)&objects[selectedObject].position.y, -512.0f, 512.0f))
                {
                    objects[selectedObject].UpdateModel();
                }
                if (ImGui::SliderFloat("Pos.z", (float *)&objects[selectedObject].position.z, -512.0f, 512.0f))
                {
                    if (snapToTerrain)
                    {
                        objects[selectedObject].position.y = terrain.heightAt(objects[selectedObject].position.x,
                                                                              objects[selectedObject].position.z) + objects[selectedObject].scaleFactor * m.findbyId(objects[selectedObject].id).y_offset;
                    }
                    objects[selectedObject].UpdateModel();
                }

                if (ImGui::SliderFloat("AngleX", (float *)&objects[selectedObject].angleX, -PI, PI))
                    objects[selectedObject].UpdateModel();
                if (ImGui::SliderFloat("AngleY", (float *)&objects[selectedObject].angleY, -PI, PI))
                    objects[selectedObject].UpdateModel();
                if (ImGui::SliderFloat("AngleZ", (float *)&objects[selectedObject].angleZ, -PI, PI))
                    objects[selectedObject].UpdateModel();

                if (ImGui::SliderFloat("Scale", (float *)&objects[selectedObject].scaleFactor, 0.0f, 10.0f))
                {
                    objects[selectedObject].UpdateModel();
                    objects[selectedObject].collision_radius = m.findbyId(objects[selectedObject].id).collision_radius * objects[selectedObject].scaleFactor;
                    objects[selectedObject].view_radius = default_view * objects[selectedObject].scaleFactor;
                    objects[selectedObject].position.y = terrain.heightAt(objects[selectedObject].position.x, objects[selectedObject].position.z) + objects[selectedObject].scaleFactor * m.findbyId(objects[selectedObject].id).y_offset;
                }

                ImGui::Checkbox("Interactible?", &objects[selectedObject].interactible);
                ImGui::SameLine();
                ImGui::Checkbox("Disappearing?", &objects[selectedObject].disappearing);
                ImGui::SliderInt("Sound", &objects[selectedObject].sound, 0, 20);

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
                                                  terrain.heightAt(camera.Position.x, camera.Position.z) + default_scale * m.findbyId(id).y_offset,
                                                  camera.Position.z),
                                             -1.6f, 0.0f, 0.0f,
                                             vec3(1), default_view * default_scale, cr * default_scale, 
                                             default_scale, false, false, 0, 1));
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
                                             objects[selectedObject].angleX,
                                             objects[selectedObject].angleY,
                                             objects[selectedObject].angleZ,
                                             vec3(1), objects[selectedObject].view_radius, objects[selectedObject].collision_radius, 
                                             objects[selectedObject].scaleFactor, false, false, 0, 1));
                    selectedObject = objects.size() - 1;
                }
                ImGui::End();
            }

            ImGui::Begin("Level Editor");

            ImGui::InputText("Name", levelName, IM_ARRAYSIZE(levelName));

            ImGui::SliderFloat3("Starting Position", (float *)&lvl.startPosition, -128.0f, 128.0f);

            if (ImGui::Button("Save"))
            {
                string str = "../levels/";
                str.append(levelName);
                lvl.SaveLevel(str, &objects, &lights, &sun,
                    &emitters, &fog, &skybox, &terrain, &bound);
                cout << "Level saved: " << str << "\n";
            }
            ImGui::SameLine();
            if (ImGui::Button("Load"))
            {
                string str = "../levels/";
                str.append(levelName);
                lvl.LoadLevel(str, &objects, &lights, &sun,
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
                lvl.LoadLevel(lvl.nextLevel, &objects, &lights, &sun,
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
            ImGui::Checkbox("Sun", &showSunEditor);

            ImGui::Checkbox("Skybox", &showSkyboxEditor);
            ImGui::SameLine();
            ImGui::Checkbox("Terrain", &showTerrainEditor);
            ImGui::SameLine();
            ImGui::Checkbox("Boundary", &showBoundaryEditor);
            ImGui::SameLine();

            ImGui::Checkbox("Fog", &showFogEditor);
            ImGui::SameLine();
            ImGui::Checkbox("Sound", &showSoundEditor);

            ImGui::Checkbox("Shadow", &showShadowEditor);
            ImGui::SameLine();


            ImGui::NewLine();

            // Settings
            ImGui::Text("Settings");
            ImGui::Checkbox("Draw Terrain", &drawTerrain);
            ImGui::SameLine();
            ImGui::Checkbox("Draw Skybox", &drawSkybox);
            ImGui::SameLine();
            ImGui::Checkbox("Draw Point Lights", &drawPointLights);

            ImGui::Checkbox("View", &drawBoundingSpheres);
            ImGui::SameLine();
            ImGui::Checkbox("Collision", &drawCollisionSpheres);
            ImGui::SameLine();
            ImGui::Checkbox("Selection", &drawSelectionSpheres);

            ImGui::Checkbox("Draw Particles", &drawParticles);

            ImGui::Text("%d ms (%d FPS) |", (int)(1000 * deltaTime), (int)(1.0f / deltaTime));
            ImGui::SameLine();
            ImGui::Text("Drawn Objects: %d", drawnObjects);

            ImGui::Text("Location: (%.02f %.02f %.02f)", camera.Position.x, camera.Position.y, camera.Position.z);
            ImGui::Text("Orientation: (%.02f %.02f %.02f)", camera.Front.x, camera.Front.y, camera.Front.z);
            ImGui::Text("Level: %s | Next: %s", lvl.currentLevel.c_str(), lvl.nextLevel.c_str());

            ImGui::SliderInt("Speed", &bobbingSpeed, 0, 100);
            ImGui::SliderFloat("Amount", &bobbingAmount, 0.0f, 0.2f);

            if(ImGui::Button("Sunset!"))
            {
                sunspline.init(sun.dirLight.direction, vec3(0.0f, 0.0f, -1.0f), 10.0f);
                sunspline.active = true;
                ambspline.init(sun.dirLight.ambient, vec3(0.01f, 0.0f, 0.0f), 10.0f);
                ambspline.active = true;
            }
            ImGui::SameLine();
            if(ImGui::Button("Sunrise!"))
            {
                sunspline.init(sun.dirLight.direction, vec3(-0.5f, -0.2f, -0.7f), 10.0f);
                sunspline.active = true;
                ambspline.init(sun.dirLight.ambient, vec3(0.4f, 0.2f, 0.2f), 10.0f);
                ambspline.active = true;
            }

            ImGui::SliderFloat("Exposure", &exposure, 0.0f, 50.0f);
            ImGui::SliderFloat("Bloom Threshold", &gBloomThreshold, 0.0f, 1.0f);
            ImGui::Checkbox("Don't Cull?", &gDONTCULL);
            ImGui::SameLine();
            ImGui::Checkbox("Effects", &toggleRenderEffects);
            ImGui::SameLine();
            ImGui::Checkbox("Bloom", &bloom);

            ImGui::End();

            ImGui::Render();
            glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

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

    if(!drawNote)
    {
        if(strcmp(lvl.nextLevel.c_str(), "../levels/credit.txt") == 0) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            camera.ProcessKeyboard(FORWARD, deltaTime);
            if (camera.Mode == WALK && (camera.Position.x < -road_width-0.5f || camera.Position.x > road_width-0.5f))
            {
                camera.ProcessKeyboard(BACKWARD, deltaTime);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (camera.Mode == WALK && (camera.Position.x < -road_width-0.5f || camera.Position.x > road_width-0.5f))
            {
                camera.ProcessKeyboard(FORWARD, deltaTime);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            camera.ProcessKeyboard(LEFT, deltaTime);
            if (camera.Mode == WALK && (camera.Position.x < -road_width-0.5f || camera.Position.x > road_width-0.5f))
            {
                camera.ProcessKeyboard(RIGHT, deltaTime);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera.ProcessKeyboard(RIGHT, deltaTime);
            if (camera.Mode == WALK && (camera.Position.x < -road_width-0.5f || camera.Position.x > road_width-0.5f))
            {
                camera.ProcessKeyboard(LEFT, deltaTime);
            }
        }

        }
        else{
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
                camera.ProcessKeyboard(FORWARD, deltaTime);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            camera.ProcessKeyboard(LEFT, deltaTime);
            if (camera.Mode == WALK && Colliding(objects))
            {
                camera.ProcessKeyboard(RIGHT, deltaTime);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera.ProcessKeyboard(RIGHT, deltaTime);
            if (camera.Mode == WALK && Colliding(objects))
            {
                camera.ProcessKeyboard(LEFT, deltaTime);
            }
        }

        }
        
        if (camera.Mode == WALK && strcmp(lvl.nextLevel.c_str(), "../levels/forest.txt") != 0 && (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || 
                                    glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || 
                                    glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || 
                                    glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS))
        {
            if(!ma_sound_is_playing(&sounds->at(0)->sound))
                ma_sound_start(&sounds->at(0)->sound);
            bobbingCounter++;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && 
            glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && 
            glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE && 
            glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
        {
            ma_sound_stop(&sounds->at(0)->sound);
        }

        if (camera.Mode == WALK && strcmp(lvl.nextLevel.c_str(), "../levels/forest.txt") == 0) {
            camera.Position = vec3(0.0f, 0.0f, 0.0f);
            ma_sound_start(&sounds->at(8)->sound);
        }
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

    if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        drawCollection = true;
    }

    if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
    {
        drawCollection = false;
    }

    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        gSelecting = true;
    }

    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
    {
        gSelecting = false;
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
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && (gSelecting || EditorMode == MOVEMENT))
    {
        double xpos;
        double ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if(EditorMode == MOVEMENT)
        {
            xpos = RETINA_SCREEN_WIDTH / 2.0f;
            ypos = RETINA_SCREEN_HEIGHT / 2.0f;
        }

        GLbyte color[4];
        GLfloat depth;
        GLuint index;

        glReadPixels(xpos, RETINA_SCREEN_HEIGHT - ypos - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
        glReadPixels(xpos, RETINA_SCREEN_HEIGHT - ypos - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
        glReadPixels(xpos, RETINA_SCREEN_HEIGHT - ypos - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

        vec4 viewport = vec4(0, 0, RETINA_SCREEN_WIDTH, RETINA_SCREEN_HEIGHT);
        vec3 wincoord = vec3(xpos, RETINA_SCREEN_HEIGHT - ypos - 1, depth);
        vec3 objcoord = unProject(wincoord, camera.GetViewMatrix(), camera.GetProjectionMatrix(), viewport);

        selectorRay = normalize(objcoord - camera.Position);    
        if(!drawNote)
        {
            checkInteraction = true;
        }
        else
        {
            pauseNote = false;
            fspline.init(camera.Zoom, 45.0f, 0.5f);
            fspline.active = true;
        }
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if(drawCollection)
    {
        collectionScroll -= (float)yoffset / 3.0f;
        if(collectionScroll < 0.0f)
            collectionScroll = 0.0f;
        if(collectionScroll > 4.0f)
            collectionScroll = 4.0f;
    }
    else
    {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
