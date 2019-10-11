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
    void update(QMatrix4x4 m, float elapsedMilli);
    void setGlobalTransform(QMatrix4x4 m);
    void setLocalTransform(QMatrix4x4 m);
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
    void loadShader(const char* vertShaderPath, const char* fragShaderPath);
    unsigned int loadTexture(const QString& filename);
    unsigned int loadTexture(const QImage& img);
    static QString readFile(const char* fileName);
    static void setGLES(bool isGLES);
    static bool getGLES();
    static bool isGLES;
    void move(QVector3D offset);
    QMatrix4x4 getModelMatrix() const;

private:
    virtual void glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix);

    QOpenGLShaderProgram _prg;
    std::vector<std::shared_ptr<Drawable>> _children;
    std::string _name;
    Material _material;
    QMatrix4x4 _globalTransform = QMatrix();
    QMatrix4x4 _localTransform = QMatrix();
    float _a = 0.f;
    GLsizei _elementsCount;
    QVector3D _offset = QVector3D();
    unsigned int _vao;
};

#endif // DRAWABLE_H
