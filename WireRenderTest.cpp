// WireRenderTest.cpp
// -- test of wire rendering class
// cs250 5/19
//
// Windows:
//   cl /EHsc WireRenderTest.cpp Affine.lib SphereMesh.cpp
//      WireRender.cpp opengl32.lib glew32.lib sdl2.lib sdl2main.lib
//      /link /subsystem:console
//
// Linux:
//   g++ -std=c++11 WireRenderTest.cpp Affine.cpp
//       SphereMesh.cpp WireRender.cpp -lGL -lGLEW -lSDL2

#include <iostream>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "SphereMesh.h"
#include "WireRender.h"
using namespace std;


const glm::vec4 O = cs250::point(0,0,0),
                EX = cs250::vector(1,0,0),
                EY = cs250::vector(0,1,0),
                EZ = cs250::vector(0,0,1),
                PURPLE(1,0,1,1),
                LT_GRAY(0.95f,0.95f,0.95f,1);


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    cs250::WireRender render;
    float sph_rot_rate;
    glm::vec4 sph_rot_axis,
              sph_center;
    glm::mat4 sph_model;
};


Client::Client(void) {
  // set renderer stuff
  render.loadMesh(cs250::SphereMesh(10));
  render.setColor(PURPLE);
  glm::mat4 V = cs250::translate(-EZ);
  render.setView(V);
  glm::mat4 P = { {1.2f,0,0,0},
                  {0,1.2f,0,0},
                  {0,0,0,-1},
                  {0,0,0,0} };
  render.setPerspective(P);

  // initialize sphere modeling transform
  sph_rot_rate = 360.0f/10.0f;
  sph_rot_axis = EY;
  sph_center = cs250::point(0,0,-2);
  sph_model = cs250::translate(sph_center-O)
              * cs250::rotate(-90,EX)
              * cs250::scale(1.75f);
}


Client::~Client(void) {
  render.unloadMesh();
}


void Client::draw(double dt) {
  // clear frame buffer
  render.clear(LT_GRAY);

  // rotating sphere modeling transformation
  sph_model = cs250::translate(sph_center-O)
              * cs250::rotate(sph_rot_rate*dt,sph_rot_axis)
              * cs250::translate(O-sph_center)
              * sph_model;

  // draw sphere
  render.setModel(sph_model);
  render.draw();
}


void Client::keypress(SDL_Keycode kc) { }


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);

  // reset the camera and shader program parameters
  float aspect = float(W)/float(H);
  glm::mat4 P = { {1.2f,0,0,0},
                  {0,1.2f*aspect,0,0},
                  {0,0,-1,-1},
                  {0,0,-0.2f,0} };
  render.setPerspective(P);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  // SDL: initialize and create a window

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("OpenGL Demo",SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,500,500,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE); 
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GLEW: get function bindings (if possible)
  GLenum value = glewInit();
  if (value != GLEW_OK) {
    cout << glewGetErrorString(value) << endl;
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return -1;
  }

  // animation loop
  bool done = false;
  Client *client = new Client();
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
      }
    }
    Uint32 ticks = SDL_GetTicks();
    double dt = 0.001*(ticks - ticks_last);
    ticks_last = ticks;
    client->draw(dt);
    SDL_GL_SwapWindow(window);
  }

  // clean up
  delete client;
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

