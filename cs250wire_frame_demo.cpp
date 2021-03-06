// cs250wire_frame_demo.cpp
// -- rendering a wire frame using OpenGL
// cs250 5/20
//
// Windows:
//   cl /EHsc cs250wire_frame_demo.cpp Affine.lib SphereMesh.cpp
//      opengl32.lib glew32.lib sdl2.lib sdl2main.lib
//      /link /subsystem:console
//
// Linux:
//   g++ -std=c++11 cs250wire_frame_demo.cpp Affine.cpp
//       SphereMesh.cpp -lGL -lGLEW -lSDL2

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "SphereMesh.h"
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
    GLint program,
          upersp_matrix,
          uview_matrix,
          umodel_matrix,
          ucolor;
    GLuint vao,
           vertex_buffer,
           edge_buffer;

    cs250::SphereMesh sphere;
    float sph_rot_rate;
    glm::vec4 sph_rot_axis,
              sph_center;
    glm::mat4 sph_model;
};


Client::Client(void)
     : sphere(10) {

  // compile fragment shader
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

  GLint value;
  glGetShaderiv(fshader,GL_COMPILE_STATUS,&value);
  if (!value) {
    string msg = "fragment shader error:\n";
    char buffer[1024];
    glGetShaderInfoLog(fshader,1024,0,buffer);
    msg += buffer;
    cout << msg << endl;
  }

  // compile vertex shader
  const char *vertex_shader_text =
    "#version 130\n\
     in vec4 position;\
     uniform mat4 persp_matrix;\
     uniform mat4 view_matrix;\
     uniform mat4 model_matrix;\
     void main() {\
       gl_Position = persp_matrix * view_matrix * model_matrix * position;\
     }";
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader,1,&vertex_shader_text,0);
  glCompileShader(vshader);

  glGetShaderiv(vshader,GL_COMPILE_STATUS,&value);
  if (!value) {
    string msg = "vertex shader error:\n";
    char buffer[1024];
    glGetShaderInfoLog(vshader,1024,0,buffer);
    msg += buffer;
    cout << msg << endl;
  }

  // link shader program
  program = glCreateProgram();
  glAttachShader(program,fshader);
  glAttachShader(program,vshader);
  glLinkProgram(program);
  glGetProgramiv(program,GL_LINK_STATUS,&value);
  if (!value) {
    string msg = "failed to link:\n";
    char buffer[1024];
    glGetProgramInfoLog(program,1024,0,buffer);
    msg += buffer;
    cout << msg << endl;
  }
  glDeleteShader(vshader);
  glDeleteShader(fshader);

  // shader parameter locations
  upersp_matrix = glGetUniformLocation(program,"persp_matrix");
  uview_matrix = glGetUniformLocation(program,"view_matrix");
  umodel_matrix = glGetUniformLocation(program,"model_matrix");
  ucolor = glGetUniformLocation(program,"color");

  // load sphere mesh (wire frame only)
  glGenBuffers(1,&vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER,sizeof(glm::vec4)*sphere.vertexCount(),
               sphere.vertexArray(),GL_STATIC_DRAW);

  glGenBuffers(1,&edge_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,edge_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(cs250::Mesh::Edge)*sphere.edgeCount(),
               sphere.edgeArray(),GL_STATIC_DRAW);

  // create vertex array object
  glGenVertexArrays(1,&vao);
  //   start state recording
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  GLint aposition = glGetAttribLocation(program,"position");
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,0,0);
  glEnableVertexAttribArray(aposition);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,edge_buffer);
  //   stop state recording
  glBindVertexArray(0);


  // initialize sphere modeling transform
  sph_rot_rate = 360.0f/10.0f;
  sph_rot_axis = EY;
  sph_center = cs250::point(0,0,-2);
  sph_model = cs250::translate(sph_center-O)
              * cs250::rotate(-90,EX)
              * cs250::scale(1.75f);

  // set shader program parameters
  glUseProgram(program);
  glUniform3f(ucolor,PURPLE.r,PURPLE.g,PURPLE.b);
  glm::mat4 V = cs250::translate(-EZ);
  glUniformMatrix4fv(uview_matrix,1,false,&V[0][0]);
  glm::mat4 P = { {1.2f,0,0,0},
                  {0,1.2f,0,0},
                  {0,0,0,-1},
                  {0,0,0,0} };
  glUniformMatrix4fv(upersp_matrix,1,false,&P[0][0]);
}


Client::~Client(void) {
  glDeleteBuffers(1,&edge_buffer);
  glDeleteBuffers(1,&vertex_buffer);
  glDeleteVertexArrays(1,&vao);

  glUseProgram(0);
  glDeleteProgram(program);
}


void Client::draw(double dt) {
  // clear frame buffer
  glClearColor(LT_GRAY.r,LT_GRAY.g,LT_GRAY.b,LT_GRAY.a);
  glClear(GL_COLOR_BUFFER_BIT);

  // rotating sphere modeling transformation
  sph_model = cs250::translate(sph_center-O)
              * cs250::rotate(sph_rot_rate*dt,sph_rot_axis)
              * cs250::translate(O-sph_center)
              * sph_model;

  // draw sphere
  glUseProgram(program);
  glUniformMatrix4fv(umodel_matrix,1,false,&sph_model[0][0]);
  // recall state
  glBindVertexArray(vao);
  glLineWidth(3.0f);
  glDrawElements(GL_LINES,2*sphere.edgeCount(),GL_UNSIGNED_INT,0);
  // don't forget to stop state recording
  glBindVertexArray(0);
}


void Client::keypress(SDL_Keycode kc) { }


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);

  // reset the camera and shader program parameters
  float aspect = float(W)/float(H);
  glUseProgram(program);
  glm::mat4 P = { {1.2f,0,0,0},
                  {0,1.2f*aspect,0,0},
                  {0,0,0,-1},
                  {0,0,0,0} };
  glUniformMatrix4fv(upersp_matrix,1,false,&P[0][0]);
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

