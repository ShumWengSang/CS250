// Roland Shum
// CS 250
// Assignment 7

#version 130
uniform vec4 eye_position;
uniform vec3 diffuse_coefficient;
uniform vec3 specular_coefficient;
uniform float specular_exponent;
uniform vec3 ambient_color;

uniform vec4 light_position[8]; 
uniform vec3 light_color[8]; 
uniform int light_use[8];

in vec4 world_position;
in vec4 world_normal;

out vec4 frag_color;
void main(void)
{
  vec4 V = normalize(eye_position - world_position);
  vec4 M = normalize(world_normal);
  vec3 ambient = vec3(ambient_color.x * diffuse_coefficient.x, ambient_color.y * diffuse_coefficient.y, ambient_color.z * diffuse_coefficient.z);

  vec3 totalDiffuse = vec3(0);
  vec3 totalSpecular = vec3(0);
  for(int i = 0; i < 8; ++i)
  {
    if(light_use[i] != 0)
    {
        vec4 L = normalize(light_position[i] - world_position);
        vec4 R = normalize(2 * (dot(M, L)) * M - L);

        vec3 diffuse = diffuse_coefficient * max(dot(L,world_normal),0.0);
        diffuse *= light_color[i];
        totalDiffuse += diffuse;

        vec3 specular = specular_coefficient * pow(max(dot(R, V), 0.0), specular_exponent);
        specular *= light_color[i];
        totalSpecular += specular;
     }
  }
  frag_color = vec4(world_normal.w);
  // ambient + diffuse + specular
  frag_color = vec4(ambient + totalSpecular, 1.0);
};