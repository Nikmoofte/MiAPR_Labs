#ifndef POINTS_HPP
#define POINTS_HPP

#include <vector>
#include <VAO/VAO.h>
#include <VBO/VBO.h>


struct vec2
{
    float x, y;
    vec2 operator-(const vec2& other) 
    {
        vec2 res;
        res.x = x - other.x;
        res.y = y - other.y;
        return res;
    }
    vec2 operator+=(const vec2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    vec2 operator/=(const vec2& other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }
    vec2 operator/=(float scale)
    {
        x /= scale;
        y /= scale;
        return *this;
    }
    float getDist()
    {
        return sqrt(x * x + y * y);
    }
};
struct vec3
{
    float x, y, z;
};


struct Points
{
    Points(size_t numOfPoints);
    Points();
    void Push(vec2 pos) 
    {
        data.push_back(pos); 
        buffilled = false;
    }
    void clear() 
    {
        data.clear(); 
        buffilled = false;
    }
    void resize(size_t newSize) { data.resize(newSize); buffilled = false;}
    size_t getSize() const {return data.size();}
    void setColor(const vec3& color);
    vec2& operator[](size_t index) {return data[index]; buffilled = false;}
    const vec2& operator[](size_t index) const {return data[index];}
    void randomFill();
    void randomFill(size_t index);
    void fillBuffer();
    friend class Renderer;
private:
    auto getData() {return data.front();}
    bool buffilled = false;
    std::vector<vec2> data;
    vec3 color;
    VBO vb;
    VAO va;
};

#endif