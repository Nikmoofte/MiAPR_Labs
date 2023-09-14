#include "VAO.h"
VAO::VAO()
{
    glGenVertexArrays(1, &this->id);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &this->id);
}

void VAO::Bind() const
{
    glBindVertexArray(this->id);
}

void VAO::Unbind() const
{
    glBindVertexArray(0);
}

void VAO::setLayout(const VBO& vb, const VBLayout &vbLayout)
{
    Bind();
    vb.Bind();
    const auto& elements = vbLayout.GetElements();
    unsigned offset = 0;
    size_t end = elements.size();
    for (size_t i = 0; i < end; i++)
    {
        const auto& element = elements[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, element.normalized, vbLayout.GetStride(), (const void*)offset);
        offset += element.count * element.GetSizeOfType(element.type);
    }
}
