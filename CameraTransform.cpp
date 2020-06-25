// Shum Weng Sang
// Assignment 5
// CS 250
#include "Camera.h"
#include "Affine.h"

namespace cs250 
{
	// Maps a canonical camera to world space (camera to world)
	glm::mat4 model(const Camera& cam)
	{
		return affine(cam.right(), cam.up(), cam.back(), cam.eye());
	}

	// Maps a world to camera. Use this to view the world from camera.
	glm::mat4 view(const Camera& cam)
	{
		return affineInverse(affine(cam.right(), cam.up(), cam.back(), cam.eye()));
	}

	// Perspective transformation that maps camera to standard cube. (NDC)
	glm::mat4 perspective(const Camera& cam)
	{
		float W = cam.viewport().x;
		float H = cam.viewport().y;
		float D = cam.viewport().z;


		glm::mat4 res(1); // Initialize to identity


		res[0][0] = 2 * D / W;
		res[1][1] = 2 * D / H;
		res[2][2] = (cam.near() + cam.far()) / (cam.near() - cam.far());
		res[3][3] = 0;



		res[2][3] = -1;
		res[3][2] = (2 * cam.near() * cam.far()) / (cam.near() - cam.far());

		return res;
	}
}