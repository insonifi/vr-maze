#ifndef BOX_H
#define BOX_H

#include <vector>
#include "drawable.h"
#include <QVector3D>
#include <QMatrix4x4>

/**
 * @brief The Box class describes the absolute path a planet follows
 */
class Box : public Drawable
{
public:
    /**
     * @brief Box constructor
     * @param name the name of the path
     */
    Box(std::string name = "UNKNOWN BOX",
         std::vector<QVector3D> box = {
            QVector3D(0.f, 0.f, 0.f)
            , QVector3D(0.f, 0.f, 0.f)
            }
            , QVector3D color = QVector3D(1.f, 1.f, 1.f));

    virtual void glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix) override;
    void setLines(bool lines);

private:
    virtual void initBuffers();
    std::vector<QVector3D> _box;
    QVector3D _color;
    bool _drawLines = true;
    GLsizei _vertexCount;
};

#endif // BOX_H
