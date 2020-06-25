// LightingTest.cpp
// -- test of vertex+fragment shaders for Phong lighting
// cs250 6/20
//
// Windows:
//   cl /EHsc LightingTest.cpp Affine.lib SphereMesh.cpp
//      Camera.lib CameraTransform.lib opengl32.lib glew32.lib
//      sdl2.lib sdl2main.lib /link /subsystem:console
//
// Linux:
//   g++ -std=c++11 LightingTest.cpp Affine.cpp
//       SphereMesh.cpp Camera.cpp CameraTransform.cpp
//       -lGL -lGLEW -lSDL2
//
// Note:
//   The vertex shader must be named 'phong.vert', and
//   the fragment shader must be named 'phong.frag'.
//   These files must be present in the same folder the
//   program is run from.


#include <iostream>
#include <fstream>
#include <string>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "Camera.h"
#include "SphereMesh.h"
using namespace std;


const glm::vec4 O = cs250::point(0,0,0),
                EX = cs250::vector(1,0,0),
                EY = cs250::vector(0,1,0),
                EZ = cs250::vector(0,0,1),
                DK_GRAY(0.1f,0.1f,0.1f,1);

const glm::vec3 LIGHT_COLOR(1,1,1),
                AMBIENT_COLOR(0.2f,0.2f,0.2f);

class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
    void mousedrag(int dx, int dy, bool left_button);
  private:
    GLint program,
          upersp_matrix,
          uview_matrix,
          umodel_matrix,
          unormal_matrix,
          ulight_position,
          ueye_position,
          ulight_color,
          udiffuse_coefficient,
          uspecular_coefficient,
          uspecular_exponent,
          uambient_color;
    GLuint vao,
           vertex_buffer,
           normal_buffer,
           face_buffer;

    cs250::SphereMesh sphere;
    float sph_rot_rate;
    glm::vec4 sph_rot_axis,
              sph_center;
    bool rotate_sph;
    double time;

    glm::vec4 light_position;
    cs250::Camera camera;

    struct Material {
      glm::vec3 diffuse_coefficient,
                specular_coefficient;
      float specular_exponent;
      glm::vec3 light_color,   // not really material properies, but
                ambient_color; // needed for rendering light source
    };
    Material properties[3];
};


Client::Client(void)
     : sphere(15) {

  // compile fragment & vertex shaders
  GLuint shader[2];
  GLenum type[2] = { GL_FRAGMENT_SHADER, GL_VERTEX_SHADER };
  const char *fname[2] = { "phong.frag", "phong.vert" };
  GLint value;
  for (int i=0; i < 2; ++i) {

    // load text file
    string shader_text;
    ifstream in(fname[i]);
    while (in) {
      string line;
      getline(in,line);
      shader_text += line + "\n";
    }

    // compile shader
    shader[i] = glCreateShader(type[i]);
    const char *text = shader_text.c_str();
    glShaderSource(shader[i],1,&text,0);
    glCompileShader(shader[i]);

    // failure check
    glGetShaderiv(shader[i],GL_COMPILE_STATUS,&value);
    if (!value) {
      string msg = "error in file ";
      msg += fname[i];
      msg += "\n";
      char buffer[1024];
      glGetShaderInfoLog(shader[i],1024,0,buffer);
      msg += buffer;
      cout << msg << endl;
    }
  }

  // link shader program
  program = glCreateProgram();
  glAttachShader(program,shader[0]);
  glAttachShader(program,shader[1]);
  glLinkProgram(program);
  glGetProgramiv(program,GL_LINK_STATUS,&value);
  if (!value) {
    string msg = "failed to link:\n";
    char buffer[1024];
    glGetProgramInfoLog(program,1024,0,buffer);
    msg += buffer;
    cout << msg << endl;
  }
  glDeleteShader(shader[0]);
  glDeleteShader(shader[1]);

  // turn on depth buffer
  glEnable(GL_DEPTH_TEST);

  // shader uniform variable locations
  upersp_matrix = glGetUniformLocation(program,"persp_matrix");
  uview_matrix = glGetUniformLocation(program,"view_matrix");
  umodel_matrix = glGetUniformLocation(program,"model_matrix");
  unormal_matrix = glGetUniformLocation(program,"normal_matrix");

  ulight_position = glGetUniformLocation(program,"light_position");
  ueye_position = glGetUniformLocation(program,"eye_position");
  ulight_color = glGetUniformLocation(program,"light_color");
  udiffuse_coefficient = glGetUniformLocation(program,"diffuse_coefficient");
  uspecular_coefficient = glGetUniformLocation(program,"specular_coefficient");
  uspecular_exponent = glGetUniformLocation(program,"specular_exponent");
  uambient_color = glGetUniformLocation(program,"ambient_color");

  // load sphere mesh (solid frame only)
  glGenBuffers(1,&vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER,sizeof(glm::vec4)*sphere.vertexCount(),
               sphere.vertexArray(),GL_STATIC_DRAW);

  glGenBuffers(1,&normal_buffer);
  glBindBuffer(GL_ARRAY_BUFFER,normal_buffer);
  glBufferData(GL_ARRAY_BUFFER,sizeof(glm::vec4)*sphere.vertexCount(),
               sphere.normalArray(),GL_STATIC_DRAW);

  glGenBuffers(1,&face_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(cs250::Mesh::Face)*sphere.faceCount(),
               sphere.faceArray(),GL_STATIC_DRAW);

  // VAO
  GLint aposition = glGetAttribLocation(program,"position"),
        anormal = glGetAttribLocation(program,"normal");
  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,0,0);
  glEnableVertexAttribArray(aposition);
  glBindBuffer(GL_ARRAY_BUFFER,normal_buffer);
  glVertexAttribPointer(anormal,4,GL_FLOAT,false,0,0);
  glEnableVertexAttribArray(anormal);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
  glBindVertexArray(0);

  glUseProgram(program);

  // sphere modeling transform parameters
  sph_rot_rate = 360.0f/10.0f;
  sph_rot_axis = EY;
  sph_center = cs250::point(0,0,-2);
  rotate_sph = true;
  time = 0;

  // material properties of meshes
  // (1) rotating sphere
  properties[0].diffuse_coefficient = glm::vec3(0.8f,0,0.8f);
  properties[0].specular_coefficient = glm::vec3(0.5f,0.5f,0.5f);
  properties[0].specular_exponent = 30;
  properties[0].light_color = LIGHT_COLOR;
  properties[0].ambient_color = AMBIENT_COLOR;
  // (2) light source
  properties[1].diffuse_coefficient = glm::vec3(1,1,1);
  properties[1].specular_coefficient = glm::vec3(0,0,0);
  properties[1].specular_exponent = 1000;
  properties[1].light_color = glm::vec3(0,0,0);
  properties[1].ambient_color = glm::vec3(1,1,1);
  // (3) "floor"
  properties[2].diffuse_coefficient = glm::vec3(0.2f,0.7f,0.2f);
  properties[2].specular_coefficient = glm::vec3(0.5f,0.5f,0.5f);
  properties[2].specular_exponent = 10;
  properties[2].light_color = LIGHT_COLOR;
  properties[2].ambient_color = AMBIENT_COLOR;

  // lighting and camera parameters
  light_position = O + EZ;
  glm::vec4 eye = O + 2.5f*EX + 0.1f*EY;
  glUniform4fv(ueye_position,1,&eye[0]);
  camera = cs250::Camera(eye,sph_center-eye,EY,80,1,0.1f,100);
  glm::mat4 V = cs250::view(camera);
  glUniformMatrix4fv(uview_matrix,1,false,&V[0][0]);
  glm::mat4 P = cs250::perspective(camera);
  glUniformMatrix4fv(upersp_matrix,1,false,&P[0][0]);
}


Client::~Client(void) {
  glDeleteBuffers(1,&face_buffer);
  glDeleteBuffers(1,&normal_buffer);
  glDeleteBuffers(1,&vertex_buffer);

  glUseProgram(0);
  glDeleteProgram(program);
}


void Client::draw(double dt) {
  // clear frame buffer and depth buffer
  glClearColor(DK_GRAY.r,DK_GRAY.g,DK_GRAY.b,DK_GRAY.a);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  // modeling and normal transforms
  glm::mat4 model[3],
            normal[3];
  // (1) rotating sphere
  model[0] = cs250::translate(sph_center-O)
             * cs250::rotate(sph_rot_rate*time,sph_rot_axis)
             * cs250::scale(0.5f,1.75f,1.0f),
  normal[0] = cs250::rotate(sph_rot_rate*time,sph_rot_axis)
              * cs250::scale(1.0f/0.5f,1.0f/1.75f,1.0f/1.0f);
  // (2) light source
  model[1] = cs250::translate(light_position-O)
             * cs250::scale(0.1f,0.1f,0.1f),
  normal[1] = glm::mat4(1);
  // (3) "floor"
  model[2] = cs250::translate(-1.75f*EY)
             * cs250::scale(100,0.1f,100);
  normal[2] = cs250::scale(1.0f/100,1.0f/0.1f,1.0f/100);

  // draw spheres
  glUseProgram(program);
  glBindVertexArray(vao);
  glUniform4fv(ulight_position,1,&light_position[0]);

  for (int i=0; i < 3; ++i) {
    glUniformMatrix4fv(umodel_matrix,1,false,&(model[i][0][0]));
    glUniformMatrix4fv(unormal_matrix,1,false,&(normal[i][0][0]));

    glUniform3fv(udiffuse_coefficient,1,&(properties[i].diffuse_coefficient[0]));
    glUniform3fv(uspecular_coefficient,1,&(properties[i].specular_coefficient[0]));
    glUniform1f(uspecular_exponent,properties[i].specular_exponent);
    glUniform3fv(ulight_color,1,&(properties[i].light_color[0]));
    glUniform3fv(uambient_color,1,&(properties[i].ambient_color[0]));

    glDrawElements(GL_TRIANGLES,3*sphere.faceCount(),GL_UNSIGNED_INT,0);
  }

  glBindVertexArray(0);

  if (rotate_sph)
    time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  const float FWD_UNIT = 0.1f;

  switch (kc) {
    case SDLK_SPACE:
      rotate_sph = !rotate_sph;
      break;
    case SDLK_UP:
      camera.forward(FWD_UNIT);
      break;
    case SDLK_DOWN:
      camera.forward(-FWD_UNIT);
  }
  glm::mat4 V = cs250::view(camera);
  glUniformMatrix4fv(uview_matrix,1,false,&V[0][0]);
  glUniform4fv(ueye_position,1,&camera.eye()[0]);
}


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);

  // reset the camera and shader program parameters
  float aspect = float(W)/float(H);
  glUseProgram(program);
  camera = cs250::Camera(camera.eye(),-camera.back(),EY,80,aspect,0.1f,100);
  glm::mat4 P = cs250::perspective(camera);
  glUniformMatrix4fv(upersp_matrix,1,false,&P[0][0]);
}


void Client::mousedrag(int dx, int dy, bool left_button) {
  const float ROT_UNIT = 1.0f;

  // left mouse drag: camera orientation
  if (left_button) {
    camera.yaw(-dx*ROT_UNIT);
    camera.pitch(-dy*ROT_UNIT);
    // force camera right vector to be perp to y-axis
    camera.roll(cs250::angle(camera.right(),EY)-90);
    glm::mat4 V = cs250::view(camera);
    glUniformMatrix4fv(uview_matrix,1,false,&V[0][0]);
    glUniform4fv(ueye_position,1,&camera.eye()[0]);
  }

  // right mouse drag: light position
  else {
    glm::vec4 L = light_position - sph_center;
    glm::mat4 R = cs250::rotate(dx*ROT_UNIT,EY)
                  * cs250::rotate(dy*ROT_UNIT,L.z*EX-L.x*EZ);
    light_position = sph_center + R*L;
  }
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("Lighting Test",SDL_WINDOWPOS_UNDEFINED,
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
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

