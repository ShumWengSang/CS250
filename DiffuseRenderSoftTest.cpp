// DiffuseRenderSoftTest.cpp
// -- test of software rendering class
// cs250 11/19
//
// Windows:
//   cl /EHsc DiffuseRenderSoftTestTest.cpp Affine.lib SphereMesh.cpp
//      Camera.lib CameraTransform.lib DiffuseRenderSoft.cpp
//      opengl32.lib glew32.lib sdl2.lib sdl2main.lib
//      /link /subsystem:console


#include <iostream>
#include <vector>
#include <exception>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "SphereMesh.h"
#include "DiffuseRenderSoft.h"
using namespace std;


const glm::vec4 O = cs250::point(0,0,0),
                EX = cs250::vector(1,0,0),
                EY = cs250::vector(0,1,0),
                EZ = cs250::vector(0,0,1);
const glm::vec3 BLACK(0,0,0),
                WHITE(1,1,1),
                REDISH(1,0.6f,0.6),
                GREENISH(0.6f,1,0.6f),
                BLUEISH(0.6f,0.6f,1);


class Client {
  public:
    Client(SDL_Window *w, int W, int H);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void mousedrag(int dx, int dy, bool left_button);
  private:
    SDL_Window *window;
    cs250::Raster *raster;
    cs250::Raster::byte *frame_buffer;
    float *zbuffer;
    cs250::DiffuseRenderSoft *render;
    bool backface_cull;

    cs250::SphereMesh sphere1,
                      sphere2;

    cs250::Camera camera;
    struct Object {
      cs250::NormalMesh *mesh;
      glm::vec3 diffuse_coef;
      glm::mat4 model;
      Object(cs250::NormalMesh *ptr, const glm::vec3 &c)
          : mesh(ptr), diffuse_coef(c) {}
    };
    vector<Object> objects;
};


Client::Client(SDL_Window *w,int W, int H)
     : window(w),
       sphere1(15),
       sphere2(10) {
  frame_buffer = new cs250::Raster::byte[3*W*H];
  zbuffer = new float[W*H];
  raster = new cs250::Raster(frame_buffer,zbuffer,W,H,3*W);
  render = new cs250::DiffuseRenderSoft(*raster);

  // objects
  objects.emplace_back(&sphere1,REDISH);   // ellipsoid thing #1
  objects[0].model = cs250::scale(1,4,4);
  objects.emplace_back(&sphere1,GREENISH); // ellipsoid thing #2
  objects[1].model = cs250::scale(4,1,4);
  objects.emplace_back(&sphere1,BLUEISH);  // ellipsoid thing #3
  objects[2].model = cs250::scale(4,4,1);

  const int COUNT = 3;                     // floor tiles
  for (int j=-COUNT; j < COUNT; ++j)
    for (int i=-COUNT; i < COUNT; ++i)
      if ((i+j)%2 == 0) {
        objects.emplace_back(&sphere2,0.8f*WHITE);
        glm::vec4 C = O + 5.0f*i*EX
                        - 4.5f*EY
                        + 5.0f*j*EZ;
        objects[objects.size()-1].model = cs250::translate(C-O)
                                          * cs250::scale(3,0.5f,3);
      }

  float aspect = float(W)/float(H);
  camera = cs250::Camera(O+10.f*EZ,-EZ,EY,80,aspect,0.1f,100);
  render->setLight(EX+EZ+EY,WHITE);
  render->setAmbient(0.2f*WHITE);
  backface_cull = false;
}


Client::~Client(void) {
  delete render;
  delete raster;
  delete[] zbuffer;
  delete[] frame_buffer;
}


void Client::draw(double dt) {
  render->clear(BLACK);

  render->setCamera(camera);

  // rotating ellipsoid thing: modeling transform update
  const float RATE = 360/5.0f;
  for (int i=0; i < 3; ++i)
    objects[i].model = cs250::rotate(RATE*dt,EY) * objects[i].model;

  // draw meshes
  for (unsigned i=0; i < objects.size(); ++i) {
    render->loadMesh(*objects[i].mesh);
    render->setModel(objects[i].model);
    render->setDiffuseCoefficient(objects[i].diffuse_coef);
    render->draw();
    render->unloadMesh();
  }

  glDrawPixels(raster->width(),raster->height(),
               GL_RGB,GL_UNSIGNED_BYTE,frame_buffer);
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
    case SDLK_SPACE:
      backface_cull = !backface_cull;
      render->backfaceCull(backface_cull);
      SDL_SetWindowTitle(window,backface_cull ? "DiffuseRenderSoft Test [BF cull on]"
                                              : "DiffuseRenderSoft Test [BF cull off]");
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


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  int win_x = (argc == 3) ? atoi(argv[1]) : 500,
      win_y = (argc == 3) ? atoi(argv[2]) : 500;

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("DiffuseRenderSoft Test [BF cull off]",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        win_x,win_y,
                                        SDL_WINDOW_OPENGL);
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
    Client *client = new Client(window,win_x,win_y);
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


