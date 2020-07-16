// DiffuseRender.h
// -- engine for rendering solid meshes using forward diffuse shading
// cs250 5/20

#ifndef CS250_DIFFUSERENDER_H
#define CS250_DIFFUSERENDER_H


#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "Mesh.h"


namespace cs250 {

  class DiffuseRender {
    public:
      DiffuseRender(void);
      ~DiffuseRender(void);
      static void backfaceCull(bool yes=true);
      static void clear(const glm::vec4 &color);
      void loadMesh(const NormalMesh &m);
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
            unormal_matrix,
            ucolor;
      GLuint vao,
             vertex_buffer,
             normal_buffer,
             face_buffer;
      int face_count;
  };

}


#endif

