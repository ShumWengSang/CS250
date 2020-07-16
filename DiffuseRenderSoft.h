// DiffuseRenderSoft.h
// -- engine for rendering solid meshes using diffuse shading,
//    software only version
// cs250 5/19

#ifndef CS250_DIFFUSERENDERSOFT_H
#define CS250_DIFFUSERENDERSOFT_H

#include <vector>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Camera.h"
#include "Raster.h"


namespace cs250 {

  class DiffuseRenderSoft {
    public:
      DiffuseRenderSoft(cs250::Raster &r);
      void backfaceCull(bool yes=true)        { backface_cull = yes; }
      void clear(const glm::vec3 &color);
      void loadMesh(const NormalMesh &m)      { mesh = &m; }
      void unloadMesh(void)                   { mesh = nullptr; }
      void setModel(const glm::mat4 &M);
      void setCamera(const cs250::Camera &cam);
      void setDiffuseCoefficient(const glm::vec3 &k);
      void setLight(const glm::vec4 &L, const glm::vec3 &c);
      void setAmbient(const glm::vec3 &c);
      void draw(void);
    private:
      Raster &raster;
      const NormalMesh *mesh;
      glm::mat4 model_matrix,
                normal_matrix,
                view_matrix,
                perspective_matrix,
                device_matrix;
      glm::vec4 light_direction;
      glm::vec3 diffuse_coefficient,
                light_color,
                ambient_color;
      bool backface_cull;

      std::vector<glm::vec4> clip_verts,
                             device_verts,
                             world_normals;
  };

}


#endif

