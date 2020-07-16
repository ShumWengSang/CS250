// Shum Weng Sang
// Assignment 5
// CS 250

#include "DiffuseRender.h"
#include "Affine.h"

static const GLuint uninitializedNumber = -1;

namespace cs250
{
	DiffuseRender::DiffuseRender(void) : face_buffer(uninitializedNumber), face_count(uninitializedNumber), 
		vao(uninitializedNumber), vertex_buffer(uninitializedNumber), normal_buffer(uninitializedNumber)
	{
		const char * fragment_shader_text = 
			"#version 130\n\
     in vec3 camera_normal;\
     uniform vec3 color;\
     out vec4 frag_color;\
     void main(void) {\
       vec3 m = normalize(camera_normal);\
       float shade = max(0,m.z);\
       frag_color = vec4(shade*color,1);\
     }";

		GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fshader, 1, &fragment_shader_text, 0);
		glCompileShader(fshader);

		GLint value;
		glGetShaderiv(fshader, GL_COMPILE_STATUS, &value);
		if (!value) {
			throw std::runtime_error("Failed compiling of fragment shader programs");
		}

		const char * vertex_shader_text =
    "#version 130\n\
     in vec4 position;\
     in vec4 normal;\
     uniform mat4 persp_matrix;\
     uniform mat4 view_matrix;\
     uniform mat4 model_matrix;\
     uniform mat4 normal_matrix;\
     out vec3 camera_normal;\
     void main() {\
       gl_Position = persp_matrix * view_matrix * model_matrix * position;\
       camera_normal = mat3(view_matrix) * mat3(normal_matrix) * normal.xyz;\
     }";

		GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vshader, 1, &vertex_shader_text, 0);
		glCompileShader(vshader);

		glGetShaderiv(vshader, GL_COMPILE_STATUS, &value);
		if (!value) {
			throw std::runtime_error("Failed compiling of vertex shader programs");
		}

		// link shader program
		program = glCreateProgram();
		glAttachShader(program, fshader);
		glAttachShader(program, vshader);
		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &value);
		if (!value) {
			throw std::runtime_error("Failed linking of shader programs");
		}
		glDeleteShader(vshader);
		glDeleteShader(fshader);

		upersp_matrix = glGetUniformLocation(program, "persp_matrix");
		uview_matrix = glGetUniformLocation(program, "view_matrix");
		umodel_matrix = glGetUniformLocation(program, "model_matrix");
		unormal_matrix = glGetUniformLocation(program, "normal_matrix");
		ucolor = glGetUniformLocation(program, "color");

		glEnable(GL_DEPTH_TEST);
	}
	DiffuseRender::~DiffuseRender(void)
	{
		glUseProgram(0);
		glDeleteProgram(program);
	}

	void DiffuseRender::backfaceCull(bool yes)
	{
		if (yes)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

	void DiffuseRender::clear(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void DiffuseRender::loadMesh(const NormalMesh& m)
	{
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

		// save drawing states in VAO
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

		face_count = m.faceCount();
	}

	void DiffuseRender::unloadMesh(void)
	{
		glDeleteBuffers(1, &normal_buffer);
		glDeleteBuffers(1, &vertex_buffer);
		glDeleteBuffers(1, &face_buffer);
		glDeleteVertexArrays(1, &vao);
	}

	void DiffuseRender::setModel(const glm::mat4& M)
	{
		glm::mat4 inverse = glm::transpose(affineInverse(M));
		glUseProgram(program);
		glUniformMatrix4fv(umodel_matrix, 1, false, &M[0][0]);
		glUniformMatrix4fv(unormal_matrix, 1, false, &inverse[0][0]);
	}

	void DiffuseRender::setView(const glm::mat4& V)
	{
		glUseProgram(program);
		glUniformMatrix4fv(uview_matrix, 1, false, &V[0][0]);
	}

	void DiffuseRender::setPerspective(const glm::mat4& P)
	{
		glUseProgram(program);
		glUniformMatrix4fv(upersp_matrix, 1, false, &P[0][0]);
	}

	void DiffuseRender::setColor(const glm::vec4& color)
	{
		glUseProgram(program);
		glUniform3f(ucolor, color.r, color.g, color.b);
	}

	void DiffuseRender::draw(void)
	{
		glUseProgram(program);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 3 * face_count, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}