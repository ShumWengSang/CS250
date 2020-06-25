// FrustumMesh.cpp
// Roland Shum
// Assignment 3

#include "FrustumMesh.h"
#include "Affine.h"

const cs250::Mesh::Face cs250::FrustumMesh::faces[12] =
{
	// Near Face
	{1, 2, 4}, 
	{4, 3, 1}, 

	// Face face
	{5, 7, 8}, 
	{8, 6, 5}, 

	// Support left
	{2, 6, 8}, 
	{8, 4, 2}, 

	// Support Right
	{7, 5, 1}, 
	{1, 3, 7}, 

	// Support top
	{5,6,2},
	{2,1,5},

	// Support bottom
	{3,4,8},
	{8,7,3}
};

const cs250::Mesh::Edge cs250::FrustumMesh::edges[16] =
{
	// Eye to near
	{0, 1},
	{0, 2},
	{0, 3},
	{0, 4},

	// Near Face
	{2, 1},
	{1, 3},
	{3, 4},
	{4, 2},

	// Far Face
	{6, 5},
	{5, 7},
	{7, 8},
	{8, 6},

	// Support edges
	{2, 6},
	{1, 5},
	{3, 7},
	{4, 8}

};

cs250::FrustumMesh::FrustumMesh(float fov, float aspect, float N, float F)
{
	// We assume the eye is at the origin
	vertices[0] = { 0,0,0,0 };

	// Let our D be the near face
	float D = N;

	float radians = glm::radians(fov);

	// Now we calculate the near face rectangle points
	float nearFaceWidth = 2 * D * std::tanf(radians / 2);
	float nearFaceHeight = nearFaceWidth / aspect;

	// Near face rectangle
	vertices[1] = { nearFaceWidth / 2, nearFaceHeight / 2, -N, 1 }; // Top right
	vertices[2] = { -nearFaceWidth / 2, nearFaceHeight / 2, -N, 1 }; // Top left
	vertices[3] = { nearFaceWidth / 2, -nearFaceHeight / 2, -N, 1 }; // Bottom Right
	vertices[4] = { -nearFaceWidth / 2, -nearFaceHeight / 2, -N, 1 }; // Bottom left

	// Calculate Far face
	D = F;
	float farFaceWidth = 2 * D * std::tanf(radians / 2);
	float farFaceHeight = farFaceWidth / aspect;
	
	// Far face rectangle
	vertices[5] = { farFaceWidth / 2, farFaceHeight / 2, -F, 1 }; // Top right
	vertices[6] = { -farFaceWidth / 2, farFaceHeight / 2, -F, 1 }; // Top left
	vertices[7] = { farFaceWidth / 2, -farFaceHeight / 2, -F, 1 }; // Bottom Right
	vertices[8] = { -farFaceWidth / 2, -farFaceHeight / 2, -F, 1 }; // Bottom left

	aabb_center = glm::vec4(0);
	aabb_dimensions = glm::vec4(1);
}
