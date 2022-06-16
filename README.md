<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#introduction">Introduction</a></li>
        <li><a href="#gameplay">Gameplay</a></li>
        <li>
          <a href="#environment">Environment</a>
          <ul>
            <li><a href="#forest---a-new-beginning">Forest - A new beginning</a></li>
            <li><a href="#desert---arduous-journey">Desert - Arduous Journey</a></li>
            <li><a href="#street---completion-and-loneliness">Street - Completion and Loneliness</a></li>
            <li><a href="#credit---with-secret-ending">Credit - With Secret Ending</a></li>
          </ul>
        </li>
        <li><a href="#technical-details">Technical Details</a>
        <ul>
            <li><a href="#instanced-rendering">Instanced Rendering</a></li>
            <li><a href="#shadow-mapping">Shadow Mapping</a></li>
            <li><a href="#high-dynamic-range">High Dynamic Range</a></li>
            <li><a href="#bloom-and-blur">Bloom and Blur</a></li>
            <li><a href="#view-frustum-culling">View Frustum Culling</a></li>
            <li><a href="#distance-fog">Distance Fog</a></li>
            <li><a href="#level-editor">Level Editor</a></li>
          </ul>
        </li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li><a href="#references">References</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>

# Experience
### Final
* A game in 10 weeks.
### Sample Workshops
* View Frustum Culling
* Framebuffer Object
* Deferred Shading
* Shadow Rendering
* Normal Mapping
* Text Rendering


## About The Project
### Introduction
Experience is a small expressive game about the process of trying to improve at something.
The game involves walking through various environments which evoke feelings like those experienced when learning something new. <br /><br />
[![Game Preview](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/exp_cover.png?raw=true)](https://www.youtube.com/watch?v=9Me2BgbvU00)
* Click on the image above for Demo

### Gameplay
The game takes place from a first-person perspective. The gameplay is as simple as walking and observing. The player can interact with their environment, reading notes strewn about the environment and talking with animals.

### Environment
The environments of the game are threefold. To travel between the environments, the player can simply walk into the fog at the edge of each environment whenever they choose.

#### Forest - A new beginning!
At first, the player will find themselves in a quiet wood. This area is intended to evoke feelings of a fresh start. Secrets are found around every corner, and the world is new and interesting.
Overview             |  POV
:-------------------------:|:-------------------------:
![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/f1.png?raw=true)  |  ![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/f2.png?raw=true)

#### Desert - Arduous Journey
Next, the player will find themselves in a desert wasteland, with long stretches of empty land and steep sand dunes. Bloom and Blur effects make the journey feel arduous. Deep in the desert you may find secrets, but they are few and far between, and this area evokes the feeling of being overwhelmed by your task.
Overview             |  POV
:-------------------------:|:-------------------------:
![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/d1.png?raw=true)  |  ![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/d2.png?raw=true)

#### Street - Completion and Loneliness
Finally, the player will find themselves on a dark street in the middle of the night. No stars in the sky, just loneliness. streetlamps light the path. You have completed your journey, and overcome much, and have seen the fleeting beauty of the journey you've traveled, but now you find yourself alone. Only you know what you have learned.
Overview             |  POV
:-------------------------:|:-------------------------:
![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/s1.png?raw=true)  |  ![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/s2.png?raw=true)

Nothing else to do, then, than to start a new journey!

#### Credit - With Secret Ending
Collect all the notes and messages to find out!
### Technical Details
#### Instanced Rendering
```c++
for(int i = 0; i < entry.model->meshes.size(); i++)
{
    glBindVertexArray(entry.model->meshes[i].VAO);
    glDrawElementsInstanced(GL_TRIANGLES,
    static_cast<unsigned int>(entry.model->meshes[i].indices.size()),
        GL_UNSIGNED_INT, 0,
        modelMatrices.size());
        glBindVertexArray(0);
}
```
Models and particles are rendered using Instancing, increasing the maximum render cap by a sizable margin.

#### Shadow Mapping
```c++
void render()
{
    shader.use();
    glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    RenderScene(shader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
```
![Shadow](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/shadow.png?raw=true)
Our game makes a first render pass to a depth buffer, which is then used later in the pipeline to calculate dynamic shadows.

#### High Dynamic Range
High Dynamic Range, in conjunction with Bloom/Blur, creates a very vibrant and lively scene. We calculate excess color above the standard white maximum, and we perform gamma correction to ensure that the scene fits within a certain exposure.

#### Bloom and Blur
The game also uses a Bloom effect to simulate bright lights. We find bright segments (above a certain threshold), and we render these to their own Framebuffer. We then perform a Gaussian Blur on that framebuffer, which we merge with the original image to get our bloomed result.
Before             |  After
:-------------------------:|:-------------------------:
![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/before_bloomblur.png?raw=true)  |  ![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/after_bloomblur.png?raw=true)

#### View Frustum Culling
```c++
bool ViewFrustCull(vec3 center, float radius)
{
    float dist;
    for(int i=0; i < 6; i++){
        dist = DistToPlane(planes[i].x, planes[i].y, planes[i].z, planes[i].w, center);
        if(dist < 0 && fabs(dist) > radius){
            return true;
        }
     }
     return false;
}
```
Before             |  After
:-------------------------:|:-------------------------:
![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/before_cull.png?raw=true)  |  ![](https://github.com/CPE-476/Experience/blob/main/Final/HTML/src/after_cull.png?raw=true)

* Notice that the drawn objects decrease from 14616 to 3123

The game needs to be as performant as possible, especially considering the amount of models that we render. For this to be feasible, we implemented View Frustum Culling. It calculates the area that can be viewed, and performs spatial queries to determine if a given object should be drawn.

#### Distance Fog
Our game includes distance fog to simulate depth more accurately. It can be tweaked in the editor.

#### Level Editor
We achieved a rich, detailed set of environments by constructing a level editor. We save files to an ASCII format and load them into our game. It allows you to tweak every aspect of the levels, from particles to distance fog.

## Getting Started
### Built With
The CMake is not tested on Windows or Linux.
It uses find-package to search for libraries, which I believe searches the source tree as well as some places on your computer, but definitely some expertise will be required if encountering any bugs.

I would recommend installing each library using a package manager like Brew if you can, and otherwise putting them in a usual spot on your system where CMake is likely to find them.

### References
LearnOpenGL: The best resource - CGTrader - Sketchfab

### Dependencies
GLFW - GLM - Assimp - Freetype

### Included
GLAD - ImGui - tinyobjloader - miniaudio - stbimage

### Contact
Lucas Li - yli76@calpoly.edu<br />
Alex Hartford - alexanderhartford@gmail.com <br />
Brett Hickman - bahickma@calpoly.edu
