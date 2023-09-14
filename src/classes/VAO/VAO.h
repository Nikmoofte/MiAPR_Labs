#ifndef VAO_H
#define VAO_H

#include <glad/glad.h>
#include <cstddef>  

#include "VBO/VBO.h"
#include "VBLayout.h"


class VAO
{
public:
    VAO();
    ~VAO();

    void Bind() const;
    void Unbind() const;

    void setLayout(const VBO& vb, const VBLayout& vbLayout);
private:
    unsigned id;
};

#endif