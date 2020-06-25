// FrustumMesh.h
// -- 3D mesh for a view frustum
// cs250 5/19

#ifndef CS250_FRUSTUMMESH_H
#define CS250_FRUSTUMMESH_H

#include "Mesh.h"

namespace cs250 {

  class FrustumMesh : public cs250::Mesh {
    public:
      FrustumMesh(float fov, float aspect, float N, float F);
      int vertexCount(void) const override              { return 9; }
      const glm::vec4* vertexArray(void) const override { return vertices; }
      glm::vec4 dimensions(void) const override         { return aabb_dimensions; }
      glm::vec4 center(void) const override             { return aabb_center; }
      int faceCount(void) const override                { return 12; }
      const Face* faceArray(void) const override        { return faces; }
      int edgeCount(void) const override                { return 16; }
      const Edge* edgeArray(void) const override        { return edges; }
    private:
      glm::vec4 vertices[9];
      glm::vec4 aabb_center,
                aabb_dimensions;
      static const Edge edges[16];
      static const Face faces[12];
  };

}


#endif

