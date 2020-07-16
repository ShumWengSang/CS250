// DiffuseRenderTest.cpp
// -- test of diffuse rendering class
// cs250 5/20
//
// Windows:
//   cl /EHsc DiffuseRenderTest.cpp Affine.lib SphereMesh.cpp
//      DiffuseRender.cpp opengl32.lib glew32.lib
//      sdl2.lib sdl2main.lib /link /subsystem:console
//
// Linux:
//   g++ -std=c++11 DiffuseRenderTest.cpp Affine.cpp
//       SphereMesh.cpp DiffuseRender.cpp -lGL -lGLEW -lSDL2


#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "SphereMesh.h"
#include "DiffuseRender.h"
using namespace std;


const int COPY_COUNT = 10000;


const glm::vec4 O = cs250::point(0,0,0),
                EX = cs250::vector(1,0,0),
                EY = cs250::vector(0,1,0),
                EZ = cs250::vector(0,0,1),
                PURPLE(1,0,1,1),
                LT_GRAY(0.95f,0.95f,0.95f,1);


float frand(float a=0, float b=1) {
  return a + (b-a)*float(rand())/float(RAND_MAX);
}


class Client {
  public:
    Client(SDL_Window *w);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    SDL_Window *window;
    cs250::DiffuseRender render;
    float sph_rot_rate;
    glm::vec4 sph_rot_axis,
              sph_center;
    bool culling,
         multiple;
    vector<glm::vec4> translations;
    double time;
};


Client::Client(SDL_Window *w)
    : window(w) {
  srand(unsigned(std::time(nullptr)));

  sph_rot_rate = 360.0f/10.0f;
  sph_rot_axis = EY;
  sph_center = cs250::point(0,0,-2);

  render.loadMesh(cs250::SphereMesh(10));
  render.setColor(PURPLE);
  glm::mat4 V = cs250::translate(-EZ);
  render.setView(V);
  glm::mat4 P = { {1.2f,0,0,0},
                  {0,1.2f,0,0},
                  {0,0,-1,-1},
                  {0,0,-0.2f,0} };
  render.setPerspective(P);

  culling = false;
  multiple = false;
  for (int i=0; i < COPY_COUNT; ++i)
    translations.push_back(
      cs250::vector(frand(-100,100),frand(-100,100),frand(-100,100)));
  translations.push_back(EZ+O-sph_center);

  time = 0;
}


Client::~Client(void) {
  render.unloadMesh();
}


void Client::draw(double dt) {
  cs250::DiffuseRender::clear(LT_GRAY);

  // draw rotating sphere
  glm::mat4 sph_model = cs250::translate(sph_center-O)
                        * cs250::rotate(sph_rot_rate*time,sph_rot_axis)
                        * cs250::scale(0.5f,1.75f,1);
  render.setModel(sph_model);
  render.draw();

  // draw copies?
  if (multiple)
    for (unsigned i=0; i < translations.size(); ++i) {
      render.setModel(cs250::translate(translations[i]) * sph_model);
      render.draw();
    }

  time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE)
    multiple = !multiple;
  else {
    culling = !culling;
    render.backfaceCull(culling);
  }
  string title = "DiffuseRender Test [instancing: ";
  title += (multiple ? "yes" : "no");
  title += ", culling: ";
  title += (culling ? "on]" : "off]");
  SDL_SetWindowTitle(window,title.c_str());
}


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);
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
  SDL_Window *window = SDL_CreateWindow("DiffuseRender Test",SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,500,500,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
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
  try {
    bool done = false;
    Client *client = new Client(window);
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
  }
  catch (exception &e) {
    cout << e.what() << endl;
  }

  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

