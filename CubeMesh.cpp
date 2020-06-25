// Roland Shum
// Assignment 2
// CS250
// CubeMesh.cpp

#include "Affine.h"
#include "CubeMesh.h"

// in a cpp file where the static variable will exist 
const glm::vec4 cs250::CubeMesh::vertices[8] = 
{ 
	// Top 4 points
	{1.0f, 1.0f, -1.0f, 1.0f},
	{-1.0f, 1.0f, -1.0f, 1.0f},
	{-1.0f, 1.0f, 1.0f, 1.0f},
	{1.0f, 1.0f, 1.0f, 1.0f },


	// Bottom 4 points
	{1.0f, -1.0f, -1.0f, 1.0f},
	{-1.0f, -1.0f, -1.0f, 1.0f},
	{-1.0f, -1.0f, 1.0f, 1.0f},
	{1.0f, -1.0f, 1.0f, 1.0f }
};

const cs250::Mesh::Face cs250::CubeMesh::faces[12] =
{
	{0, 1, 2}, // Top
	{2, 3, 0}, // Top
	{3, 2, 6}, // Forward
	{6, 7, 3}, // forward
	{2, 1, 5}, // Left
	{5, 6, 2}, // Left
	{4, 0, 3}, // Right
	{3, 7, 4}, // Right
	{5, 1, 0}, // Back
	{0, 4, 5}, // Back
	{5, 4, 7}, // Bottom
	{7, 6 ,5}  // Bottom
};

const cs250::Mesh::Edge cs250::CubeMesh::edges[12] =
{
	// top four edges
	{0, 1},
	{1, 2},
	{2, 3},
	{3, 0},

	// Bottom four edges
	{4, 5},
	{5, 6},
	{6, 7},
	{7, 4},

	// Support edges
	{0, 4},
	{1, 5},
	{2, 6},
	{3, 7}
};

int cs250::CubeMesh::vertexCount(void) const
{
	return 8;
}

const glm::vec4* cs250::CubeMesh::vertexArray(void) const
{
	return &vertices[0];
}

glm::vec4 cs250::CubeMesh::dimensions(void) const
{
	glm::vec4 res = { 2,2,2,0 };
	return res;
}

glm::vec4 cs250::CubeMesh::center(void) const
{
	return glm::vec4(0, 0, 0, 1);
}

int cs250::CubeMesh::faceCount(void) const
{
	return 12;
}

const cs250::Mesh::Face* cs250::CubeMesh::faceArray(void) const
{
	return &faces[0];
}

int cs250::CubeMesh::edgeCount(void) const
{
	return 12;
}

const cs250::Mesh::Edge* cs250::CubeMesh::edgeArray(void) const
{
	return &edges[0];
}
