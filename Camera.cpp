// CS250
// Roland Shum
// Assignment 3
// Camera.cpp
#include "Camera.h"
#include <cmath>
#include "Affine.h"

using Radians = float;
using Degrees = float;

cs250::Camera::Camera(void) : eye_point(glm::vec4(0)), right_vector(glm::vec4(1, 0, 0, 0)),
up_vector(glm::vec4(0, 1, 0, 0)), back_vector(glm::vec4(0, 0, 0, 1)), near_distance(0.1f), far_distance(near_distance),
vp_distance(5)
{
	// Aspect ratio of 1, compile time evaluation
	constexpr Radians angle = glm::radians(90.0f / 2);
	vp_height = vp_width = std::tanf(angle)* vp_distance * 2;

}

cs250::Camera::Camera(const glm::vec4& E, const glm::vec4& look, const glm::vec4& rel, float fov, float aspect, float N, float F) :
	eye_point(E), back_vector(glm::normalize( look * -1.0f)), up_vector(glm::normalize(rel)), right_vector(glm::normalize(cs250::cross(look, rel))), near_distance(N), far_distance(F), vp_distance(N)
{
	Degrees const FOV = fov;
	float aspectRatio = aspect;

	Radians radian = glm::radians(FOV / 2);
	vp_width = std::tanf(radian) * vp_distance * 2;
	vp_height = vp_width / aspectRatio;
}

glm::vec4 cs250::Camera::viewport(void) const
{
	return glm::vec4(vp_width, vp_height, vp_distance, 0);
}

cs250::Camera& cs250::Camera::zoom(float factor)
{
	this->vp_height *= factor;
	this->vp_width *= factor;
	return *this;
}

cs250::Camera& cs250::Camera::forward(float distance)
{
	glm::vec4 movementVector = this->back_vector * -distance;
	this->eye_point += movementVector;
	return *this;
}

cs250::Camera& cs250::Camera::yaw(float angle)
{
	auto rotationMatrix = cs250::rotate(angle, up_vector);
	right_vector = rotationMatrix * right_vector;
	back_vector = rotationMatrix * back_vector;
	return *this;
}

cs250::Camera& cs250::Camera::pitch(float angle)
{
	auto rotationMatrix = cs250::rotate(angle, right_vector);
	up_vector = rotationMatrix * up_vector;
	back_vector = rotationMatrix * back_vector;
	return *this;
}

cs250::Camera& cs250::Camera::roll(float angle)
{
	auto rotationMatrix = cs250::rotate(angle, back_vector);
	right_vector = rotationMatrix * right_vector;
	up_vector = rotationMatrix * up_vector;
	return *this;
}
