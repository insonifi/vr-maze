#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <iostream>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix>
#include <drawable.h>
#include <aabb.h>
#include <box.h>

class Maze : public Drawable
{
public:
    Maze(unsigned short x = 16, unsigned short = 16);
    QVector3D getRandomPos() const;
    QVector3D collision(QVector3D position, QVector3D movement, BoundingBox observerBox);

private:
    std::vector<bool> _maze;
    unsigned short _width;
    unsigned short _height;
    std::vector<std::shared_ptr<Aabb>> _aabb_list;
    void initMaze();
    std::vector<bool>::reference mazeBlockAt(unsigned short x, unsigned short y);
    void addRandomLoop();
    void generate();
    void generateGeometry();
    void genFace(
            std::vector<QVector3D> *vertices,
            std::vector<QVector3D> *normals,
            std::vector<QVector2D> *texcoords,
            std::vector<unsigned short> *indices,
            QMatrix4x4 transform
            );
    void generateAabb();
    void printMaze();
signals:

public slots:
};

#endif // MAZE_H
