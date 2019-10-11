#include <iostream>

#include "line.h"

Line::Line(std::string name, std::vector<QVector3D> line, QVector3D color):
    Drawable(name), _color(color)
{
    setLine(line);
    Drawable::loadShader(
                ":vertex-shader.glsl"
                , ":fragment-shader_dbg.glsl"
                );
}

void Line::initBuffers()
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    GLuint vao = getVao();

    if (vao == 0)
        f->glGenVertexArrays(1, &vao);
    f->glBindVertexArray(vao);

    GLuint vertexBuf;
    f->glGenBuffers(1, &vertexBuf);
    f->glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    f->glBufferData(GL_ARRAY_BUFFER, GLsizeiptr (_line.size() * sizeof(QVector3D)), _line.data(), GL_DYNAMIC_DRAW);

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(
        0,// attribute position in buffer
        3,// size (number of items per vertex)
        GL_FLOAT,// data type
        GL_FALSE,// is normalised?
        0,// stride
        nullptr// array buffer offset
    );

    f->glBindVertexArray(0);
    f->glDeleteBuffers(1, &vertexBuf);

    _vertexCount = _line.size();
    setVao(vao);
}

void Line::glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix)
{
    initBuffers();

    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    QMatrix4x4 modelViewMatrix = vMatrix * getModelMatrix();
    QOpenGLShaderProgram& prg = getShader();

    prg.bind();

    prg.setUniformValue("color", _color.x(), _color.y(), _color.z());
    prg.setUniformValue("model_view_matrix", modelViewMatrix);
    prg.setUniformValue("projection_model_view_matrix", pMatrix * modelViewMatrix);
    prg.setUniformValue("normal_matrix", modelViewMatrix.normalMatrix());
    f->glBindVertexArray(getVao());
    f->glDrawArrays(GL_LINES, 0, _vertexCount);

    prg.release();
}

void Line::setLine(std::vector<QVector3D> line)
{
    _line = line;
}
