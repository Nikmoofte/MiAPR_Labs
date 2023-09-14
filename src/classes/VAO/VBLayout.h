#ifndef VBLAYOUT_H
#define VBLAYOUT_H

#include <glad/glad.h>
#include <cstddef>
#include <vector>
#include <concepts>

#include "VBO/VBO.h"

struct VBElement
{
    unsigned type;
    unsigned count;
    GLboolean normalized;

    static unsigned GetSizeOfType(GLenum type)
    {
        switch (type)
        {
            case GL_FLOAT:          return sizeof(GLfloat);
            case GL_UNSIGNED_INT:   return sizeof(GLuint);
            case GL_UNSIGNED_BYTE:  return sizeof(GLbyte);        
            
            default:                return 0;
        }
    }
};

class VBLayout
{
public:
    VBLayout() : stride(0) {}

    template<class T>
    void Push(unsigned count) = delete;
    template<>
    void Push<float>(unsigned count)
    {
        elements.push_back({GL_FLOAT, count, GL_FALSE});
        stride += count * VBElement::GetSizeOfType(GL_FLOAT);
    }
    template<>
    void Push<unsigned>(unsigned count)
    {
        elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
        stride += count * VBElement::GetSizeOfType(GL_UNSIGNED_INT);
    }
    template<>
    void Push<unsigned char>(unsigned count)
    {
        elements.push_back({GL_UNSIGNED_BYTE, count, GL_FALSE});
        stride += count * VBElement::GetSizeOfType(GL_UNSIGNED_BYTE);
    }

    inline const auto& GetElements() const {return elements;}
    inline const auto& GetStride() const {return stride;}

private:
    unsigned stride;
    std::vector<VBElement> elements;
};




#endif