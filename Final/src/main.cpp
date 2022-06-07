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

using namespace std;
using namespace glm;

#define PI 3.14159265

// NOTE(Alex): Global State!
const unsigned int SCREEN_WIDTH = 1440;
const unsigned int SCREEN_HEIGHT = 900;
const unsigned int RETINA_SCREEN_WIDTH = SCREEN_WIDTH / 2;
const unsigned int RETINA_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;
const unsigned int TEXT_SIZE = 16;
const float PLAYER_HEIGHT = 1.4f;
const float default_scale = 1.0f;
const float default_view = 1.414f;
const float default_selection = 0.5f;

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

// NOTE(Lucas) For collision detection
vector<int> ignore_objects = {18, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};

// For FBO Stuff
bool bloom = true;
float exposure = 1.0f;
float gBloomThreshold = 0.5f;

bool gDONTCULL = false;

bool  gSelecting = false;
bool  deleteObject = false;
bool  deleteCheck = false;

float shadowAmount = 1.0f;

bool forestBeginning = false;
bool forestContinuing = false;
bool sunsetting = false;
bool sunrising = false;
bool exposingOut = false;

bool intro = true;
bool loadForest = false;

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
#include "cursor.h"

using namespace std;
using namespace glm;

void processInput(GLFWwindow *window, vector<Object> *objects, vector<Sound *> *sounds);

FloatSpline fspline;
FloatSpline exposurespline;
FloatSpline volumespline;
FloatSpline skyboxspline;
FloatSpline shadowspline;
FloatSpline rotspline;

Level lvl;

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

bool completedGame(vector<bool> *checks);

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

    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Experience", NULL, NULL);
    //GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Experience", glfwGetPrimaryMonitor(), NULL);
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
    int discoveredCount = 0;

    // Notes
    notes.push_back(Note("../resources/notes/note1.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/note2.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/note3.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/note4.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/note5.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/note6.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/note7.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/note8.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);

    notes.push_back(Note("../resources/notes/box1.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/box2.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/box9.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/box4.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/box5.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/box6.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/box7.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/box10.png"));
    discoveredNotes.push_back(false);
    //discoveredNotes.push_back(true);
    notes.push_back(Note("../resources/notes/box11.png"));
    notes.push_back(Note("../resources/notes/box12.png"));
    notes.push_back(Note("../resources/notes/note10.png"));
    notes.push_back(Note("../resources/notes/box13.png"));


    Note credit1 = Note("../resources/notes/credit1.png");
    Note credit2 = Note("../resources/notes/credit2.png");
    Note credit3 = Note("../resources/notes/credit3.png");

    Note opening1 = Note("../resources/notes/opening1.png");
    Note opening2 = Note("../resources/notes/opening2.png");
    Note opening3 = Note("../resources/notes/opening3.png");

    // Sounds
    Sound whistle = Sound("../resources/audio/whistle.wav", 1.0f, false);
    Sound pickup = Sound("../resources/audio/pickup2.mp3", 1.0f, false);
    Sound hmm = Sound("../resources/audio/hmm.wav", 1.0f, false);
    Sound rock = Sound("../resources/audio/desert.wav", vec3(25, 0, 0), 1.0f, 5.0f, 2.0f, 50.0f, true, false);
    Sound music = Sound("../resources/audio/BGM/愛にできることはまだあるかい.mp3", 0.1f, true);
    Sound streetMusic = Sound("../resources/audio/BGM/Sunrise.mp3", 0.3f, true);
    Sound sadMusic = Sound("../resources/audio/BGM/street.mp3", 0.3f, true);
    Sound forestMusic1 = Sound("../resources/audio/BGM/Forest_1_calm.mp3", 0.1f, true);
    Sound forestMusic2 = Sound("../resources/audio/BGM/Forest_1_dynamic.mp3", 0.1f, true);
    Sound desertMusic = Sound("../resources/audio/BGM/Desert.mp3", 0.3f, true);
    Sound alert = Sound("../resources/audio/alert.wav", 1.0f, false);
    Sound walk = Sound("../resources/audio/step.wav", 0.3f, false);
    Sound EVA = Sound("../resources/audio/congrats.wav", 1.0f, false);
    EVA.isLooping = false;
    Sound fire = Sound("../resources/audio/fire.mp3", vec3(-13.6, -3.799, 10.2), 1.0f, 7.0f, 1.0f, 10.0f, true, false);
    Sound whisper = Sound("../resources/audio/whisper.wav", 1.0f, false);
    Sound waterWalk = Sound("../resources/audio/waterWalk.wav", 0.3f, false);

    //Ambient Sounds
    Sound fogAmb = Sound("../resources/audio/wind.wav", 0.3f, true);
    Sound forestAmb = Sound("../resources/audio/bird.wav", vec3(0, 0, 0), 1.0f, 1.0f, 2.0f, 50.0f, true, false);
    Sound desertAmb = Sound("../resources/audio/fog.wav", vec3(0, 0, 0), 1.0f, 1.0f, 2.0f, 50.0f, true, false);

    //Talking Sounds
    Sound deep1 = Sound("../resources/audio/Talking/deep1.wav", 1.0f, false);
    Sound deep2 = Sound("../resources/audio/Talking/deep2.wav", 1.0f, false);
    Sound deep3 = Sound("../resources/audio/Talking/deep3.wav", 1.0f, false);
    Sound mid1 = Sound("../resources/audio/Talking/mid1.wav", 1.0f, false);
    Sound mid2 = Sound("../resources/audio/Talking/mid2.wav", 1.0f, false);
    Sound mid3 = Sound("../resources/audio/Talking/mid3.wav", 1.0f, false);
    Sound high1 = Sound("../resources/audio/Talking/high1.wav", 1.0f, false);
    Sound high2 = Sound("../resources/audio/Talking/high2.wav", 1.0f, false);
    Sound high3 = Sound("../resources/audio/Talking/high3.wav", 1.0f, false);
    Sound high4 = Sound("../resources/audio/Talking/high4.wav", 1.0f, false);
    Sound underwater1 = Sound("../resources/audio/Talking/underwater.wav", 1.0f, false);
    Sound underwater2 = Sound("../resources/audio/Talking/underwater2.wav", 1.0f, false);

    Sound laugh = Sound("../resources/audio/laugh.wav", 1.0f, false);

    sounds.push_back(&walk); // 0
    sounds.push_back(&whistle); // 1
    sounds.push_back(&pickup); // 2
    sounds.push_back(&hmm); // 3
    sounds.push_back(&rock); // 4
    sounds.push_back(&fogAmb); // 5
    sounds.push_back(&music); // 6
    sounds.push_back(&alert); // 7
    sounds.push_back(&EVA); // 8
    sounds.push_back(&forestAmb); // 9
    sounds.push_back(&fire); // 10
    sounds.push_back(&whisper); // 11
    sounds.push_back(&deep1); // 12
    sounds.push_back(&deep2); // 13
    sounds.push_back(&deep3); // 14
    sounds.push_back(&mid1); // 15
    sounds.push_back(&mid2); // 16
    sounds.push_back(&mid3); // 17
    sounds.push_back(&high1);// 18 
    sounds.push_back(&high2);// 19
    sounds.push_back(&high3); // 20
    sounds.push_back(&high4); // 21
    sounds.push_back(&underwater1); // 22
    sounds.push_back(&underwater2); // 23 
    sounds.push_back(&laugh); // 24


    Skybox skybox;
    stbi_set_flip_vertically_on_load(false);
    // Default value.
    skybox.init("../resources/skyboxes/sunsky/", false);
    float skyboxMaskAmount = 0.0f;

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

    lvl.LoadLevel("../levels/opening.txt", &objects, &lights,
                  &sun, &emitters, &fog, &skybox, &terrain, &bound);
    Frustum frustum;

    Water water;
    water.gpuSetup();

    Cursor cursor;
    cursor.init(vec3(1.0f, 1.0f, 1.0f));

    Spline sunspline;
    Spline particlespline;
    Spline suncolorspline;
    Spline ambspline;
    Spline diffspline;
    Spline terrainspline;

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

        fogAmb.updateSound();

        bool underwater = true;
        if(strcmp(lvl.currentLevel.c_str(), "../levels/forest.txt") == 0)
        {
            if(camera.Position.y < (water.height + PLAYER_HEIGHT))
            {
                sounds[0]->stopSound();
                sounds[0] = &waterWalk;
                camera.Slow = true;
            }
            else
            {
                sounds[0]->stopSound();
                camera.Slow = false;
                sounds[0] = &walk;
            }

            static vec3 oldAmb = sun.dirLight.ambient;
            static vec3 oldDif = sun.dirLight.diffuse;
            if(camera.Position.y < (water.height) && underwater)
            {
                sun.dirLight.ambient = vec3(0.1, 0.2, 0.9);
                sun.dirLight.diffuse = vec3(0.2, 0.1, 0.9);
                forestAmb.stopSound();
                fire.stopSound();
                underwater = true;
            }
            else if(underwater && camera.Position.y > (water.height))
            {
                sun.dirLight.ambient = oldAmb;
                sun.dirLight.diffuse = oldDif;
                //forestAmb.startSound();
                //fire.startSound();
                underwater = false;
            }
        }

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ++frameCount;

        drawnObjects = 0;

        if(loadForest)
        {
            loadForest = false;
            lvl.LoadLevel("../levels/forest.txt", &objects, &lights, &sun,
                          &emitters, &fog, &skybox, &terrain, 
                          &bound);

            exposure = 50.0f;
            exposurespline.init(exposure, 1.5f, 10.0f);
            exposurespline.active = true;
        }

        // Input Resolution
        glfwPollEvents();
        processInput(window, &objects, &sounds);
        if (camera.Mode == WALK)
        {
            float dist = sqrt((abs(camera.Position.x) * abs(camera.Position.x)) + (abs(camera.Position.z) * abs(camera.Position.z)));
            if(dist > bound.width - 2)
            {
                if(strcmp(lvl.nextLevel.c_str(), "../levels/street.txt") == 0)
                {
                    sunspline.deactivate();
                    suncolorspline.deactivate();
                    ambspline.deactivate();
                    diffspline.deactivate();

                    camera.Yaw = -90.0f;
                    camera.Pitch = 0.0f;
                    camera.updateCameraVectors();
                }

                cout << "Boundary Collision. Loading Next Level.\n";
                lvl.LoadLevel(lvl.nextLevel, &objects, &lights, &sun,
                              &emitters, &fog, &skybox, &terrain, 
                              &bound);

                // Transitions!
                if(strcmp(lvl.currentLevel.c_str(), "../levels/desert.txt") == 0)
                {
                    exposure = 50.0f;
                    exposurespline.init(exposure, 2.0f, 10.0f);
                    exposurespline.active = true;
                }
                else if(strcmp(lvl.currentLevel.c_str(), "../levels/street.txt") == 0)
                {
                    exposure = 0.0f;
                    exposurespline.init(exposure, 0.4f, 10.0f);
                    exposurespline.active = true;
                }
            }
            if(dist > bound.width - 50)
            {
                fogAmb.volume = (dist-(bound.width-50))/(bound.width-50);
            }
            else
            {
                fogAmb.volume = 0.0f;
            }

            if(dist > bound.width - 20.0f)
            {
                if(strcmp(lvl.currentLevel.c_str(), "../levels/forest.txt") == 0)
                {
                    exposure = 1 + ((dist - (bound.width - 20.0f)) / (bound.width- 20.0f)) * 10.0f;
                }
            }

            camera.Position.y = terrain.heightAt(camera.Position.x, camera.Position.z) + PLAYER_HEIGHT + bobbingAmount * sin((float)bobbingCounter / (float)bobbingSpeed);
        }

        if(deleteObject)
        { 
            objects.erase(objects.begin() + selectedObject);
            deleteObject = false;
        }

        // Spline
        fspline.update(deltaTime);
        exposurespline.update(deltaTime);
        volumespline.update(deltaTime);
        skyboxspline.update(deltaTime);
        shadowspline.update(deltaTime);
        rotspline.update(deltaTime);

        particlespline.update(deltaTime);
        sunspline.update(deltaTime);
        suncolorspline.update(deltaTime);
        ambspline.update(deltaTime);
        diffspline.update(deltaTime);
        terrainspline.update(deltaTime);

        if(fspline.active)
        {
            camera.Zoom = fspline.getPosition();
        }

        if(rotspline.active)
        {
            objects[interactingObject].angleY = rotspline.getPosition();
            objects[interactingObject].UpdateModel();
        }
        if(volumespline.active)
        {
            walk.volume = volumespline.getPosition();
                walk.updateSound();
        }
        if(exposurespline.active)
        {
            exposure = exposurespline.getPosition();
        }
        if(skyboxspline.active)
        {
            skyboxMaskAmount = skyboxspline.getPosition();
        }
        if(shadowspline.active)
        {
            shadowAmount = shadowspline.getPosition();
        }

        if(strcmp(lvl.currentLevel.c_str(), "../levels/street.txt") == 0)
        {
            skyboxMaskAmount = 0.0f;
        }

        if(sunspline.active)
        {
            sun.position = sunspline.getPosition();
        } 
        if(suncolorspline.active)
        {
            sun.color = suncolorspline.getPosition();
        }
        if(ambspline.active)
        {
            sun.dirLight.ambient = ambspline.getPosition();
        }
        if(diffspline.active)
        {
            sun.dirLight.diffuse = diffspline.getPosition();
        }
        if(particlespline.active)
        {
            if(emitters.size() > 0)
            {
                emitters[0].startColor = vec4(particlespline.getPosition(), 1.0f);
                emitters[0].endColor = vec4(particlespline.getPosition(), 1.0f);
            }
        }
        if(terrainspline.active)
        {
	    terrain.top = terrainspline.getPosition();
        }

        sun.updateLight();

        // SHADOW STUFF
        if(strcmp(lvl.currentLevel.c_str(), "../levels/street.txt") == 0)
        {
            far_plane = 800.0f;
            gDONTCULL = true;

            if(!sunrising && camera.Position.z < 60.0f)
            {
                float sunriseTimer = 30.0f;

                sunspline.init(sun.position, vec3(sun.position.x, 200.0f, sun.position.z), sunriseTimer);
                sunspline.active = true;
                suncolorspline.init(sun.color, vec3(20.0f, 20.0f, 0.1f), sunriseTimer);
                suncolorspline.active = true;
                ambspline.init(sun.dirLight.ambient, vec3(0.3f, 0.3f, 0.01f), sunriseTimer);
                ambspline.active = true;
                diffspline.init(sun.dirLight.diffuse, vec3(0.5f, 0.5f, 0.01f), sunriseTimer);
                diffspline.active = true;
                shadowspline.init(shadowAmount, 1.0f, sunriseTimer);
                shadowspline.active = true;
                
                streetMusic.startSound();

                volumespline.init(walk.volume, 0.0f, 10.0f);
                volumespline.active = true;

                desertAmb.volume = 0.1f;
                sunrising = true;
            }

            if(!exposingOut && camera.Position.z < -70.0f)
            {
                exposurespline.init(exposure, -0.5f, 5.0f);
                exposurespline.active = true;
                exposingOut = true;
            }

            if(exposure < -0.1)
            {
                lvl.LoadLevel("../levels/credit.txt", &objects, &lights,
                              &sun, &emitters, &fog, &skybox, &terrain, &bound);
                exposure = 1.0f;
            }
        }

        static float counter1 = 0;
        static float counter2 = 0;
        static float counter3 = 0;

        if (strcmp(lvl.currentLevel.c_str(), "../levels/opening.txt") == 0)
        {
            camera.Position = vec3(0.0f, 0.0f, 0.0f);

            counter1 += 50.0f * deltaTime;

            exposure = 2.0f;

            if(counter1 >= 157)
            {
                counter1 = 157;
                counter2 += 50.0f * deltaTime;
                if(counter2 > 157) 
                {
                    counter2 = 157;
                    counter3 += 50.0f * deltaTime;
                    if(counter3 > 157)
                    {
                        counter3 = 157;
                    }
                }
            }

            fogAmb.startSound();
        }

        if (strcmp(lvl.currentLevel.c_str(), "../levels/credit.txt") == 0)
        {
            camera.Position = vec3(0.0f, 0.0f, 0.0f);

            counter1 += 50.0f * deltaTime;
            streetMusic.stopSound();
            exposure = 2.0f;

            if(counter1 >= 157)
            {
                counter1 = 157;
                counter2 += 50.0f * deltaTime;
                if(counter2 > 157) 
                {
                    counter2 = 157;
                    counter3 += 50.0f * deltaTime;
                    if(counter3 > 157)
                    {
                        counter3 = 157;
                    }
                }
            }

            if(completedGame(&discoveredNotes))
            {
                EVA.startSound();
            }
            else
            {
                sadMusic.startSound();
            }
        }

        if(strcmp(lvl.currentLevel.c_str(), "../levels/desert.txt") == 0)
        {
            forestMusic1.stopSound();
            forestMusic2.stopSound();

	    far_plane = 800.0f;
	    shadow_frustum = 226.0f;

            if(!sunsetting && discoveredNotes[0])
            {
                float sunsetTimer = 30.0f;

                sunspline.init(sun.position, vec3(sun.position.x, -80.0f, sun.position.z), sunsetTimer * 3);
                sunspline.active = true;
                suncolorspline.init(sun.color, vec3(20.0f, 1.0f, 0.1f), sunsetTimer);
                suncolorspline.active = true;
                ambspline.init(sun.dirLight.ambient, vec3(0.1f, 0.1f, 0.1f), sunsetTimer);
                ambspline.active = true;
                diffspline.init(sun.dirLight.diffuse, vec3(0.0f, 0.0f, 0.0f), sunsetTimer * 3);
                diffspline.active = true;
                exposurespline.init(exposure, 0.2f, sunsetTimer);
                exposurespline.active = true;
                skyboxspline.init(skyboxMaskAmount, 1.0f, sunsetTimer);
                skyboxspline.active = true;
                shadowspline.init(shadowAmount, 0.0f, sunsetTimer);
                shadowspline.active = true;
                particlespline.init(vec3(emitters[0].endColor.x, emitters[0].endColor.y, emitters[0].endColor.z), vec3(0.0f), sunsetTimer);
                particlespline.active = true;
                terrainspline.init(terrain.top, vec3(1.0f), sunsetTimer);
                terrainspline.active = true;

                desertMusic.startSound();

                emitters.push_back(Emitter("../resources/models/particle/part.png", 10000, vec3(0, 50, 0), 500.0f, 3.0f, 7.0f, vec3(0, 0, 0), 8.424f, 0.0f, vec4(1, 1, 1, 1), vec4(1, 1, 1, 1), 0.424f, 0.0f));

                sunsetting = true;
            }
        }

        if(strcmp(lvl.currentLevel.c_str(), "../levels/forest.txt") == 0)
        {
            counter1 = 0;
            counter2 = 0;
            counter3 = 0;
            if(!forestBeginning && discoveredCount > 0)
            {
                forestMusic1.startSound();
                forestBeginning = true;
            }
            if(!forestContinuing && discoveredCount > 4)
            {
                forestMusic1.stopSound();
                forestMusic2.startSound();
                forestContinuing = true;
            }
        }

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
                m.shaders.shadowShader.setFloat("shadowAmount", shadowAmount);
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
                m.shaders.shadowShader.setFloat("shadowAmount", shadowAmount);

                // Render Terrain
                if (drawTerrain && 
                    (strcmp(lvl.currentLevel.c_str(), "../levels/credit.txt") != 0))
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
                skybox.Draw(m.shaders.skyboxShader, skyboxMaskAmount);
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

            if (strcmp(lvl.currentLevel.c_str(), "../levels/forest.txt") == 0) {
                water.Draw(m.shaders.waterShader, deltaTime);
                if(underwater && camera.Position.y > (water.height)) {
                    forestAmb.updateSound();
                    fire.updateSound();
                }
            }

            if(strcmp(lvl.currentLevel.c_str(), "../levels/desert.txt") == 0) {
                bound.height = 25.0f;
                forestAmb.stopSound();
                fire.stopSound();
                desertAmb.updateSound();
            }

            if (strcmp(lvl.currentLevel.c_str(), "../levels/street.txt") == 0) {
                water.height = -18.5f;
                water.color = vec4(0.15f, 0.15, 0.10f, 0.7f);
                water.Draw(m.shaders.waterShader, deltaTime);
                desertAmb.stopSound();
                desertMusic.stopSound();
            }
            if(drawParticles)
            {
                bound.height = -7.0f;
                if(strcmp(lvl.currentLevel.c_str(), "../levels/street.txt") == 0) {
                    bound.height = -35.0f;
                    fog_offset = 40.0f;
                }
                if(strcmp(lvl.currentLevel.c_str(), "../levels/desert.txt") == 0) {
                    bound.height = 25.0f;
                }
                for (int i = 0; i < emitters.size(); ++i)
                {
                    emitters[i].Draw(m.shaders.particleShader, deltaTime, bound.width, bound.height, fog_offset);
                }
            }

            // Credits Rendering
            if(strcmp(lvl.currentLevel.c_str(), "../levels/credit.txt") == 0)
            {
                credit1.DrawCredit(m.shaders.noteShader, counter1, 1.0f, 0.5f);
                credit2.DrawCredit(m.shaders.noteShader, counter2, 0.5f, 0.4f);
                credit3.DrawCredit(m.shaders.noteShader, counter3, -0.2f, 1.0f);
            }

            if(strcmp(lvl.currentLevel.c_str(), "../levels/opening.txt") == 0)
            {
                opening1.DrawCredit(m.shaders.noteShader, counter1, 1.0f, 0.5f);
                opening2.DrawCredit(m.shaders.noteShader, counter2, 0.5f, 0.4f);
                opening3.DrawCredit(m.shaders.noteShader, counter3, -0.2f, 1.0f);
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
		    if(selectedNote < 16 && !discoveredNotes[selectedNote])
		    {
			discoveredCount++;
		    }
                    if(objects[interactingObject].disappearing)
                    {
                        objects.erase(objects.begin() + interactingObject);
                        sounds[2]->startSound();
                    }
                    else
                    {
                        fspline.init(camera.Zoom, 20.0f, 0.5f);
                        fspline.active = true;

                        // vec3 pointVec = camera.Position - objects[interactingObject].position;
                        // float rot = dot(vec2(pointVec.x, pointVec.z), vec2(0, 1));
                        // rotspline.init(0.0f, rot, 2.5f);
                        // rotspline.active = true;

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

            cursor.Draw(m.shaders.cursorShader);


            // FRAMEBUFFER Render Normal scene plus bright portions
            // ----------------------------------------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            glEnable(GL_DEPTH_TEST);
          
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
                skybox.Draw(m.shaders.skyboxShader, skyboxMaskAmount);
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
                    objects[selectedObject].Draw(&m.shaders.lightShader, 
                        m.findbyId(objects[selectedObject].id).model, 
                        m.findbyId(objects[selectedObject].id).shader_type);
                }
            }
            m.shaders.lightShader.unbind();

            if (strcmp(lvl.currentLevel.c_str(), "../levels/forest.txt") == 0) {
                water.Draw(m.shaders.waterShader, deltaTime);
            }

            if (strcmp(lvl.currentLevel.c_str(), "../levels/street.txt") == 0) {
                water.height = -18.5f;
                water.color = vec4(0.15f, 0.15, 0.10f, 0.7f);
                water.Draw(m.shaders.waterShader, deltaTime);
            }

            if(drawParticles)
            {
                bound.height = -7.0f;
                if(strcmp(lvl.currentLevel.c_str(), "../levels/street.txt") == 0) {
                    bound.height = -35.0f;
                    fog_offset = 40.0f;
                }
                if(strcmp(lvl.currentLevel.c_str(), "../levels/desert.txt") == 0) {
                    bound.height = 25.0f;
                }
                for (int i = 0; i < emitters.size(); ++i)
                {
                    emitters[i].Draw(m.shaders.particleShader, 
                                     deltaTime, 
                                     bound.width, 
                                     bound.height, 
                                     fog_offset);
                }
            }

            // Render Note
            float minDistance = FLT_MAX;
            if(checkInteraction)
            {
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

            bound.DrawWall(m.shaders.boundaryShader, &m.models.cylinder);
        }

        /* Present Render */
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

bool completedGame(vector<bool> *checks)
{
    bool check = true;
    for(int i = 0; i < checks->size(); ++i)
    {
        if(!checks->at(i))
        {
            check = false;
        }
    }
    return check;
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

    if(!drawNote)
    {
        if(strcmp(lvl.currentLevel.c_str(), "../levels/street.txt") == 0)
        {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            {
                camera.ProcessKeyboard(FORWARD, deltaTime);
                if (camera.Mode == WALK
                     && ((camera.Position.x < -road_width-0.5f || camera.Position.x > road_width-0.5f)
                     || (camera.Position.z > 110.0f)))
                {
                    camera.ProcessKeyboard(BACKWARD, deltaTime);
                }
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            {
                camera.ProcessKeyboard(BACKWARD, deltaTime);
                if (camera.Mode == WALK
                     && ((camera.Position.x < -road_width-0.5f || camera.Position.x > road_width-0.5f)
                     || (camera.Position.z > 110.0f)))
                {
                    camera.ProcessKeyboard(FORWARD, deltaTime);
                }
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            {
                camera.ProcessKeyboard(LEFT, deltaTime);
                if (camera.Mode == WALK
                     && ((camera.Position.x < -road_width-0.5f || camera.Position.x > road_width-0.5f)
                     || (camera.Position.z > 110.0f)))
                {
                    camera.ProcessKeyboard(RIGHT, deltaTime);
                }
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            {
                camera.ProcessKeyboard(RIGHT, deltaTime);
                if (camera.Mode == WALK
                     && ((camera.Position.x < -road_width-0.5f || camera.Position.x > road_width-0.5f)
                     || (camera.Position.z > 110.0f)))
                {
                    camera.ProcessKeyboard(LEFT, deltaTime);
                }
            }

        }
        else
        {
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
        
        if (camera.Mode == WALK && strcmp(lvl.currentLevel.c_str(), "../levels/credit.txt") != 0 
                                && (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || 
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
    }

    if(glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        drawCollection = true;
    }

    if(glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
    {
        drawCollection = false;
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && intro)
    {
        intro = false;
        loadForest = true;
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
