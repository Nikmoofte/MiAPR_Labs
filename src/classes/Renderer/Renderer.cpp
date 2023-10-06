#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

void Renderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw(const VAO &va, const EBO &ib, const ShaderProg &prog)
{
    va.Bind();
    ib.Bind();
    prog.Use();

    glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, 0);
}

void Renderer::Draw(Points &points, ShaderProg &prog)
{ 
    if(!points.buffilled)
        points.fillBuffer();
    points.va.Bind();
    points.vb.Bind();
    prog.Use();
    auto color = points.color;
    glUniform3fv(prog.GetLocation("color"), 1, (GLfloat*)&color);

    glDrawArrays(GL_POINTS, 0, points.getSize());
}

void Renderer::Draw(Point2D& point, ShaderProg &prog)
{
    if(!point.buffilled)
        point.fillBuffer();
    point.va.Bind();
    point.vb.Bind();
    prog.Use();
    
    auto color = point.color;
    glUniform3fv(prog.GetLocation("color"), 1, (GLfloat*)&color);

    glDrawArrays(GL_POINTS, 0, 1);
}
