// CubeMeshTest.cpp
// -- display a rotating flat shaded cube
// cs250 5/19
//
// To compile/link using Visual Studio Command Prompt:
//     cl /EHsc CubeMeshTest.cpp Affine.lib CubeMesh.cpp
//        opengl32.lib glew32.lib SDL2.lib SDL2main.lib /link /subsystem:console
// To compile under Linux:
//     g++ CubeMeshTest.cpp Affine.cpp CubeMesh.cpp
//         -lGL -lGLEW -lSDL2


#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "CubeMesh.h"
using namespace std;


const glm::vec4 O = cs250::point(0,0,0),
                EZ = cs250::vector(0,0,1);


namespace {
  float frand(float a=0, float b=1) {
    return a + (b-a)*float(rand())/float(RAND_MAX);
  }
}


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    cs250::CubeMesh cube;
    glm::vec4 cube_center;
    float cube_scale,
          cube_rot_rate;
    glm::vec4 cube_rot_axis;
    double time;
    bool draw_solid;
    glm::mat4 ObjToWorld,
              PersProj;
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

  // cube parameters
  cube_center = cs250::point(0,0,-2.5f);
  cube_scale = 1.4f;
  cube_rot_rate = 360.0f/10.0f;
  cube_rot_axis = glm::normalize(cs250::vector(frand(-1,1),frand(-1,1),frand(-1,1)));
  draw_solid = false;
  time = 0;

  // perspective projection
  PersProj[0] = glm::vec4(1.4f,0,0,0);
  PersProj[1] = glm::vec4(0,1.4f,0,0);
  PersProj[2] = glm::vec4(0,0,-10.1f/9.9f,-1);
  PersProj[3] = glm::vec4(0,0,-2/9.9f,0);
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

  // modeling transform update
  glm::mat4 ObjToWorld = cs250::translate(cube_center-O)
                         * cs250::rotate(cs250::angle(EZ,cube_rot_axis),
                                         cs250::cross(EZ,cube_rot_axis))
                         * cs250::rotate(cube_rot_rate*time,EZ)
                         * cs250::scale(0.5f*cube_scale);

  // compute mesh vertices in NDC (we should really do this
  //   on the GPU side)
  glm::mat4 obj2ndc = PersProj * ObjToWorld;
  vector<glm::vec4> ndc_verts(cube.vertexCount());
  for (int i=0; i < cube.vertexCount(); ++i)
    ndc_verts[i] = obj2ndc * cube.vertexArray()[i];

  // draw mesh
  glUseProgram(program);
  glEnableVertexAttribArray(aposition);

  if (draw_solid) {
    for (int n=0; n < cube.faceCount(); ++n) {
      const cs250::Mesh::Face &f = cube.faceArray()[n];
      const glm::vec4 &P = ndc_verts[f.index1],
                      &Q = ndc_verts[f.index2],
                      &R = ndc_verts[f.index3];
      glm::vec4 m = glm::normalize(cs250::cross(Q-P,R-P));
      float shade = glm::max(m.z,0.0f);
      glUniform3f(ucolor,shade,0.75f*shade,shade);

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
    for (int n=0; n < cube.edgeCount(); ++n) {
      const cs250::Mesh::Edge &e = cube.edgeArray()[n];
      const glm::vec4 &P = ndc_verts[e.index1],
                      &Q = ndc_verts[e.index2];
      glUniform3f(ucolor,0.5f,0,0.5f);
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

  time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE)
    draw_solid = !draw_solid;
  else if (kc == SDLK_r)
    cube_rot_axis = glm::normalize(cs250::vector(frand(-1,1),frand(-1,1),frand(-1,1)));
}


void Client::resize(int W, int H) {
  int D = min(W,H);
  glViewport(0,0,D,D);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  srand(unsigned(time(0)));

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: CubeMesh Test";
  int width = 600,
      height = 600;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
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

