// Jonathan Bourim  |  CS250 Assignment 7  |  03/27/2020

#include "PhongRender.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace cs250
{

	PhongRender::PhongRender(void)
	{
		// Get files locally
		std::ifstream fragFile("./phong.frag");
		std::ifstream vertFile("./phong.vert");
		std::stringstream fragStream, vertStream;
		std::string fragSrc, vertSrc;
		
		if (!fragFile)
		{
			std::string msg = "Cannot open fragment shader file";
			throw msg.c_str();
		}

		// Dump file to string stream
		fragStream << fragFile.rdbuf();
		fragFile.close();

		if (!vertFile)
		{
			std::string msg = "Cannot open vertex shader file";
			throw msg.c_str();
		}
		
		vertStream << vertFile.rdbuf();
		vertFile.close();

		fragSrc = fragStream.str();
		vertSrc = vertStream.str();
		// Create src const char pointers
		const char * frag = fragSrc.c_str();
		const char * vert = vertSrc.c_str();
	
		// Compile fragment shader
		GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fshader, 1, &frag, 0);
		glCompileShader(fshader);

		GLint value;
		glGetShaderiv(fshader, GL_COMPILE_STATUS, &value);
		if (!value) {
			std::string msg = "fragment shader error:\n";
			char buffer[1024];
			glGetShaderInfoLog(fshader, 1024, 0, buffer);
			msg += buffer;

			throw msg.c_str();
		}

		// Compile vertex shader
		GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vshader, 1, &vert, 0);
		glCompileShader(vshader);

		glGetShaderiv(vshader, GL_COMPILE_STATUS, &value);
		if (!value) {
			std::string msg = "vertex shader error:\n";
			char buffer[1024];
			glGetShaderInfoLog(vshader, 1024, 0, buffer);
			msg += buffer;

			throw msg.c_str();
		}

		// link shader program
		program = glCreateProgram();
		glAttachShader(program, fshader);
		glAttachShader(program, vshader);
		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &value);
		if (!value) {
			std::string msg = "failed to link:\n";
			char buffer[1024];
			glGetProgramInfoLog(program, 1024, 0, buffer);
			msg += buffer;

			throw msg.c_str();
		}
		glDeleteShader(vshader);
		glDeleteShader(fshader);

		// turn on depth buffer
		glEnable(GL_DEPTH_TEST);

		// shader parameter locations
		upersp_matrix = glGetUniformLocation(program, "persp_matrix");
		uview_matrix = glGetUniformLocation(program, "view_matrix");
		umodel_matrix = glGetUniformLocation(program, "model_matrix");
		unormal_matrix = glGetUniformLocation(program, "normal_matrix");
		uambient_color = glGetUniformLocation(program, "ambient_color");
		ueye_position = glGetUniformLocation(program, "eye_position");
		udiffuse_coefficient = glGetUniformLocation(program, "diffuse_coefficient");
		uspecular_coefficient = glGetUniformLocation(program, "specular_coefficient");
		uspecular_exponent = glGetUniformLocation(program, "specular_exponent");
		ulight_position = glGetUniformLocation(program, "light_position");
		ulight_color = glGetUniformLocation(program, "light_color");
		ulight_use = glGetUniformLocation(program, "light_use");
	}

	PhongRender::~PhongRender(void)
	{
		glDeleteProgram(program);
	}

	void PhongRender::clear(const glm::vec4& color)
	{
		glClearColor(color.x, color.y, color.z, color.w);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	int PhongRender::loadMesh(const NormalMesh& m)
	{
		MeshData* mesh = nullptr;
		// Check if reuseable mesh exists
		for (auto & it : mesh_data)
		{
			if (it.face_count == 0)
			{
				mesh = &it;
			}
		}
		if (!mesh)
		{
			mesh_data.emplace_back();
			mesh = &mesh_data.back();
		}

		// Load Mesh
		glGenBuffers(1, &mesh->buffer_objects[MeshData::VERT]);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->buffer_objects[MeshData::VERT]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * m.vertexCount(),
			m.vertexArray(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh->buffer_objects[MeshData::NORM]);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->buffer_objects[MeshData::NORM]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * m.vertexCount(),
			m.normalArray(), GL_STATIC_DRAW);

		mesh->face_count = m.faceCount();

		glGenBuffers(1, &mesh->buffer_objects[MeshData::FACE]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffer_objects[MeshData::FACE]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(cs250::Mesh::Face) * mesh->face_count,
			m.faceArray(), GL_STATIC_DRAW);

		// Get attribs and set/enable
		GLint aposition = glGetAttribLocation(program, "position"),
			anormal = glGetAttribLocation(program, "normal");
		glGenVertexArrays(1, &mesh->vertex_array_buffer);
		glBindVertexArray(mesh->vertex_array_buffer);

		glBindBuffer(GL_ARRAY_BUFFER, mesh->buffer_objects[MeshData::VERT]);
		glVertexAttribPointer(aposition, 4, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(aposition);

		glBindBuffer(GL_ARRAY_BUFFER, mesh->buffer_objects[MeshData::NORM]);
		glVertexAttribPointer(anormal, 4, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(anormal);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffer_objects[MeshData::FACE]);
		glBindVertexArray(0);

		// Return index
		return (mesh_data.size() - 1);
	}

	void PhongRender::unloadMesh(int mi)
	{
		// Unload mesh at mi
		MeshData & mesh = mesh_data[mi];

		glDeleteVertexArrays(1, &mesh.vertex_array_buffer);
		glDeleteBuffers(3, mesh.buffer_objects);	
		mesh.face_count = 0;
	}


	void PhongRender::setModel(const glm::mat4& M)
	{
		glUseProgram(program);

		glUniformMatrix4fv(umodel_matrix, 1, GL_FALSE, &M[0][0]);

		// Get inverse transpose of the linear portion of the model matrix
		glm::mat4 normalMatrix = affineInverse(glm::transpose(M));
		glUniformMatrix4fv(unormal_matrix, 1, GL_FALSE, &normalMatrix[0][0]);
	}

	void PhongRender::setCamera(const Camera& cam)
	{
		glUseProgram(program);
	
		// Persp, view and eye pos
		glUniformMatrix4fv(upersp_matrix, 1, GL_FALSE, &perspective(cam)[0][0]);
		glUniformMatrix4fv(uview_matrix, 1, GL_FALSE, &view(cam)[0][0]);
		glUniform4fv(ueye_position, 1, &cam.eye()[0]);
	}

	void PhongRender::setMaterial(const glm::vec3& diff_coef, const glm::vec3& spec_coef, float spec_exp)
	{
		glUseProgram(program);

		// Diffuse coeff, specular coeff, and spec exp
		glUniform3fv(udiffuse_coefficient, 1, &diff_coef[0]);
		glUniform3fv(uspecular_coefficient, 1, &spec_coef[0]);
		glUniform1f(uspecular_exponent, spec_exp);
	}

	void PhongRender::setLight(int li, const glm::vec4& position, const glm::vec3& color)
	{
		glUseProgram(program);

		// Light at index li
		glUniform4fv(ulight_position + li, 1, &position[0]);
		glUniform3fv(ulight_color + li , 1, &color[0]);
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
		MeshData& mesh = mesh_data[mi];

		glBindVertexArray(mesh.vertex_array_buffer);
		glDrawElements(GL_TRIANGLES, 3 * mesh.face_count, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}


}
