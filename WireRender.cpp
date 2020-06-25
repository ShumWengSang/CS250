// Shum Weng Sang
// Assignment 4
// CS 250

#include "WireRender.h"
#include <stdexcept>

static const GLuint uninitializedNumber = -1;

cs250::WireRender::WireRender(void) : edge_buffer(uninitializedNumber), edge_count(uninitializedNumber), vao(uninitializedNumber), vertex_buffer(uninitializedNumber)
{
    // compile fragment shader
    const char* fragment_shader_text =
        "#version 130\n\
     uniform vec3 color;\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = vec4(color,1);\
     }";
    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fragment_shader_text, 0);
    glCompileShader(fshader);

    GLint value;
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &value);
    if (!value) {
        throw std::runtime_error("Failed compiling of fragment shader programs");
    }

    // compile vertex shader
    const char* vertex_shader_text =
        "#version 130\n\
     in vec4 position;\
     uniform mat4 persp_matrix;\
     uniform mat4 view_matrix;\
     uniform mat4 model_matrix;\
     void main() {\
       gl_Position = persp_matrix * view_matrix * model_matrix * position;\
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

    // shader parameter locations
    upersp_matrix = glGetUniformLocation(program, "persp_matrix");
    uview_matrix = glGetUniformLocation(program, "view_matrix");
    umodel_matrix = glGetUniformLocation(program, "model_matrix");
    ucolor = glGetUniformLocation(program, "color");
}

cs250::WireRender::~WireRender(void)
{
    glUseProgram(0);
    glDeleteProgram(program);
}

void cs250::WireRender::clear(const glm::vec4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void cs250::WireRender::loadMesh(const Mesh& m)
{
    glUseProgram(program);
    // load vertex array
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * m.vertexCount(),
        m.vertexArray(), GL_STATIC_DRAW);

    // Edge buffer
    glGenBuffers(1, &edge_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(cs250::Mesh::Edge) * m.edgeCount(),
        m.edgeArray(), GL_STATIC_DRAW);

    // create vertex array object
    glGenVertexArrays(1, &vao);
    //   start state recording
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    GLint aposition = glGetAttribLocation(program, "position");
    glVertexAttribPointer(aposition, 4, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(aposition);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_buffer);
    //   stop state recording
    glBindVertexArray(0);

    edge_count = m.edgeCount();

}

void cs250::WireRender::unloadMesh(void)
{
    glDeleteBuffers(1, &edge_buffer);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteVertexArrays(1, &vao);
}

void cs250::WireRender::setModel(const glm::mat4& M)
{
    glUseProgram(program);
    glUniformMatrix4fv(umodel_matrix, 1, false, &M[0][0]);
}

void cs250::WireRender::setView(const glm::mat4& V)
{
    glUseProgram(program);
    glUniformMatrix4fv(uview_matrix, 1, false, &V[0][0]);
}

void cs250::WireRender::setPerspective(const glm::mat4& P)
{
    glUseProgram(program);
    glUniformMatrix4fv(upersp_matrix, 1, false, &P[0][0]);
}

void cs250::WireRender::setColor(const glm::vec4& color)
{
    glUseProgram(program);
    glUniform3f(ucolor, color.r, color.g, color.b);
}

void cs250::WireRender::draw(void)
{
    // draw sphere
    glUseProgram(program);
    // recall state
    glBindVertexArray(vao);
    glLineWidth(3.0f);
    glDrawElements(GL_LINES, 2 * edge_count, GL_UNSIGNED_INT, 0);
    // don't forget to stop state recording
    glBindVertexArray(0);
}
