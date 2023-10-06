#include "Point.hpp"

#include <random>

#include "VAO/VBLayout.h"

extern const float maxCoord; 
extern const float minCoord;

Point2D::Point2D() : Point2D(glm::vec2(0.f)) 
{
}

Point2D::Point2D(const glm::vec2 &pos) : pos{pos}, buffilled{false}
{
    fillBuffer();
    VBLayout lo;
    lo.Push<float>(2);
    va.setLayout(vb, lo);
}

const glm::vec2 &Point2D::getPos() const
{
    return pos;
}

void Point2D::setPos(const glm::vec2 & _pos)
{
    pos = glm::max(glm::vec2(minCoord), glm::min(glm::vec2(maxCoord), _pos)); 
    _pos;
    buffilled = false;
}

const glm::vec3 &Point2D::getColor() const
{
    return color;
}

void Point2D::setColor(const glm::vec3 &_color)
{
    color = _color;
}

void Point2D::fillBuffer()
{
    vb.SetBufferData(sizeof(glm::vec2), getData(), GL_STATIC_DRAW);
    buffilled = true;
}
