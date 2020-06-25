// PhongRenderTest.cpp
// -- test of Phong lighting renderer
// cs250 10/19
//
// Windows:
//   cl /EHsc PhongRenderTest.cpp Affine.lib SphereMesh.cpp
//      Camera.lib CameraTransform.lib PhongRender.cpp
//      opengl32.lib glew32.lib sdl2.lib sdl2main.lib
//      /link /subsystem:console
//
// Linux:
//   g++ -std=c++11 PhongRenderTest.cpp Affine.cpp Camera.cpp
//       SphereMesh.cpp CameraTransform.cpp PhongRender.cpp
//       -lGL -lGLEW -lSDL2


#include <iostream>
#include <vector>
#include <exception>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "SphereMesh.h"
#include "PhongRender.h"
using namespace std;


const glm::vec4 O = cs250::point(0,0,0),
                EX = cs250::vector(1,0,0),
                EY = cs250::vector(0,1,0),
                EZ = cs250::vector(0,0,1);
const glm::vec3 BLACK(0,0,0),
                WHITE(1,1,1),
                REDISH(1,0.6f,0.6f),
                GREENISH(0.6f,1,0.6f),
                BLUEISH(0.6f,0.6f,1);


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void mousedrag(int dx, int dy, bool left_button);
    void resize(int W, int H);
  private:
    cs250::PhongRender render;
    cs250::Camera camera;
    struct Object {
      int mesh_index;
      glm::vec3 diffuse_coef,
                specular_coef;
      float specular_exp;
      glm::mat4 model;
      Object(int mi, const glm::vec3 &dc, const glm::vec3 &sc, float se)
        : mesh_index(mi), diffuse_coef(dc), specular_coef(sc), specular_exp(se) {}
    };
    vector<Object> objects;
};


Client::Client(void) {
  // three sphere meshes
  render.loadMesh(cs250::SphereMesh(15)); // index 0
  render.loadMesh(cs250::SphereMesh(30)); // index 1
  render.loadMesh(cs250::SphereMesh(5));  // index 2

  // objects
  objects.emplace_back(0,0.5f*REDISH,0.5f*WHITE,40.0f);   // ellipsoid thing #1
  objects[0].model = cs250::scale(1,4,4);
  objects.emplace_back(0,0.5f*GREENISH,0.5f*WHITE,40.0f); // ellipsoid thing #2
  objects[1].model = cs250::scale(4,1,4);
  objects.emplace_back(0,0.5f*BLUEISH,0.5f*WHITE,40.0f);  // ellipsoid thing #3
  objects[2].model = cs250::scale(4,4,1);

  objects.emplace_back(1,0.5f*WHITE,0.4f*WHITE,20.0f);    // floor
  objects[3].model = cs250::translate(-4.1f*EY)
                     * cs250::scale(100,0.25f,100);

  objects.emplace_back(2,0.8f*REDISH,WHITE,100.0f);   // light #1
  objects[4].model = cs250::translate(5.0f*EX+5.0f*EY+5.0f*EZ) * cs250::scale(0.25f);
  objects.emplace_back(2,0.8f*GREENISH,WHITE,100.0f); // light #2
  objects[5].model = cs250::translate(-5.0f*EX+5.0f*EY+5.0f*EZ) * cs250::scale(0.25f);
  objects.emplace_back(2,0.8f*BLUEISH,WHITE,100.0f);  // light #3
  objects[6].model = cs250::translate(2.0f*EY+6.0f*EZ) * cs250::scale(0.25f);

  // three point light sources + ambient
  for (int i=4; i <= 6; ++i) {
    render.setLight(i-4,objects[i].model*O,objects[i].diffuse_coef);
    objects[i].diffuse_coef *= 2;  // unphysical, but ...
  }
  render.setAmbient(0.3f*WHITE); 

  camera = cs250::Camera(O+10.f*EZ,-EZ,EY,80,1,0.1f,100);
}


Client::~Client(void) { }


void Client::draw(double dt) {
  render.clear(glm::vec4(BLACK,1));

  render.setCamera(camera);

  // rotating ellipsoid thing: modeling transform update
  const float RATE = 360/5.0f;
  for (int i=0; i < 3; ++i)
    objects[i].model = cs250::rotate(RATE*dt,EY) * objects[i].model;

  // draw meshes
  for (int i=0; i < 7; ++i) {
    render.setModel(objects[i].model);
    render.setMaterial(objects[i].diffuse_coef,
                       objects[i].specular_coef,
                       objects[i].specular_exp);
    render.draw(objects[i].mesh_index);
  }
}


void Client::keypress(SDL_Keycode kc) {
  const float ANG_INC = 1.0f,
              FWD_INC = 0.1f;
  switch (kc) {
    case SDLK_a:
      camera.yaw(ANG_INC);
      break;
    case SDLK_d:
      camera.yaw(-ANG_INC);
      break;
    case SDLK_w:
      camera.pitch(ANG_INC);
      break;
    case SDLK_x:
      camera.pitch(-ANG_INC);
      break;
    case SDLK_s:
      camera.roll(ANG_INC);
      break;
    case SDLK_q:
      camera.roll(-ANG_INC);
      break;
    case SDLK_UP:
      camera.forward(FWD_INC);
      break;
    case SDLK_DOWN:
      camera.forward(-FWD_INC);
      break;
  }
}


void Client::mousedrag(int dx, int dy, bool left_button) {
  const float ROT_UNIT = 1.0f;
  if (left_button) {
    glm::mat4 R = cs250::rotate(-dx*ROT_UNIT,EY)
                  * cs250::rotate(-dy*ROT_UNIT,camera.right());
    float aspect = camera.viewport().x/camera.viewport().y;
    camera = cs250::Camera(camera.eye(),-R*camera.back(),EY,80,aspect,0.1f,100);
  }
}


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);
  float aspect = float(W)/float(H);
  camera = cs250::Camera(camera.eye(),-camera.back(),EY,80,aspect,0.1f,100);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("PhongRender Test",SDL_WINDOWPOS_UNDEFINED,
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


