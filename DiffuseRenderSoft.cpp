// Roland Shum
// Assignment 9

#include "DiffuseRenderSoft.h"
#include "Affine.h"
#include <cmath>
#include <algorithm>

namespace cs250
{
    //will represent a vertex transformed into NDC containing the following fields
    class VertexNDC
    {
    public:
        glm::vec4 Position; // in NDC coordinates
        glm::vec3 Normal; // in world coordinates
        glm::vec3 World; // position in world coordinates

        static VertexNDC Interpolate(float t, VertexNDC a, VertexNDC b);
    };

    class VertexPIX
    {
    public:
        glm::vec3 Position; // x and y in pixel coordinates, z left in NDC coordinates
        glm::vec3 Normal; // in world coordinates
        glm::vec3 World; // position in world coordinates
        float w; // Homogeneous coordinate needed for perspective correct interpolation.

        static VertexPIX Interpolate(float t, VertexPIX a, VertexPIX b);
    };

    VertexPIX Project(VertexNDC);

    VertexNDC VertexNDC::Interpolate(float t, VertexNDC a, VertexNDC b)
    {
        VertexNDC result;

        result.Position = a.Position + t * (b.Position - a.Position);
        result.Normal = a.Normal + t * (b.Normal - a.Normal);
        result.World = a.World + t * (b.World - a.World);
        return result;
    }

    VertexPIX VertexPIX::Interpolate(float t, VertexPIX a, VertexPIX b)
    {
        VertexPIX result;
        result.Position = a.Position + t * (b.Position - a.Position);
        const float s = t * a.w / (t * a.w + (1 - t) * b.w); // the perspective correction term
        result.Normal = a.Normal + s * (b.Normal - a.Normal);
        result.World = a.World + s * (b.World - a.World);
        result.w = a.w + s * (b.w - a.w);
        return result;
    }

    VertexPIX Project(VertexNDC a)
    {
        VertexPIX res;

        res.Position = glm::vec3(a.Position) / a.Position.w;
        res.w = a.Position.w;

        res.Normal = a.Normal;
        res.World = res.World;

        return res;
    }

    std::vector<VertexNDC> PolyClip(std::vector<VertexNDC> const& input, glm::vec4 plane)
    {
        std::vector<VertexNDC> output;
        output.reserve(8);

        if (input.empty())
            return output;

        VertexNDC const* S = &input.back();

        for (int i = 0; i < input.size(); ++i)
        {
            VertexNDC const& T = input[i];

            float d0 = glm::dot(plane, (*S).Position);
            float d1 = glm::dot(plane, T.Position);

            if (d0 > 0 && d1 > 0)
            {
                output.emplace_back(T);
            }
            else if (d0 > 0)
            {
                float t = d0 / (d0 - d1);
                VertexNDC I = VertexNDC::Interpolate(t, *S, T);
                output.emplace_back(I);
            }
            else if (d1 > 0)
            {
                float t = d0 / (d0 - d1);
                VertexNDC I = VertexNDC::Interpolate(t, *S, T);
                output.emplace_back(I);
                output.emplace_back(T);
            }
            S = &T;
        }
        return output;
    }

    void DepthTestandColor(VertexPIX const& coord, DiffuseRenderSoft& render)
    {

    }

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

    auto Interpolate_y(float y, VertexPIX const& a, VertexPIX const& b)
    {
        float t = (y - a.Position.y) / (b.Position.y - a.Position.y);
        return VertexPIX::Interpolate(t, a, b);
    }

    auto Interpolate_x(float x, VertexPIX const& a, VertexPIX const& b)
    {
        float t = (x - a.Position.x) / (b.Position.x - a.Position.x);
        return VertexPIX::Interpolate(t, a, b);
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
        int width = raster.width();
        int height = raster.height();

        // The viewing transformations
        glm::mat4 WorldInverse = glm::inverse(view_matrix);
        glm::mat3 NormalMatrix = glm::transpose(glm::mat3(glm::inverse(model_matrix)));

        // The arrays of vertices, normals, texture coordinates, ...
        glm::vec4 const * Pnt = mesh->vertexArray();
        glm::vec4 const* Nrm = mesh->normalArray();
        

        // The "tri" input to this procedure describes a single
        // triangle as three indices into the above arrays. Continue from
        // here with code to transform, clip, scan-convert, depth test,
        // and light this single triangle.

        // Asssuming triangles, the vectors for points are all size() == 3

        // FOR EACH TRIANGLE/FACE, do the rendering
        for (int k = 0; k < mesh->faceCount(); ++k)
        {
            cs250::Mesh::Face const& tri = mesh->faceArray()[k];
            // TRANSFORM
            std::vector<VertexNDC> transform;
            transform.reserve(3);

            for (int i = 0; i < 3; ++i)
            {
                // Lazy haha
                const int index = reinterpret_cast<const int*>(&tri)[i];
                glm::vec4 const& ptr = Pnt[index];
                VertexNDC temp = { perspective_matrix * view_matrix * model_matrix * ptr , NormalMatrix * Nrm[index], (NormalMatrix * ptr) };
                transform.emplace_back(temp);
            }

            // CLIP using cohen sutherland 
            std::vector<VertexNDC> a = PolyClip(transform, { 1,0,0,1 });
            std::vector<VertexNDC> b = PolyClip(a, { -1,0,0,1 });
            std::vector<VertexNDC> c = PolyClip(b, { 0,-1,0,1 });
            std::vector<VertexNDC> d = PolyClip(c, { 0,1,0,1 });
            std::vector<VertexNDC> e = PolyClip(d, { 0,0,1,1 });
            std::vector<VertexNDC> f = PolyClip(e, { 0,0,-1,1 });
            //Clip(transform);
            // TRANSFORM
            std::vector<VertexPIX> transformed;
            transformed.reserve(f.size());

            for (int i = 0; i < f.size(); ++i)
            {
                // Project by doing /w
                VertexPIX temp = { (glm::vec3)(f[i].Position) / f[i].Position.w , f[i].Normal, f[i].World, f[i].Position.w };
                temp.Position.x = width * (temp.Position.x + 1) / 2;
                temp.Position.y = height * (temp.Position.y + 1) / 2;
                transformed.emplace_back(temp);
            }

            // TRIANGULATE AND DO THINGS
            for (int i = 2; i < transformed.size(); ++i)
            {
                VertexPIX const& s0 = transformed.at(0);
                VertexPIX const& s1 = transformed.at(i - 1);
                VertexPIX const& s2 = transformed.at(i);

                // Reorder points
                auto cmp = [](VertexPIX a, VertexPIX b) { return a.Position.y < b.Position.y; };
                VertexPIX s[3] = { s0, s1, s2 };
                std::sort(s, s + 3, cmp);

                if (s[0].Position.y != s[1].Position.y)
                {
                    for (float y = std::ceil(s[0].Position.y); y <= std::ceil(s[1].Position.y - 1); ++y)
                    {
                        VertexPIX e0 = Interpolate_y(y, s[0], s[1]);
                        VertexPIX e1 = Interpolate_y(y, s[0], s[2]);

                        if (e0.Position.x > e1.Position.x)
                        {
                            std::swap(e0, e1);
                        }

                        for (float x = std::ceil(e0.Position.x); x <= std::ceil(e1.Position.x - 1); ++x)
                        {
                            VertexPIX ep = Interpolate_x(x, e0, e1);

                            // Compare depth
                            raster.gotoPoint(ep.Position.x, ep.Position.y);
                            if (ep.Position.z > raster.getZ())
                            {
                                // Pixel is discarded.
                                continue;
                            }
                            float mL = glm::dot(glm::vec3(light_direction), glm::normalize(ep.Normal));
                            glm::vec3 color = (ambient_color)+std::max(0.0f, mL) * (diffuse_coefficient) * (light_color);
                            color *= 255.0f;
                            //if(glm::dot(m,m) < 1e-3f)
                              ////  color = glm::vec3(0,0,0);

                            raster.setColor(color.x, color.y, color.z);
                            raster.writeZ(ep.Position.z);
                            raster.writePixel();
                        }
                    }
                }

                if (s[1].Position.y != s[2].Position.y)
                {
                    // @@ Check this with Prof
                    for (float y = std::ceil(s[1].Position.y); y <= std::ceil(s[2].Position.y - 1); ++y)
                    {
                        VertexPIX e0 = Interpolate_y(y, s[1], s[2]);
                        VertexPIX e1 = Interpolate_y(y, s[0], s[2]);

                        if (e0.Position.x > e1.Position.x)
                        {
                            std::swap(e0, e1);
                        }

                        for (float x = std::ceil(e0.Position.x); x <= std::ceil(e1.Position.x - 1); ++x)
                        {
                            VertexPIX ep = Interpolate_x(x, e0, e1);
                            raster.gotoPoint(ep.Position.x, ep.Position.y);
                            if (ep.Position.z > raster.getZ())
                            {
                                // Pixel is discarded.
                                continue;
                            }
                            float mL = glm::dot(glm::vec3(light_direction), glm::normalize(ep.Normal));
                            glm::vec3 color = (ambient_color)+std::max(0.0f, mL) * (diffuse_coefficient) * (light_color);
                            color *= 255.0f;
                            //if(glm::dot(m,m) < 1e-3f)
                              ////  color = glm::vec3(0,0,0);

                            raster.setColor(color.x, color.y, color.z);
                            raster.writeZ(ep.Position.z);
                            raster.writePixel();
                        }
                    }
                }
            }
        }
    }
}