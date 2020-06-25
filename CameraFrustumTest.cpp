// CameraFrustumTest.cpp
// -- display the camera (uses Camera and FrustumMesh)
// cs250 5/20
//
// To compile/link using Visual Studio command prompt:
//     cl /EHsc CameraFrustumTest.cpp Affine.lib Camera.cpp
//        FrustumMesh.cpp opengl32.lib glew32.lib SDL2.lib
//        SDL2main.lib /link /subsystem:console
// Under Linux:
//     g++ -std=c++11 CameraFrustumTest.cpp Affine.lib Camera.cpp
//         FrustumMesh.cpp  -lGL -lGLEW -lSDL2

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "Camera.h"
#include "FrustumMesh.h"
using namespace std;


const glm::vec4 O = cs250::point(0,0,0),
                EX = cs250::vector(1,0,0),
                EY = cs250::vector(0,1,0),
                EZ = cs250::vector(0,0,1);

const glm::vec3 FRUSTUM_COLOR(1,0.75f,1),
                ARROW_COLOR(0.75f,1,0.75f);

class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void mousedrag(int dx, int dy, bool left_button);
    void resize(int W, int H);
  private:
    cs250::Camera camera;
    bool draw_solid,
         draw_orientation;
    glm::mat4 PersProj;
    GLint program,
          aposition,
          ucolor;
};


Client::Client(void) {
  // compile and link shaders
  const char *fragment_shader_text =
    "#version 130\n\
     uniform vec3 color;\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = vec4(color,1);\
     }";
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader,1,&fragment_shader_text,0);
  glCompileShader(fshader);

  const char *vertex_shader_text =
    "#version 130\n\
     in vec4 position;\
     void main(void) {\
       gl_Position = position;\
     }";
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader,1,&vertex_shader_text,0);
  glCompileShader(vshader);

  program = glCreateProgram();
  glAttachShader(program,fshader);
  glAttachShader(program,vshader);
  glLinkProgram(program);
  glDeleteShader(fshader);
  glDeleteShader(vshader);

  // shader parameter locations
  aposition = glGetAttribLocation(program,"position");
  ucolor = glGetUniformLocation(program,"color");

  glEnable(GL_DEPTH_TEST);

  // intialize camera
  camera = cs250::Camera(O-3.0f*EZ,-EZ,EY,72,1.5f,1,3);
  draw_solid = false;
  draw_orientation = false;

  // perspective projection
  PersProj[0] = glm::vec4(1.4f,0,0,0);
  PersProj[1] = glm::vec4(0,1.4f,0,0);
  PersProj[2] = glm::vec4(0,0,-1,-1);
  PersProj[3] = glm::vec4(0,0,-0.2f,0);
}


Client::~Client(void) {
  glUseProgram(0);
  glDeleteProgram(program);
}


void Client::draw(double dt) {
  // clear frame buffer and z-buffer
  glClearColor(0.95f,0.95f,0.95f,1);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  // frustum mesh
  const glm::vec4 &geom = camera.viewport();
  float aspect = geom.x/geom.y,
        fov = glm::degrees(2.0f*atan(0.5f*geom.x/geom.z));
  cs250::FrustumMesh frustum(fov,aspect,1,3);

  // modeling transformations for frustum and oriention arrows
  glm::mat4 U1 = PersProj
                * glm::mat4(camera.right(),
                            camera.up(),
                            camera.back(),
                            camera.eye()),
            U0 = cs250::translate(1.33f*EZ)
                 * cs250::scale(0.1f*geom.z/geom.x,0.1f*geom.z/geom.y,0.33f);
  vector<glm::mat4> model;
  model.push_back(U1);
  if (draw_orientation) {
    model.push_back(U1 * cs250::rotate(90,EY) * U0);
    model.push_back(U1 * cs250::rotate(-90,EX) * U0);
    model.push_back(U1 * U0);
  }

  // draw meshes
  glUseProgram(program);
  glEnableVertexAttribArray(aposition);
  vector<glm::vec4> ndc_verts(frustum.vertexCount());

  for (unsigned k=0; k < model.size(); ++k) {
    glm::vec3 base_color = (k == 0) ? FRUSTUM_COLOR : ARROW_COLOR;

    // compute mesh vertices in NDC (we should really do this
    //   on the GPU side)
    for (int i=0; i < frustum.vertexCount(); ++i)
      ndc_verts[i] = model[k] * frustum.vertexArray()[i];

    if (draw_solid) {
      for (int n=0; n < frustum.faceCount(); ++n) {
        const cs250::Mesh::Face &f = frustum.faceArray()[n];
        const glm::vec4 &P = ndc_verts[f.index1],
                        &Q = ndc_verts[f.index2],
                        &R = ndc_verts[f.index3];
        glm::vec4 m = glm::normalize(cs250::cross(Q-P,R-P));
        float shade = glm::max(m.z,0.0f);
        glm::vec3 color = shade * base_color;
        glUniform3fv(ucolor,1,&color[0]);

        GLuint vbo;
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glm::vec4 verts[3] = { P, Q, R };
        glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);
        glVertexAttribPointer(aposition,4,GL_FLOAT,false,0,0);
        glDrawArrays(GL_TRIANGLES,0,3);
        glDeleteBuffers(1,&vbo);
      }
    }

    else {
      glLineWidth(2.0f);
      glm::vec3 color = 0.5f*base_color;
      glUniform3fv(ucolor,1,&color[0]);
      for (int n=0; n < frustum.edgeCount(); ++n) {
        const cs250::Mesh::Edge &e = frustum.edgeArray()[n];
        const glm::vec4 &P = ndc_verts[e.index1],
                        &Q = ndc_verts[e.index2];
        GLuint vbo;
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glm::vec4 verts[2] = { P, Q };
        glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);
        glVertexAttribPointer(aposition,4,GL_FLOAT,false,0,0);
        glDrawArrays(GL_LINES,0,2);
        glDeleteBuffers(1,&vbo);
      }
    }
  }

}


void Client::keypress(SDL_Keycode kc) {
  const float angle_increment = 1.0f,
              dist_increment = 0.1f,
              zoom_increment = 0.95f;
  switch (kc) {
    case SDLK_SPACE:
      draw_solid = !draw_solid;
      break;
    case SDLK_w:
      camera.pitch(angle_increment);
      break;
    case SDLK_x:
      camera.pitch(-angle_increment);
      break;
    case SDLK_a:
      camera.yaw(angle_increment);
      break;
    case SDLK_d:
      camera.yaw(-angle_increment);
      break;
    case SDLK_LEFT:
      camera.roll(angle_increment);
      break;
    case SDLK_RIGHT:
      camera.roll(-angle_increment);
      break;
    case SDLK_UP:
      camera.forward(dist_increment);
      break;
    case SDLK_DOWN:
      camera.forward(-dist_increment);
      break;
    case SDLK_1:
      camera.zoom(zoom_increment);
      break;
    case SDLK_2:
      camera.zoom(1.0f/zoom_increment);
      break;
    case SDLK_o:
      draw_orientation = !draw_orientation;
  }
}


void Client::mousedrag(int dx, int dy, bool left_button) {
  const float ROT_UNIT = 0.5f;
  if (left_button) {
    camera.yaw(-dx*ROT_UNIT);
    camera.pitch(-dy*ROT_UNIT);
  }
}


void Client::resize(int W, int H) {
  int D = min(W,H);
  glViewport(0,0,D,D);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: Camera+Frustum Test";
  int width = 600,
      height = 600;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GLEW: get function bindings (if possible)
  glewInit();
  if (!GLEW_VERSION_2_0) {
    cout << "needs OpenGL version 3.0 or better" << endl;
    return -1;
  }

  // animation loop
  try {
    Client *client = new Client();
    bool done = false;
    Uint32 ticks_last = SDL_GetTicks();
    while (!done) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT:
            done = true;
            break;
          case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
              done = true;
            else
              client->keypress(event.key.keysym.sym); 
            break;
          case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
              client->resize(event.window.data1,event.window.data2);
            break;
          case SDL_MOUSEMOTION:
            if ((event.motion.state&SDL_BUTTON_LMASK) != 0
                || (event.motion.state&SDL_BUTTON_RMASK) != 0)
              client->mousedrag(event.motion.xrel,event.motion.yrel,
                                (event.motion.state&SDL_BUTTON_LMASK) != 0);
            break;
        }
      }
      Uint32 ticks = SDL_GetTicks();
      double dt = 0.001*(ticks - ticks_last);
      ticks_last = ticks;
      client->draw(dt);
      SDL_GL_SwapWindow(window);
    }
    delete client;
  }

  catch (exception &e) {
    cout << e.what() << endl;
  }

  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

