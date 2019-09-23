#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <vector>
#include <memory>
#include <material.h>

class Drawable : protected QOpenGLExtraFunctions
{
public:
    Drawable(std::string name);
    void update(QMatrix4x4 modelMatrix, float elapsedMilli);
    void render(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix);
    void addChild(std::shared_ptr<Drawable> child);
    void setMaterial(const Material &material);
    void initBuffers(std::vector<QVector3D> *vertices
                     , std::vector<QVector3D> *normals
                     , std::vector<QVector2D> *texcoords
                     , std::vector<unsigned short> *indices);
    QOpenGLShaderProgram& getShader();
    GLuint& getVao();
    QMatrix4x4& getModelMatrix();

private:
    virtual void glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix);

    QOpenGLShaderProgram _prg;
    std::vector<std::shared_ptr<Drawable>> _children;
    std::string _name;
    Material _material;
    QMatrix4x4 _modelMatrix;
    GLsizei _elementsCount;
    unsigned int _vao;
};

#endif // DRAWABLE_H
