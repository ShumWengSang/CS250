// RenderDemo.cpp
// -- combines wire and solid rendering with an interactive camera.
// cs250 6/19
//
// Windows:
//   cl /EHsc RenderDemo.cpp Affine.lib Camera.lib FrustumMesh.lib
//      WireRender.lib SphereMesh.cpp CameraTransform.cpp
//      DiffuseRender.cpp opengl32.lib glew32.lib sdl2.lib
//      sdl2main.lib /link /subsystem:console
//
// Linux:
//   g++ -std=c++11 RenderDemo.cpp Affine.cpp Camera.cpp
//       FrustumMesh.cpp SphereMesh.cpp CameraTransform.cpp
//       DiffuseRender.cpp WireRender.cpp -lGL -lGLEW -lSDL2


#include <iostream>
#include <cmath>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "Camera.h"
#include "SphereMesh.h"
#include "FrustumMesh.h"
#include "WireRender.h"
#include "DiffuseRender.h"
using namespace std;


const glm::vec4 O = cs250::point(0,0,0),
                EX = cs250::vector(1,0,0),
                EY = cs250::vector(0,1,0),
                EZ = cs250::vector(0,0,1),
                PURPLE(1,0,1,1),
                WHITE(1,1,1,1),
                BLACK(0,0,0,1),
                RED(1,0,0,1),
                GREEN(0,1,0,1);


class Client {
  public:
    Client(SDL_Window*);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void mousedrag(int dx, int dy, bool left_button);
    void resize(int W, int H);
  private:
    SDL_Window *window;
    cs250::DiffuseRender solid_render;
    cs250::WireRender wire_render;
    cs250::Camera cam1,
                  cam2;
    glm::mat4 floor_model[9],
              thing_model[3];
    cs250::FrustumMesh frustum1,
                       frustum2;
    float thing_rot_rate;
    glm::vec4 thing_rot_axis,
              thing_center;
    bool use_cam1,
         bf_cull;
};


Client::Client(SDL_Window *w)
    : window(w),
      frustum1(80,1,0.1f,1),  // fixed camera frustum
      frustum2(80,1,0.1f,1) { // initial moving camera frustum

  // solid models are all spheres
  solid_render.loadMesh(cs250::SphereMesh(30));

  // rotating thing (3 ellipsoids)
  thing_rot_rate = 360.0f/10.0f;
  thing_rot_axis = EY;
  thing_center = O - 2.0f*EZ;
  thing_model[0] = cs250::translate(thing_center-O)
                   * cs250::scale(0.2f,1,1);
  thing_model[1] = cs250::translate(thing_center-O)
                   * cs250::scale(1,0.2f,1);
  thing_model[2] = cs250::translate(thing_center-O)
                   * cs250::scale(1,1,0.2f);

  // ellipsoid floor
  for (int i=0; i < 9; ++i)
    floor_model[i] = cs250::translate(cs250::vector((i%3)-1,-1.1f,-(i/3)-1))
                     * cs250::scale(0.375f,0.05f,0.375f);

  bf_cull = false;
  use_cam1 = true;
  // fixed camera
  cam1 = cs250::Camera(O+EZ,-EZ,EY,80,1,0.1f,10);
  // moving camera
  cam2 = cs250::Camera(O+2.0f*EX-2.0f*EZ,-EX,EY,80,1,0.1f,10);
}


Client::~Client(void) {
  solid_render.unloadMesh();
}


void Client::draw(double dt) {
  cs250::DiffuseRender::clear(WHITE);

  cs250::Camera &cam = use_cam1 ? cam1 : cam2;
  solid_render.setView(cs250::view(cam));
  solid_render.setPerspective(cs250::perspective(cam));
  wire_render.setView(cs250::view(cam));
  wire_render.setPerspective(cs250::perspective(cam));

  // draw floor
  solid_render.setColor(PURPLE);
  for (int i=0; i < 9; ++i) {
    solid_render.setModel(floor_model[i]);
    solid_render.draw();
  }

  // update & draw rotating thing
  glm::mat4 R = cs250::translate(thing_center-O)
                * cs250::rotate(thing_rot_rate*dt,thing_rot_axis)
                * cs250::translate(O-thing_center);
  solid_render.setColor(GREEN);
  for (int i=0; i < 3; ++i) {
    thing_model[i] = R * thing_model[i];
    solid_render.setModel(thing_model[i]);
    solid_render.draw();
  }

  if (use_cam1) {
    // draw moving camera frustum
    wire_render.setColor(BLACK);
    wire_render.loadMesh(frustum2);
    wire_render.setModel(cs250::model(cam2));
    wire_render.draw();
    wire_render.unloadMesh();
  }
  else {
    // draw fixed camera frustum
    wire_render.setColor(RED);
    wire_render.loadMesh(frustum1);
    wire_render.setModel(cs250::model(cam1));
    wire_render.draw();
    wire_render.unloadMesh();
  }

}


void Client::keypress(SDL_Keycode kc) {
  const float angle_increment = 1.0f,
              dist_increment = 0.1f,
              zoom_increment = 0.95f;
  bool dirty = false;
  switch (kc) {
    case SDLK_w:
      cam2.pitch(angle_increment);
      break;
    case SDLK_x:
      cam2.pitch(-angle_increment);
      break;
    case SDLK_a:
      cam2.yaw(angle_increment);
      break;
    case SDLK_d:
      cam2.yaw(-angle_increment);
      break;
    case SDLK_LEFT:
      cam2.roll(angle_increment);
      break;
    case SDLK_RIGHT:
      cam2.roll(-angle_increment);
      break;
    case SDLK_UP:
      cam2.forward(dist_increment);
      break;
    case SDLK_DOWN:
      cam2.forward(-dist_increment);
      break;
    case SDLK_SPACE:
      use_cam1 = !use_cam1;
      break;
    case SDLK_1:
      cam2.zoom(zoom_increment);
      dirty = true;
      break;
    case SDLK_2:
      cam2.zoom(1.0f/zoom_increment);
      dirty = true;
      break;
    case SDLK_c:
      bf_cull = !bf_cull;
      solid_render.backfaceCull(bf_cull);
      if (bf_cull)
        SDL_SetWindowTitle(window,"DiffuseRender Test [culling on]");
      else
        SDL_SetWindowTitle(window,"DiffuseRender Test [culling off]");
  }
  // reset moving camera frustum ?
  if (dirty) {
    float aspect = cam2.viewport().x/cam2.viewport().y,
          fov = 2*glm::degrees(atan(0.5f*cam2.viewport().x/cam2.viewport().z));
    frustum2 = cs250::FrustumMesh(fov,aspect,0.1f,1);
  }
}


void Client::mousedrag(int dx, int dy, bool left_button) {
  const float ROT_UNIT = 0.5f;
  if (left_button) {
    cam2.yaw(-dx*ROT_UNIT);
    cam2.pitch(-dy*ROT_UNIT);
  }
}


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);

  // change aspect ratio of both cameras
  float aspect = float(W)/float(H),
        fov = 2*glm::degrees(atan(0.5f*cam1.viewport().x/cam1.viewport().z));
  cam1 = cs250::Camera(cam1.eye(),-cam1.back(),EY,80,aspect,0.1f,10);
  frustum1 = cs250::FrustumMesh(fov,aspect,0.1f,1);

  fov = 2*glm::degrees(atan(0.5f*cam2.viewport().x/cam2.viewport().z));
  cam2 = cs250::Camera(cam2.eye(),-cam2.back(),EY,80,aspect,0.1f,10);
  frustum2 = cs250::FrustumMesh(fov,aspect,0.1f,1);
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

