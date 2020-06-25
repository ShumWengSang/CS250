// Roland Shum
// CS 250
// Assignment 6

#version 130
uniform vec4 light_position;
uniform vec4 eye_position;
uniform vec3 light_color;
uniform vec3 diffuse_coefficient;
uniform vec3 specular_coefficient;
uniform float specular_exponent;
uniform vec3 ambient_color;

in vec4 world_position;
in vec4 world_normal;

out vec4 frag_color;
void main(void)
{
  vec4 V = normalize(eye_position - world_position);
  vec4 M = normalize(world_normal);
  vec4 L = normalize(light_position - world_position);
  vec4 R = normalize(2 * (dot(M, L)) * M - L);

  vec3 ambient = vec3(ambient_color.x * diffuse_coefficient.x, ambient_color.y * diffuse_coefficient.y, ambient_color.z * diffuse_coefficient.z);

  vec3 diffuse = diffuse_coefficient * max(dot(L,world_normal),0.0);
  diffuse = vec3(diffuse.x * light_color.x, diffuse.y * light_color.y, diffuse.z * light_color.z);

  vec3 specular = specular_coefficient * pow(max(dot(R, V), 0.0), specular_exponent);
  specular = vec3(specular.x * light_color.x, specular.y * light_color.y, specular.z * light_color.z);

  // ambient + diffuse + specular
  frag_color = vec4(ambient + specular + diffuse, 1.0);
};