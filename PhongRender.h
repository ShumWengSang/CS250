// PhongRender.h
// -- engine for rendering solid meshes using Phong shading
//    with multiple meshes and light sources
// cs250 10/19

#ifndef CS250_PHONGRENDER_H
#define CS250_PHONGRENDER_H


#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Camera.h"
#include "Affine.h"


namespace cs250 {

  class PhongRender {
    public:
      PhongRender(void);
      ~PhongRender(void);
      static void backfaceCull(bool yes=true);
      static void clear(const glm::vec4 &color);
      int loadMesh(const cs250::NormalMesh &m);
      void unloadMesh(int mi);
      void setModel(const glm::mat4 &M);
      void setCamera(const cs250::Camera &cam);
      void setMaterial(const glm::vec3 &diff_coef,
                       const glm::vec3 &spec_coef,
                       float spec_exp);
      void setLight(int li, const glm::vec4 &position,
                            const glm::vec3 &color);
      void setAmbient(const glm::vec3 &color);
      void draw(int mi);
    private:
      GLint program,
            upersp_matrix,
            uview_matrix,
            umodel_matrix,
            unormal_matrix,
            ueye_position,
            udiffuse_coefficient,
            uspecular_coefficient,
            uspecular_exponent,
            uambient_color,
            ulight_position,
            ulight_color,
            ulight_use;
      struct MeshData {
        enum { VERT=0, NORM=1, FACE=2 };
        GLuint vertex_array_buffer,
               buffer_objects[3];
        int face_count;
      };
      std::vector<MeshData> mesh_data;
  };

}


#endif

