// Mesh.h
// -- 3D triangular mesh interfaces
// cs250 5/19

#ifndef CS250_MESH_H
#define CS250_MESH_H


#include <glm/glm.hpp>


namespace cs250 {

  struct Mesh {

    struct Face {
      int index1, index2, index3;
      Face(int i=-1, int j=-1, int k=-1)
          : index1(i), index2(j), index3(k) {}
    };

    struct Edge {
      int index1, index2;
      Edge(int i=-1, int j=-1)
          : index1(i), index2(j) {}
    };

    virtual ~Mesh(void) {}
    virtual int vertexCount(void) const = 0;
    virtual const glm::vec4* vertexArray(void) const = 0;
    virtual glm::vec4 dimensions(void) const = 0;
    virtual glm::vec4 center(void) const = 0;
    virtual int faceCount(void) const = 0;
    virtual const Face* faceArray(void) const = 0;
    virtual int edgeCount(void) const = 0;
    virtual const Edge* edgeArray(void) const = 0;
  };


  struct NormalMesh : Mesh {
    virtual const glm::vec4* normalArray(void) const = 0;
  };


  NormalMesh* createFlatShadedMesh(const Mesh &m);

}


#endif

