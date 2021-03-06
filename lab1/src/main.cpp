/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <random>
#include <time.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

using namespace std;
using namespace glm;
shared_ptr<Shape> shape;
shared_ptr<Shape> shape2;
ma_engine engine;
ma_result result = ma_engine_init(NULL, &engine);


float gen_rand(){
    float bound = ((float) rand()/RAND_MAX) - 0.5;
    return bound;
}

float lastX = 1920 / 2.0f;
float lastY = 1080 / 2.0f;
float dimension = 20.0f;
bool firstMouse = true;
int death = 0;
int live = 0;


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}

class entity
{
public:
    double posX, posZ, velX, velZ;
    float scale = 1;
    float bob = 0;
    bool state, dying = 0;
    glm::vec3 color = glm::vec3(0, 2.4, 0);
};
entity objects[50];

class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d, shift, control;
	camera()
	{
		w = a = s = d = shift = control = 0;
		rot = glm::vec3(0, 2.4, 0);
        pos = glm::vec3(10, 0, 10);
	}
	glm::mat4 process(double ftime)
	{
		float speedZ = 0;
        float speedX = 0;
		if (w == 1)
		{
			speedZ = 10*ftime;
		}
		else if (s == 1)
		{
			speedZ = -10*ftime;
		}
		if (a == 1)
			speedX = 10*ftime;
		else if(d==1)
			speedX = -10*ftime;
        if (shift == 1){
            speedX *= 2;
            speedZ *= 2;
        }
        if (control == 1){
            speedX /= 2;
            speedZ /= 2;
        }
        
		glm::mat4 Ry = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
        glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(-1, 0, 0));
		glm::vec4 dir = glm::vec4(speedX, 0, speedZ,1);
		dir = dir*Rx*Ry;
		pos += glm::vec3(dir.x, 0.0f, dir.z);
        if (pos.x > dimension/2 || pos.x < -dimension/2) {
            pos.x -= dir.x;
        }
        if (pos.z > dimension/2 || pos.z < -dimension/2) {
            pos.z -= dir.z;
        }
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return Rx*Ry*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog,psky,papp;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox;

	//texture data
	GLuint Texture, TextureN;
	GLuint Texture2;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
        if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
        {
            mycam.shift = 1;
        }
        if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
        {
            mycam.shift = 0;
        }
        if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
        {
            mycam.control = 1;
        }
        if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
        {
            mycam.control = 0;
        }
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void curCallback(GLFWwindow *window, double posX, double posY)
	{
        
        glfwGetCursorPos(window, &posX, &posY);
        
        float xpos = static_cast<float>(posX);
        float ypos = static_cast<float>(posY);
        
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        
        float xoff = xpos - lastX;
        float yoff = lastY - ypos;
        
        lastX = xpos;
        lastY = ypos;
        
        mycam.rot.y += xoff *0.005;
        mycam.rot.x += yoff *0.005;
        
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat rect_vertices[] = {
			// front
			-1.0, -1.0,  1.0,//LD
			1.0, -1.0,  1.0,//RD
			1.0,  1.0,  1.0,//RU
			-1.0,  1.0,  1.0,//LU
		};
		//make it a bit smaller
		for (int i = 0; i < 12; i++)
			rect_vertices[i] *= 0.5;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		GLfloat cube_norm[] = {
			// front colors
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,

		};
		glGenBuffers(1, &VertexNormDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		glm::vec2 cube_tex[] = {
			// front colors
			glm::vec2(0.0, 2.0),
			glm::vec2(2.0, 2.0),
			glm::vec2(2.0, 0.0),
			glm::vec2(0.0, 0.0),

		};
		glGenBuffers(1, &VertexTexBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {

			// front
			0, 1, 2,
			2, 3, 0,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);



		glBindVertexArray(0);

		string resourceDirectory = "../../resources" ;
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();
        
        shape2 = make_shared<Shape>();
        shape2->loadMesh(resourceDirectory + "/Banana.obj");
        shape2->resize();
        shape2->init();

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/grass.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture Night
		str = resourceDirectory + "/banana.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureN);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);





		//texture 2
		str = resourceDirectory + "/sky.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(psky->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(psky->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(psky->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);
        
        Tex1Location = glGetUniformLocation(papp->pid, "tex");//tex, tex2... sampler in the fragment shader
        Tex2Location = glGetUniformLocation(papp->pid, "tex2");
        // Then bind the uniform samplers to texture units:
        glUseProgram(papp->pid);
        glUniform1i(Tex1Location, 0);
        glUniform1i(Tex2Location, 1);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");


		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("dn");
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");
        
        papp = std::make_shared<Program>();
        papp->setVerbose(true);
        papp->setShaderNames(resourceDirectory + "/applevertex.glsl", resourceDirectory + "/applefrag.glsl");
        if (!papp->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        papp->addUniform("P");
        papp->addUniform("V");
        papp->addUniform("M");
        papp->addUniform("campos");
        papp->addUniform("objColor");
        papp->addAttribute("vertPos");
        papp->addAttribute("vertNor");
        papp->addAttribute("vertTex");
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
    
    double velX = gen_rand(); // set velocity of banana (should be random)
    double velZ = gen_rand();
    float scale = 1;
	void render()
	{
        static int renderNum;
        renderNum += 1;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
//		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		
		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * RotateXSky * SSky;

		psky->bind();		
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureN);
		static float ttime = 0;
		ttime += frametime;
		float dn = sin(ttime)*0.5 +0.5;
		glUniform1f(psky->getUniform("dn"), dn);		
		glDisable(GL_DEPTH_TEST);
		shape->draw(psky, false);
		glEnable(GL_DEPTH_TEST);	
		psky->unbind();

		
		glm::mat4 RotateX;
		glm::mat4 TransZ;
		glm::mat4 S;

	
		// Draw the box using GLSL.
		prog->bind();

		
		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);	
	
		glBindVertexArray(VertexArrayID);
		//actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		
        
		TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.8f, 0.0f));
		S = glm::scale(glm::mat4(1.0f), glm::vec3(dimension, dimension, 0.f));
		float angle = 3.1415926 / 2.0f;
		RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));

		M = TransZ *  RotateX * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);		
		glBindVertexArray(0);
        
        prog->unbind();
        
        papp->bind();

        //send the matrices to the shaders
        glUniformMatrix4fv(papp->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(papp->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(papp->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(papp->getUniform("campos"), 1, &mycam.pos[0]);
        
        static float inc = 0;
        inc += frametime;
        
        if(renderNum % 100 == 1)
        {
            for (int i=0; i<50;i++)
            {
                if(objects[i].state == 0)
                {
                    objects[i].state = 1;
                    objects[i].scale = 1;
                    objects[i].bob = 0;
                    objects[i].velX = gen_rand();
                    objects[i].velZ = gen_rand();
                    objects[i].posX = gen_rand() * 15;
                    objects[i].posZ = gen_rand() * 15;
                    objects[i].color = glm::vec3(0, 0, 0);
                    break;
                }
            }
        }
        
        for (int i=0;i<50;i++)
        {
           if(objects[i].state == 1)
           {
               live++;
               objects[i].posX += objects[i].velX * frametime * 5;
               objects[i].posZ += objects[i].velZ * frametime * 5;
               
               float dot = objects[i].velX*0 + objects[i].velZ*-1; //Mesh Orientation
               float det = objects[i].velX*-1 - objects[i].velZ*0;
               angle = atan2(det, dot) + 3.141592653589;
               
               objects[i].bob = sin(inc*7*(abs(objects[i].velX) + abs(objects[i].velZ)))/3;
               
               if (objects[i].posX > dimension/2 || objects[i].posX < -dimension/2) {
                   objects[i].velX = -objects[i].velX;
               }
               if (objects[i].posZ > dimension/2 || objects[i].posZ < -dimension/2) {
                   objects[i].velZ = -objects[i].velZ;
               }
               
               if(distance(glm::vec3(objects[i].posX, 0, objects[i].posZ), -mycam.pos) < 1 &&  objects[i].dying == 0)
               {
                   objects[i].velX = 0;
                   objects[i].velZ = 0;
                   objects[i].dying = 1;
                   ma_engine_play_sound(&engine, "../../resources/eat.wav", NULL);
               }
               
               for(int j = 0;j<50;j++)
               {
                   if(objects[j].state == 1 && j != i)
                   {
                       if(distance(glm::vec3(objects[i].posX, 0, objects[i].posZ), glm::vec3(objects[j].posX, 0, objects[j].posZ)) < 1 && objects[i].dying == 0 && objects[j].dying == 0)
                       {
                           objects[i].velX = 0;
                           objects[i].velZ = 0;
                           objects[j].velX = 0;
                           objects[j].velZ = 0;
                           objects[i].dying = 1;
                           objects[j].dying = 1;
                           ma_engine_play_sound(&engine, "../../resources/splat.wav", NULL);
                       }
                   }
               }
               if(objects[i].dying == 1 && (renderNum % 200) == 1)
               {
                   objects[i].state = 0;
                   objects[i].dying = 0;
                   death++;
                   cout << "Living banana: " << live << " | Collided banana: " << death << endl;
               }
               if(objects[i].dying == 1)
               {
                   objects[i].bob = 0;
                   objects[i].color.x = sin(inc*5);
                   objects[i].color.y = cos(inc*6);
                   objects[i].color.z = sin(inc*4);
                   angle += (renderNum % 150)/5;
                   objects[i].scale -= 0.44 * frametime;
               }
               
               
               glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(objects[i].posX, 0.0f, objects[i].posZ));
               glm::mat4 R = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
               glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), objects[i].bob, glm::vec3(1.0f, 0.0f, 0.0f));
               glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(objects[i].scale, objects[i].scale, objects[i].scale));
               M = T  * R * Rx * S;
               glUniformMatrix4fv(papp->getUniform("M"), 1, GL_FALSE, &M[0][0]);
               glUniform3fv(papp->getUniform("objColor"), 1, &objects[i].color[0]);
               shape2->draw(papp, false); //Draw Banana
           }
               
        }
        
        live = 0;
        
        
        
        
        
        papp->unbind();
		
		

	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
    srand (static_cast <unsigned> (time(0)));
	std::string resourceDir = "../../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

    Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
