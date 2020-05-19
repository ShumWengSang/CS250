// CubeMesh.h
// -- standard cube
// cs250 5/19

#ifndef CS250_CUBEMESH_H
#define CS250_CUBEMESH_H

#include "Mesh.h"


namespace cs250 {

  class CubeMesh : public Mesh {
    public:
      int vertexCount(void) const override;
      const glm::vec4* vertexArray(void) const override;
      glm::vec4 dimensions(void) const override;
      glm::vec4 center(void) const override;
      int faceCount(void) const override;
      const Face* faceArray(void) const override;
      int edgeCount(void) const override;
      const Edge* edgeArray(void) const override;
    private:
      static const glm::vec4 vertices[8];
      static const Face faces[12];
      static const Edge edges[12];
  };

}


#endif

