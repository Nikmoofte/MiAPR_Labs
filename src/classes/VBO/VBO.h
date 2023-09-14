#ifndef VBO_H
#define VBO_H

#include <glad/glad.h>

class VBO
{
public:
    VBO();
    VBO(GLsizeiptr size, GLvoid* data, GLenum usage = GL_STATIC_DRAW);
    ~VBO();
    void Bind() const;
    void Unbind() const;
    unsigned GetId() const;
    void SetBufferData(GLsizeiptr size, GLvoid* data, GLenum usage);
private:
    unsigned id;
    GLenum target = GL_ARRAY_BUFFER;

};

#endif