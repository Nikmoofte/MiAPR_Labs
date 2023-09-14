#include "EBO.h"
#include <iostream>

EBO::EBO()
{
    glGenBuffers(1, &id);
}

EBO::EBO(size_t count, uint32_t* data, GLenum usage) : EBO()
{
    SetBufferData(count * sizeof(uint32_t), data, usage);
}

EBO::~EBO()
{
    glDeleteBuffers(1, &id);
}

void EBO::Bind() const
{
    glBindBuffer(target, this->id);
}

void EBO::Unbind() const
{
    glBindBuffer(target, 0);
}

void EBO::SetBufferData(size_t count, GLvoid *data, GLenum usage)
{
    Bind();
    this->count = count;
    glBufferData(target, count * sizeof(GLuint), data, usage);
}

