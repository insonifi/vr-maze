#include <iostream>

#include "box.h"

Box::Box(std::string name, std::vector<QVector3D> box):
    Drawable(name), _box(box)
{
    init();
}

void Box::init()
{
    QVector3D k = _box.at(0);
    QVector3D l = _box.at(1);

    QVector3D p1 = QVector3D(k.x(), k.y(), k.z());
    QVector3D p2 = QVector3D(k.x(), k.y(), l.z());
    QVector3D p3 = QVector3D(l.x(), k.y(), l.z());
    QVector3D p4 = QVector3D(l.x(), k.y(), k.z());

    QVector3D p5 = QVector3D(k.x(), l.y(), k.z());
    QVector3D p6 = QVector3D(k.x(), l.y(), l.z());
    QVector3D p7 = QVector3D(l.x(), l.y(), l.z());
    QVector3D p8 = QVector3D(l.x(), l.y(), k.z());

    std::vector<QVector3D> vertices = {
        p1, p2
        , p2, p3
        , p3, p4
        , p4, p1

        , p5, p6
        , p6, p7
        , p7, p8
        , p8, p5

        , p1, p5
        , p2, p6
        , p3, p7
        , p4, p8
    };

    GLuint& vao = getVao();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vertexBuf;
    glGenBuffers(1, &vertexBuf);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr (vertices.size() * sizeof(QVector3D)), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,// attribute position in buffer
        3,// size (number of items per vertex)
        GL_FLOAT,// data type
        GL_FALSE,// is normalised?
        0,// stride
        nullptr// array buffer offset
    );

    glBindVertexArray(0);
    glDeleteBuffers(1, &vertexBuf);

    _vertexCount = vertices.size();
}

void Box::glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix)
{
    QMatrix4x4 modelViewMatrix = vMatrix * getModelMatrix();
    QOpenGLShaderProgram& prg = getShader();
    prg.setUniformValue("model_view_matrix", modelViewMatrix);
    prg.setUniformValue("projection_model_view_matrix", pMatrix * modelViewMatrix);
    prg.setUniformValue("normal_matrix", modelViewMatrix.normalMatrix());
    glBindVertexArray(getVao());
    glDrawArrays(GL_LINES, 0, _vertexCount);
}
