#ifndef EBO_H
#define EBO_H

#include <glad/glad.h>
#include <stdint.h>


class EBO
{
public:
    typedef uint32_t indexT;
    EBO();
    EBO(size_t count, indexT* data, GLenum usage = GL_STATIC_DRAW);
    ~EBO();
    void Bind() const;
    void Unbind() const;
    inline unsigned GetId() const { return id; };
    inline size_t GetCount() const { return count; };
    void SetBufferData(size_t count, GLvoid* data, GLenum usage);
private:
    unsigned id;
    size_t count;
    GLenum target = GL_ELEMENT_ARRAY_BUFFER;

};

#endif