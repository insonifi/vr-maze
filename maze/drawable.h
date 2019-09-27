#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <QFile>
#include <QImage>
#include <vector>
#include <memory>
#include <iostream>
#include <material.h>

class Drawable : protected QOpenGLExtraFunctions
{
public:
    Drawable(std::string name);
    void update(QMatrix4x4 modelMatrix, float elapsedMilli);
    void render(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix);
    void addChild(std::shared_ptr<Drawable> child);
    void setMaterial(const Material material);
    virtual void initBuffers(  std::vector<QVector3D> *vertices
                     , std::vector<QVector3D> *normals
                     , std::vector<QVector2D> *texcoords
                     , std::vector<unsigned short> *indices);
    QOpenGLShaderProgram& getShader();
    GLuint getVao();
    void setVao(GLuint vao);
    QMatrix4x4& getModelMatrix();
    void loadShader(const char* vertShaderPath, const char* fragShaderPath);
    unsigned int loadTexture(const QString& filename);
    unsigned int loadTexture(const QImage& img);
    static QString readFile(const char* fileName);
    static void setGLES(bool isGLES);
    static bool getGLES();
    static bool isGLES;

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
