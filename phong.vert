// Roland Shum
// CS 250
// Assignment 6


#version 130
in vec4 position;
in vec4 normal;

uniform mat4 persp_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat4 normal_matrix;

out vec4 world_normal;
out vec4 world_position;

void main()
{
  gl_Position = persp_matrix * view_matrix * model_matrix * position;
  world_position = model_matrix * position;
  world_normal = normal_matrix * normal;
};