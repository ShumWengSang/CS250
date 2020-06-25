// WireRender.h
// -- engine for rendering meshes as wire frames
// cs250 1/20

#ifndef CS250_WIRERENDER_H
#define CS250_WIRERENDER_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "Mesh.h"

namespace cs250 {

  class WireRender {
    public:
      WireRender(void);
      ~WireRender(void);
      static void clear(const glm::vec4 &color);
      void loadMesh(const Mesh &m);
      void unloadMesh(void);
      void setModel(const glm::mat4 &M);
      void setView(const glm::mat4 &V);
      void setPerspective(const glm::mat4 &P);
      void setColor(const glm::vec4 &color);
      void draw(void);
    private:
      GLint program,
            upersp_matrix,
            uview_matrix,
            umodel_matrix,
            ucolor;
      GLuint vao,
             vertex_buffer,
             edge_buffer;
      int edge_count;
  };

}


#endif

