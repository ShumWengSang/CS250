// CS250
// Shum Weng Sang
// Assignment 7

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "PhongRender.h"

namespace cs250
{
	PhongRender::PhongRender(void)
	{
        // compile fragment & vertex shaders
        GLuint shader[2];
        GLenum type[2] = { GL_FRAGMENT_SHADER, GL_VERTEX_SHADER };
        const char* fname[2] = { "PhongRender.frag", "PhongRender.vert" };
        GLint value;
        for (int i = 0; i < 2; ++i) {

            // load text file
            std::string shader_text;
            std::ifstream in(fname[i]);
            while (in) {
                std::string line;
                std::getline(in, line);
                shader_text += line + "\n";
            }

            // compile shader
            shader[i] = glCreateShader(type[i]);
            const char* text = shader_text.c_str();
            glShaderSource(shader[i], 1, &text, 0);
            glCompileShader(shader[i]);

            // failure check
            glGetShaderiv(shader[i], GL_COMPILE_STATUS, &value);
            if (!value) {
                std::string msg = "error in file ";
                msg += fname[i];
                msg += "\n";
                char buffer[1024];
                glGetShaderInfoLog(shader[i], 1024, 0, buffer);
                msg += buffer;
                std::cout << msg << std::endl;
            }
        }

        // link shader program
        program = glCreateProgram();
        glAttachShader(program, shader[0]);
        glAttachShader(program, shader[1]);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &value);
        if (!value) {
            std::string msg = "failed to link:\n";
            char buffer[1024];
            glGetProgramInfoLog(program, 1024, 0, buffer);
            msg += buffer;
            std::cout << msg << std::endl;
        }
        glDeleteShader(shader[0]);
        glDeleteShader(shader[1]);

        // turn on depth buffer
        glEnable(GL_DEPTH_TEST);

        // shader uniform variable locations
        upersp_matrix = glGetUniformLocation(program, "persp_matrix");
        uview_matrix = glGetUniformLocation(program, "view_matrix");
        umodel_matrix = glGetUniformLocation(program, "model_matrix");
        unormal_matrix = glGetUniformLocation(program, "normal_matrix");

        ulight_position = glGetUniformLocation(program, "light_position");
        ueye_position = glGetUniformLocation(program, "eye_position");
        ulight_color = glGetUniformLocation(program, "light_color");
        udiffuse_coefficient = glGetUniformLocation(program, "diffuse_coefficient");
        uspecular_coefficient = glGetUniformLocation(program, "specular_coefficient");
        uspecular_exponent = glGetUniformLocation(program, "specular_exponent");
        uambient_color = glGetUniformLocation(program, "ambient_color");
        ulight_use = glGetUniformLocation(program, "light_use");

        // Initialize use light arr to 0.
        glUseProgram(program);
        int array[8] = { 0 };
        glUniform1iv(ulight_use, 8, array);
	}

	PhongRender::~PhongRender(void)
	{
        glUseProgram(0);
        glDeleteProgram(program);
	}

	void PhongRender::backfaceCull(bool yes)
	{
        if (yes)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
	}

	void PhongRender::clear(const glm::vec4& color)
	{
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearDepth(1);
	}

	int PhongRender::loadMesh(const cs250::NormalMesh& m)
	{
        glUseProgram(program);
        MeshData newMeshData;
        GLuint& vertex_buffer = newMeshData.buffer_objects[static_cast<int>(MeshData::VERT)];
        GLuint& normal_buffer = newMeshData.buffer_objects[static_cast<int>(MeshData::NORM)];
        GLuint& face_buffer = newMeshData.buffer_objects[static_cast<int>(MeshData::FACE)];
        GLuint& vao = newMeshData.vertex_array_buffer;

        // load sphere mesh (solid frame only)
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * m.vertexCount(),
            m.vertexArray(), GL_STATIC_DRAW);

        glGenBuffers(1, &normal_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * m.vertexCount(),
            m.normalArray(), GL_STATIC_DRAW);

        glGenBuffers(1, &face_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            sizeof(cs250::Mesh::Face) * m.faceCount(),
            m.faceArray(), GL_STATIC_DRAW);

        // VAO
        GLint aposition = glGetAttribLocation(program, "position"),
            anormal = glGetAttribLocation(program, "normal");
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(aposition, 4, GL_FLOAT, false, 0, 0);
        glEnableVertexAttribArray(aposition);

        glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
        glVertexAttribPointer(anormal, 4, GL_FLOAT, false, 0, 0);
        glEnableVertexAttribArray(anormal);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
        glBindVertexArray(0);

        newMeshData.face_count = m.faceCount();
        mesh_data.push_back(newMeshData);

		return mesh_data.size() - 1;
	}

	void PhongRender::unloadMesh(int mi)
	{
        glUseProgram(program);
        MeshData& newMeshData = mesh_data[mi];
        GLuint& vertex_buffer = newMeshData.buffer_objects[static_cast<int>(MeshData::VERT)];
        GLuint& normal_buffer = newMeshData.buffer_objects[static_cast<int>(MeshData::NORM)];
        GLuint& face_buffer = newMeshData.buffer_objects[static_cast<int>(MeshData::FACE)];
        GLuint& vao = newMeshData.vertex_array_buffer;

        glDeleteBuffers(1, &normal_buffer);
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteBuffers(1, &face_buffer);
        glDeleteVertexArrays(1, &vao);

        newMeshData.face_count = 0;
	}

	void PhongRender::setModel(const glm::mat4& M)
	{
        glUseProgram(program);
        glm::mat4 inverse = glm::transpose(affineInverse(M));
        glUniformMatrix4fv(umodel_matrix, 1, false, &M[0][0]);
        glUniformMatrix4fv(unormal_matrix, 1, false, &inverse[0][0]);
	}

	void PhongRender::setCamera(const cs250::Camera& cam)
	{
        glUseProgram(program);
        glm::mat4x4 perspectiveMatrix = perspective(cam);
        glm::mat4x4 viewMatrix = view(cam); 
        glm::vec4 eyePos = cam.eye();
        glUniformMatrix4fv(upersp_matrix, 1, false, &perspectiveMatrix[0][0]);
        glUniformMatrix4fv(uview_matrix, 1, false, &viewMatrix[0][0]);

        glUniform4fv(ueye_position, 1, &eyePos[0]);
	}

	void PhongRender::setMaterial(const glm::vec3& diff_coef, const glm::vec3& spec_coef, float spec_exp)
	{
        glUseProgram(program);
        glUniform3fv(udiffuse_coefficient, 1, &diff_coef[0]);
        glUniform3fv(uspecular_coefficient, 1, &spec_coef[0]);
        glUniform1f(uspecular_exponent, spec_exp);
	}

	void PhongRender::setLight(int li, const glm::vec4& position, const glm::vec3& color)
	{
        if (li < 0 || li >= 8)
            return;
        glUseProgram(program); 
        // Set the light position
        glUniform4fv(ulight_position + li, 1, &position[0]);
        // Set the light color
        glUniform3fv(ulight_color + li, 1, &color[0]);
        // Turn light on
        glUniform1i(ulight_use + li, 1);
	}

	void PhongRender::setAmbient(const glm::vec3& color)
	{
        glUseProgram(program);
        glUniform3fv(uambient_color, 1, &color[0]);
	}

	void PhongRender::draw(int mi)
	{
        glUseProgram(program);
        if (mesh_data[mi].face_count != 0)
        {
            // Draw the object
            glBindVertexArray(mesh_data[mi].vertex_array_buffer);
            glDrawElements(GL_TRIANGLES, 3 * mesh_data[mi].face_count, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
	}
}