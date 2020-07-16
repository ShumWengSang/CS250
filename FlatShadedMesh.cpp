// Roland Shum
// Assignment 2
// CS250
// FlatShadedMesh.cpp

#include "Affine.h"
#include "CubeMesh.h"
#include <vector>

struct NMesh : cs250::NormalMesh
{
	int vertexCount(void) const override;
	const glm::vec4* vertexArray(void) const override;
	glm::vec4 dimensions(void) const override;
	glm::vec4 center(void) const override;
	int faceCount(void) const override;
	const Face* faceArray(void) const override;
	int edgeCount(void) const override;
	const Edge* edgeArray(void) const override;
	const glm::vec4* normalArray(void) const override;

	std::vector<glm::vec4> vertices;
	std::vector<glm::vec4> normals;
	std::vector<Mesh::Face> faces;
	glm::vec4 _dimensions, _center;
};

cs250::NormalMesh* cs250::createFlatShadedMesh(const Mesh& m)
{
	NMesh* normalizedMesh = new NMesh();
	normalizedMesh->vertices.reserve(m.faceCount() * 3);
	normalizedMesh->faces.reserve(m.faceCount());
	normalizedMesh->normals.reserve(m.faceCount() * 3);

	// Insert into vertices the seperated vertexes
	for (int i = 0, vertexCount = 0; i < m.faceCount(); ++i, vertexCount += 3)
	{
		// Using the index available from faces, insert vertex into array
		const cs250::Mesh::Face& face = m.faceArray()[i];
		normalizedMesh->vertices.emplace_back(m.vertexArray()[face.index1]);
		normalizedMesh->vertices.emplace_back(m.vertexArray()[face.index2]);
		normalizedMesh->vertices.emplace_back(m.vertexArray()[face.index3]);

		// For every three vertex we push in, we make a face.

		int currentCount = vertexCount;
		// Emplace in the last 3 indexes
		normalizedMesh->faces.emplace_back(cs250::Mesh::Face{ currentCount, currentCount + 1, currentCount + 2});

		// For every face, we calculate the normal for that face.
		glm::vec4 P = normalizedMesh->vertices[currentCount];
		glm::vec4 Q = normalizedMesh->vertices[currentCount + 1];
		glm::vec4 R = normalizedMesh->vertices[currentCount + 2];

		glm::vec4 normal = cs250::cross(Q - P, R - P);
		normal = glm::normalize(normal);
		// Insert
		normalizedMesh->normals.emplace_back(normal);
		normalizedMesh->normals.emplace_back(normal);
		normalizedMesh->normals.emplace_back(normal);
	}
	normalizedMesh->_dimensions = m.dimensions();
	normalizedMesh->_center = m.center();

	return normalizedMesh;
}

int NMesh::vertexCount(void) const
{
	return vertices.size();
}

const glm::vec4* NMesh::vertexArray(void) const
{
	return vertices.data();
}

glm::vec4 NMesh::dimensions(void) const
{
	return _dimensions;
}

glm::vec4 NMesh::center(void) const
{
	return _center;
}

int NMesh::faceCount(void) const
{
	return faces.size();
}

const cs250::Mesh::Face* NMesh::faceArray(void) const
{
	return faces.data();
}

int NMesh::edgeCount(void) const
{
	return 0;
}

const cs250::Mesh::Edge* NMesh::edgeArray(void) const
{
	return nullptr;
}

const glm::vec4* NMesh::normalArray(void) const
{
	return normals.data();
}
