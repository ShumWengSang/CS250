// FlatShadedMeshTest.cpp
// -- display a rotating sphere
// cs250 5/20
//
// To compile/link using Visual Studio Command Prompt:
//     cl /EHsc FlatShadedMeshTest.cpp Affine.lib SphereMesh.cpp FlatShadedMesh.cpp
//        opengl32.lib glew32.lib SDL2.lib SDL2main.lib /link /subsystem:console
// To compile under Linux:
//     g++ FlatShadedMeshTest.cpp Affine.cpp SphereMesh.cpp FlatShadedMesh.cpp
//         -lGL -lGLEW -lSDL2


#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "SphereMesh.h"
using namespace std;


const int WIN_WIDTH_INIT = 600,
          WIN_HEIGHT_INIT = 600;

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
    int face_count[2];
    glm::vec4 sph_center;
    float sph_scale,
          sph_rot_rate;
    glm::vec4 sph_rot_axis;
    double time;
    bool render_flat;
    GLint utransform,
          uperspective;
    enum { VERT=0, NORM=1, FACE=2 };
    GLuint program,
           vaos[2],
           buffers[6];
};


Client::Client(void) {
  // compile and link shader program
  const char *fragment_shader_text =
    "#version 130\n\
     in vec4 world_normal;\
     out vec4 frag_color;\
     void main(void) {\
       vec4 m = normalize(world_normal);\
       float shading = max(0,m.z);\
       frag_color = vec4(shading,0.75*shading,1,1);\
     }";
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader,1,&fragment_shader_text,0);
  glCompileShader(fshader);

  const char *vertex_shader_text =
    "#version 130\n\
     in vec4 position;\
     in vec4 normal;\
     uniform mat4 transform;\
     uniform mat4 perspective;\
     out vec4 world_normal;\
     void main() {\
       gl_Position = perspective * transform * position;\
       world_normal = transform * normal;\
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
  utransform = glGetUniformLocation(program,"transform");
  uperspective = glGetUniformLocation(program,"perspective");

  glEnable(GL_DEPTH_TEST);

  // vertex, normal, & face buffers
  int size = 4 + rand()%17;
  cs250::NormalMesh *meshes[2] = {
    new cs250::SphereMesh(size),
    cs250::createFlatShadedMesh(*meshes[0])
  };

  glGenBuffers(6,buffers);
  for (int i=0; i < 2; ++i) {
    cs250::NormalMesh &m = *meshes[i];
    glBindBuffer(GL_ARRAY_BUFFER,buffers[3*i+VERT]);
    glBufferData(GL_ARRAY_BUFFER,m.vertexCount()*sizeof(glm::vec4),
                 m.vertexArray(),GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,buffers[3*i+NORM]);
    glBufferData(GL_ARRAY_BUFFER,m.vertexCount()*sizeof(glm::vec4),
                 m.normalArray(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[3*i+FACE]);
    face_count[i] = m.faceCount();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,face_count[i]*sizeof(cs250::Mesh::Face),
                 m.faceArray(),GL_STATIC_DRAW);
    delete meshes[i];
  }

  // VAOS
  GLuint aposition = glGetAttribLocation(program,"position"),
         anormal = glGetAttribLocation(program,"normal");
  glGenVertexArrays(2,vaos);
  for (int i=0; i < 2; ++i) {
    glBindVertexArray(vaos[i]);
    glBindBuffer(GL_ARRAY_BUFFER,buffers[3*i+VERT]);
    glVertexAttribPointer(aposition,4,GL_FLOAT,false,sizeof(glm::vec4),0);
    glEnableVertexAttribArray(aposition);
    glBindBuffer(GL_ARRAY_BUFFER,buffers[3*i+NORM]);
    glVertexAttribPointer(anormal,4,GL_FLOAT,false,sizeof(glm::vec4),0);
    glEnableVertexAttribArray(anormal);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[3*i+FACE]);
    glBindVertexArray(0);
  }

  // sphere parameters
  sph_center = cs250::point(0,0,-2.5f);
  sph_scale = 1.4f;
  sph_rot_rate = 360.0f/10.0f;
  sph_rot_axis = glm::normalize(cs250::vector(frand(-1,1),frand(-1,1),frand(-1,1)));
  time = 0;

  // start with smooth mesh
  render_flat = false;

  // force resize to set perspective transform;
  resize(WIN_WIDTH_INIT,WIN_HEIGHT_INIT);
}


Client::~Client(void) {
  glDeleteVertexArrays(2,vaos);
  glDeleteBuffers(6,buffers);
  glUseProgram(0);
  glDeleteProgram(program);
}


void Client::draw(double dt) {
  // clear frame buffer and z-buffer
  glClearColor(0.95f,0.95f,0.95f,1);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  // modeling transform
  glm::mat4 obj_to_world = cs250::translate(sph_center-O)
                           * cs250::rotate(cs250::angle(EZ,sph_rot_axis),
                                           cs250::cross(EZ,sph_rot_axis))
                           * cs250::rotate(sph_rot_rate*time,EZ)
                           * cs250::scale(0.9f*sph_scale);

  // render mesh
  glUseProgram(program);
  glUniformMatrix4fv(utransform,1,false,&obj_to_world[0][0]);
  int index = render_flat ? 1 : 0;
  glBindVertexArray(vaos[index]);
  glDrawElements(GL_TRIANGLES,3*face_count[index],GL_UNSIGNED_INT,0);
  glBindVertexArray(0);

  time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE)
    render_flat = !render_flat;
  else if (kc == SDLK_r)
    sph_rot_axis = glm::normalize(cs250::vector(frand(-1,1),frand(-1,1),frand(-1,1)));
}


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);
  glm::mat4 persp = {{1.4f,0,0,0},
                     {0,1.4f*float(W)/float(H),0,0},
                     {0,0,-1.0f,-1.0f},
                     {0,0,-0.2f,0}};
  glUseProgram(program);
  glUniformMatrix4fv(uperspective,1,false,&persp[0][0]);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  srand(unsigned(time(nullptr)));

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: FlatShadedMesh Test";
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,WIN_WIDTH_INIT,
                                        WIN_HEIGHT_INIT,
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

