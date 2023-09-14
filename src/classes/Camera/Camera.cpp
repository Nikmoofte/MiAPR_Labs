#include "Camera.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>


extern unsigned RenderDist;

Camera::Camera(const glm::vec3& vPos, uint16_t uScreenWidth, uint16_t uScreenHeight)
{
	CameraPos = vPos;
	this->uScreenHeight = uScreenHeight;
	this->uScreenWidth = uScreenWidth;
	proj = glm::perspective(glm::radians(90.f), static_cast<float>(uScreenWidth) / uScreenHeight, 0.1f, 100.0f);

	view = lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
}

void Camera::MouseControl(GLFWwindow* window)
{
	using namespace glm;

	float fSensivity = 0.005f;

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	if (bFirst || (x > uScreenWidth - 100) || (x < 100))
	{
		glfwSetCursorPos(window, uScreenWidth / 2, y);
		uLastX = uScreenWidth / 2;
		uLastY = y;
		bFirst = false;
	}

	int iDeltaX = x - uLastX;
	int iDeltaY = uLastY - y;
	uLastX = x;
	uLastY = y;



	fYaw += iDeltaX * fSensivity;
	fPitch += iDeltaY * fSensivity;

	if (fPitch > pi<float>() / 4)
		fPitch = pi<float>() / 4;
	if (fPitch < -pi<float>() / 4)
		fPitch = -pi<float>() / 4;

	if (fYaw > 2 * pi<float>())
		fYaw -= 2 * pi<float>();
	if (fYaw < 2 * pi<float>())
		fYaw += 2 * pi<float>();

	vec3 direction;
	direction.x = cos(fYaw) * cos(fPitch);
	direction.y = sin(fPitch);
	direction.z = sin(fYaw) * cos(fPitch);

	CameraFront = normalize(direction);
	view = lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
}

void Camera::SetPos(const glm::vec3 &vNewPos)
{
	CameraPos = vNewPos;
}

const glm::mat4 &Camera::GetProjMat() const
{
	return proj;
}

const glm::mat4& Camera::GetViewMat() const
{
	return view;
}

const glm::vec3 &Camera::GetPos() const
{
    return CameraPos;
}


/*float GetHeight(const glm::vec2& pos, Map& map)
{
	const size_t sideLen = (map.getChunk(0, 0)->buffer)->GetSideLen() - 1;

	// Determine grid cell coordinates
	int x0 = (int)floor(pos.x);
	int x1 = x0 + 1;
	int y0 = (int)floor(pos.y);
	int y1 = y0 + 1;

	// Determine interpolation weights
	// Could also use higher order polynomial/s-curve here
	float sx = pos.x - (float)x0;
	float sy = pos.y - (float)y0;

	int cx0 = x0 / sideLen;
	int cx1 = x1 / sideLen;
	int cy0 = y0 / sideLen;
	int cy1 = y1 / sideLen;

	float z00 = map.getChunk(cx0, cy0)->buffer->Get(pos.x - x0, pos.y - y0);
	float z01 = map.getChunk(cx0, cy1)->buffer->Get(pos.x - x0, pos.y - y1);
	float z10 = map.getChunk(cx1, cy0)->buffer->Get(pos.x - x1, pos.y - y0);
	float z11 = map.getChunk(cx1, cy1)->buffer->Get(pos.x - x1, pos.y - y1);

	float zy0 = interpolate(z00, z01, sy);
    float zy1 = interpolate(z10, z11, sy);

	float interp_val = interpolate(zy0, zy1, sx);

	return interp_val;

}*/
/*glm::vec2 randomGradient(int ix, int iy)  
{
	// No precomputed gradients mean this works for any number of grid coordinates 3284157443
	const unsigned w = 8 * sizeof(unsigned);
	const unsigned s = w / 2; // rotation width
	unsigned a = ix, b = iy;
	a *= 93498348U; b ^= a << s | a >> w-s;
	b *= 1911520717; a ^= b << s | b >> w-s;
	a *= 2048419325;
	float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
	return glm::vec2(glm::cos(random), glm::sin(random));
}
float dotGridGradient(int ix, int iy, float x, float y) 
{
	// Get gradient from integer coordinates
	glm::vec2 gradient = randomGradient(ix, iy);

	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	// Compute the dot-product
	return (dx*gradient.x + dy*gradient.y);
}

float perlin(float x, float y) 
{
	// Determine grid cell coordinates
	int x0 = (int)floor(x);
	int x1 = x0 + 1;
	int y0 = (int)floor(y);
	int y1 = y0 + 1;

	// Determine interpolation weights
	// Could also use higher order polynomial/s-curve here
	float sx = x - (float)x0;
	float sy = y - (float)y0;

	// Interpolate between grid point gradients
	float n0, n1, ix0, ix1, value;

	n0 = dotGridGradient(x0, y0, x, y);
	n1 = dotGridGradient(x1, y0, x, y);
	ix0 = interpolate(n0, n1, sx);

	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	ix1 = interpolate(n0, n1, sx);

	value = interpolate(ix0, ix1, sy);
	return value; // Will return in range -1 to 1. To make it in range 0 to 1, multiply by 0.5 and add 0.5
}

float generate(float x, float y) 
{
	float res = perlin(x, y);
	x /= 8; y /= 8;
	res += perlin(x, y);
	x /= 2; y /= 2;
	res += perlin(x, y);
	x /= 2; y /= 2;
	res += 10 * perlin(x, y);
	x /= 2; y /= 2;
	res += 10 * perlin(x, y);
	x /= 2; y /= 2;
	res += 40 * perlin(x, y);
	return res;
}*/

float interpolate(float a0, float a1, float w)  
{
	return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
	//return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
	// return (a1 - a0) * w + a0;
}
void Camera::KeyboardControl(GLFWwindow* window, std::chrono::duration<float> tFrameTime)
{

	float fSpeed = 1.f;

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		fSpeed = 5.0f;

	float fMovSpeed = fSpeed * tFrameTime.count();


	// if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	// {
	// 	CameraPos -= fMovSpeed * CameraUp;
	// }
	
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		CameraPos += fMovSpeed * CameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		CameraPos -= fMovSpeed * CameraFront;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		glm::vec3 right = normalize(cross(CameraFront, CameraUp));

		CameraPos += fMovSpeed * right;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		glm::vec3 right = normalize(cross(CameraFront, CameraUp));
		CameraPos -= fMovSpeed * right;
	}

}
