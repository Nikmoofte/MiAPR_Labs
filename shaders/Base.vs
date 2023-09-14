#version 330

layout (location = 0) in vec2 aPos;

uniform float maxCoord = 10.f;

void main()
{
    vec2 Pos = aPos / maxCoord * 2 - 1.0f;
    gl_Position = vec4(Pos, 0.0f, 1.0f);
}