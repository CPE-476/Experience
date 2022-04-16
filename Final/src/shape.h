#ifndef SHAPE_H
#define SHAPE_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <cassert>

#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>

void resize_obj(std::vector<tinyobj::shape_t> &shapes);

class Shape
{
public:
    void createShape(tinyobj::shape_t & shape);
    void init();
    void draw(Shader shader) const;

private:
    std::vector<unsigned int> elements;
    std::vector<float> positions;
    std::vector<float> norBuf;
    std::vector<float> texBuf;
    unsigned int EBO = 0;
    unsigned int VBO = 0;
    unsigned int norBufID = 0;
    unsigned int texBufID = 0;
    unsigned int VAO = 0;
};

using namespace std;

// copy the data from the objloader shape to this object
void Shape::createShape(tinyobj::shape_t & shape)
{
    positions = shape.mesh.positions;
    norBuf = shape.mesh.normals;
    texBuf = shape.mesh.texcoords;
    elements = shape.mesh.indices;
}

void Shape::init()
{
    // Initialize the vertex array object
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Send the position array to the GPU
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(float), &positions[0], GL_STATIC_DRAW);

    // Send the normal array to the GPU
    if (norBuf.empty())
    {
        norBufID = 0;
        cout << "warning no normals!" << endl;
    }
    else
    {
        glGenBuffers(1, &norBufID);
        glBindBuffer(GL_ARRAY_BUFFER, norBufID);
        glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
    }

    // Send the texture array to the GPU
    if (texBuf.empty())
    {
        texBufID = 0;
        cout << "warning no textures!" << endl;
    }
    else
    {
        glGenBuffers(1, &texBufID);
        glBindBuffer(GL_ARRAY_BUFFER, texBufID);
        glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
    }

    // Send the element array to the GPU
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size()*sizeof(unsigned int), &elements[0], GL_STATIC_DRAW);

    // Unbind the arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Shape::draw(Shader shader) const
{
    glBindVertexArray(VAO);

    // Bind position buffer
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    // Bind normal buffer
    if (norBufID != 0)
    {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, norBufID);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }

    if (texBufID != 0)
    {
        // Bind texcoords buffer
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, texBufID);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }

    // Bind element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Draw
    glDrawElements(GL_TRIANGLES, (int)elements.size(), GL_UNSIGNED_INT, (const void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/* Transforms new shapes to always be the same size. 
 * [-1, 1] in all three axes. */
void resize_obj(std::vector<tinyobj::shape_t> &shapes)
{
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    float scaleX, scaleY, scaleZ;
    float shiftX, shiftY, shiftZ;
    float epsilon = 0.001;

    minX = minY = minZ = 1.1754E+38F;
    maxX = maxY = maxZ = -1.1754E+38F;

    //Go through all vertices to determine min and max of each dimension
    for (size_t i = 0; i < shapes.size(); i++) {
        for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
            if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
            if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];

            if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
            if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];

            if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
            if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
        }
    }

    //From min and max compute necessary scale and shift for each dimension
    float maxExtent, xExtent, yExtent, zExtent;
    xExtent = maxX-minX;
    yExtent = maxY-minY;
    zExtent = maxZ-minZ;
    if (xExtent >= yExtent && xExtent >= zExtent) {
        maxExtent = xExtent;
    }
    if (yExtent >= xExtent && yExtent >= zExtent) {
        maxExtent = yExtent;
    }
    if (zExtent >= xExtent && zExtent >= yExtent) 
    {
        maxExtent = zExtent;
    }
    scaleX = 2.0 /maxExtent;
    shiftX = minX + (xExtent/ 2.0);
    scaleY = 2.0 / maxExtent;
    shiftY = minY + (yExtent / 2.0);
    scaleZ = 2.0/ maxExtent;
    shiftZ = minZ + (zExtent)/2.0;

    //Go through all vertices shift and scale them
    for (size_t i = 0; i < shapes.size(); i++) {
        for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
            shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
            assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
            assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
            shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
            assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
            assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
            shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
            assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
            assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
        }
    }
}

#endif
