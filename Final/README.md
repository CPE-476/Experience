# Experience
A project of graphic proportions.

## Building
The CMake is not perfect, I have not tested on Windows, Linux, or old Mac.
It uses find-package to search for libraries, which I believe searches the source tree as well as some places on your computer, but definitely some expertise will be required if encountering any bugs.

I would recommend installing each library using a package manager like Brew if you can, and otherwise putting them in a usual spot on your system where CMake is likely to find them.

### Dependencies
GLFW
GLM
Assimp
Freetype

### Included
GLAD
tinyobjloader
miniaudio
stbimage
