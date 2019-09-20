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
    ~Drawable();
    void update(QMatrix4x4 modelMatrix);
    void render(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix);
    void addChild(std::shared_ptr<Drawable> child);
    void setMaterial(const Material &material);
    void initBuffers(std::vector<QVector3D> vertices
                     , std::vector<QVector3D> normals
                     , std::vector<QVector2D> texcoords
                     , std::vector<unsigned short> indices);
private:
    void init() const;
    void glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix);
    QOpenGLShaderProgram _prg;
    std::vector<std::shared_ptr<Drawable>> _children;
    std::string _name;
    Material _material;
    QMatrix4x4 _modelMatrix;
    std::vector<int> _indices;
    unsigned int _vao;
signals:

public slots:
};

#endif // DRAWABLE_H
