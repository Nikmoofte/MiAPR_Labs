#ifndef POINT_HPP
#define POINT_HPP

#include <vector>
#include <VAO/VAO.h>
#include <VBO/VBO.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


struct Point2D
{
    Point2D(const glm::vec2& pos);
    Point2D();

    const glm::vec2& getPos() const;
    void setPos(const glm::vec2&);

    const glm::vec3& getColor() const;
    void setColor(const glm::vec3&);
    
    auto getData() {return glm::value_ptr(pos);}

    friend class Renderer;
private:
    void fillBuffer();
    bool buffilled = false;
    glm::vec2 pos;
    glm::vec3 color;
    VBO vb;
    VAO va;
};

#endif