// Roland Shum
// Assignment 9

#include "DiffuseRenderSoft.h"
#include "Affine.h"
#include <cmath>
#include <algorithm>

namespace cs250
{
    DiffuseRenderSoft::DiffuseRenderSoft(cs250::Raster &r) : raster(r)
    {
        device_matrix = cs250::translate(cs250::vector(-0.5f, -0.5f, 0))
        * cs250::scale(r.width() * 0.5f, r.height() * 0.5f, 1.0f) 
        * cs250::translate(cs250::vector(1,1,0));
    }
    
    void DiffuseRenderSoft::clear(const glm::vec3 &color)
    {
        // 255 blah blah blah
        glm::vec3 iclr = 255.0f * color;
        raster.setColor(iclr.x, iclr.y, iclr.z);
        for(int j = 0; j < raster.height(); ++j)
        {
            raster.gotoPoint(0, j);
            for(int i = 0; i < raster.width(); ++i)
            {
                raster.writePixel();
                raster.writeZ(1.0f);
                raster.incrementX();
            }
        }
    }
    
    void DiffuseRenderSoft::setModel(const glm::mat4 &M)
    {
        model_matrix = M;
        normal_matrix = glm::transpose(glm::inverse(glm::mat3(M)));
    }
    
    void DiffuseRenderSoft::setCamera(const cs250::Camera &cam)
    {
        view_matrix = cs250::view(cam);
        perspective_matrix = cs250::perspective(cam);
    }
    
    void DiffuseRenderSoft::setDiffuseCoefficient(const glm::vec3 &k)
    {
        diffuse_coefficient = k;
    }
    
    void DiffuseRenderSoft::setLight(const glm::vec4 &L, const glm::vec3 &c)
    {
        light_direction = glm::normalize(L);
        light_color = c;
    }
    
    void DiffuseRenderSoft::setAmbient(const glm::vec3 &c)
    {
        ambient_color = c ;
    }
    
    void DiffuseRenderSoft::draw(void)
    {
        clip_verts.clear();
        device_verts.clear();
        world_normals.clear();
        glm::mat4x4 mat = perspective_matrix * view_matrix * model_matrix;
        for(int i = 0; i < mesh->vertexCount(); ++i)
        {
            const glm::vec4 &position = mesh->vertexArray()[i];
            const glm::vec4 &normal = mesh->normalArray()[i];
            glm::vec4 gl_Position = mat * position;
            glm::vec4 Pdev = device_matrix * gl_Position;
            clip_verts.push_back(Pdev);
            device_verts.push_back(Pdev / Pdev.w);
            
            world_normals.push_back(normal_matrix * normal);
        }
        
        // Process triangles
        for(int n = 0; n < mesh->faceCount(); ++n)
        {
            const cs250::Mesh::Face &f = mesh->faceArray()[n];
            const glm::vec4 &PClip = clip_verts[f.index1],
                            &QClip = clip_verts[f.index2],
                            &RClip = clip_verts[f.index3],
                            &Pdev = device_verts[f.index1],
                            &Qdev = device_verts[f.index2],
                            &Rdev = device_verts[f.index3],
                            &mP = world_normals[f.index1],
                            &mQ = world_normals[f.index2],
                            &mR = world_normals[f.index3];
                            
            // rejection test
            if(PClip.w <= 0 || QClip.w <= 0 || RClip.w <= 0)
                continue;
            
            int xmin = std::ceil(std::min(Pdev.x, std::min(Qdev.x, Rdev.x)));
            int ymin = std::ceil(std::min(Pdev.y, std::min(Qdev.y, Rdev.y)));
            int xmax = std::floor(std::max(Pdev.x, std::max(Qdev.x, Rdev.x)));
            int ymax = std::floor(std::max(Pdev.y, std::max(Qdev.y, Rdev.y)));
            
            
            // Clip to framebuffer
            xmin = std::max(xmin, 0);
            ymin = std::max(ymin, 0);
            xmax = std::min(xmax, raster.width() - 1);
            ymax = std::min(ymax, raster.height() - 1);
            

            
            // Find barycentric coordinates planar
            glm::vec4 Pu = cs250::point(Pdev.x, Pdev.y, 0);
            glm::vec4 Qu = cs250::point(Qdev.x, Qdev.y, 0);
            glm::vec4 Ru = cs250::point(Rdev.x, Rdev.y, 0);
            glm::vec4 u = cs250::cross(Qu - Pu, Ru - Pu);
            float du = glm::dot(u, Pu); 
            glm::vec3 u_fn(-u.x/ u.z, -u.y / u.z, du / u.z);
            
            glm::vec4 Pv = cs250::point(Pdev.x, Pdev.y, 0);
            glm::vec4 Qv = cs250::point(Qdev.x, Qdev.y, 0);
            glm::vec4 Rv = cs250::point(Rdev.x, Rdev.y, 0);
            glm::vec4 v = cs250::cross(Qv - Pv, Rv - Pv);
            float dv = glm::dot(v, Pv); 
            glm::vec3 v_fn(-v.x/ v.z, -v.y / v.z, dv / v.z);
            
            glm::mat2x2 inverseMatrix(1);
            inverseMatrix[0] = Qu - Pu;
            inverseMatrix[1] = Ru - Pu;
            inverseMatrix = glm::inverse(inverseMatrix);

            
            raster.gotoPoint(xmin,ymin);
            for(int i = ymin; i <= ymax; ++i)
            {
                //raster.incrementY();
                for(int j = xmin; j <= xmax; ++j)
                {
                    // Compute barycentric coordinates
                    glm::vec2 I(j, i);
                    I = I - glm::vec2(Pu);
                    // Check if within barycentric coordinates
                    //float Iu = glm::dot(u_fn, I);
                    //float Iv = glm::dot(v_fn, I);   
                    //float Il = 1 - Iu - Iv;
                    glm::vec2 resultantBarycentric = inverseMatrix * I;
                    float Iu = resultantBarycentric[0];
                    float Iv = resultantBarycentric[1];
                    float Il = 1 - Iu - Iv;
                    
                    raster.gotoPoint(j, i);
                    // Test if pixel is in triangle
                    if(Il >= 0 && Iu >= 0 && Iv == 0)
                    {
                        float z = Il * Pdev.z + Iv * Qdev.z + Iu * Rdev.z;
                        if(z < raster.getZ())
                        {
                            // Interpolate world normal, perspective divide
                            glm::vec3 warpedBary(Il / PClip.w, Iu / QClip.w,Iv / RClip.w);
                            float D = warpedBary.x + warpedBary.y + warpedBary.z;
                            warpedBary /= D;
                            
                            glm::vec4 m = warpedBary.x * mP + warpedBary.y * mQ + warpedBary.z * mR;
                            m = glm::normalize(m);
                            glm::vec4 pos = warpedBary.x * j + warpedBary.y * i + warpedBary.z * mR;
                            
                            float mL = glm::dot(light_direction, m);
                            glm::vec3 color = (ambient_color) + std::max(0.0f, mL) * (diffuse_coefficient) * (light_color);
                            color *= 255.0f;
                            //if(glm::dot(m,m) < 1e-3f)
                              ////  color = glm::vec3(0,0,0);
                            
                            raster.setColor(color.x, color.y, color.z);
                            raster.writeZ(z);
                            raster.writePixel();
                        }
                    }
                    raster.incrementX();
                }
            }
        }
    }
}