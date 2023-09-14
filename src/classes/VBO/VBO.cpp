#include "VBO.h"

VBO::VBO()
{
    glGenBuffers(1, &id);
}


VBO::VBO(GLsizeiptr size, GLvoid *data, GLenum usage) : VBO()
{
    SetBufferData(size, data, usage);
}

VBO::~VBO()
{
    glDeleteBuffers(1, &id);
}

void VBO::Bind() const
{
    glBindBuffer(target, this->id);
}

void VBO::Unbind() const
{
    glBindBuffer(target, 0);
}

void VBO::SetBufferData(GLsizeiptr size, GLvoid *data, GLenum usage)
{
    Bind();
    glBufferData(target, size, data, usage);
}

unsigned VBO::GetId() const
{
    return id;
}
