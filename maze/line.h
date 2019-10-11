#ifndef LINE_H
#define LINE_H

#include <vector>
#include "drawable.h"
#include <QVector3D>
#include <QMatrix4x4>

/**
 * @brief The Line class describes the absolute path a planet follows
 */
class Line : public Drawable
{
public:
    /**
     * @brief Line constructor
     * @param name the name of the path
     */
    Line(std::string name = "UNKNOWN LINE",
         std::vector<QVector3D> line = {
            QVector3D(0.f, 0.f, 0.f)
            , QVector3D(0.f, 0.f, 0.f)
            }
            , QVector3D color = QVector3D(1.f, 1.f, 1.f));

    virtual void glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix) override;
    void setLine(std::vector<QVector3D> line);

private:
    virtual void initBuffers();
    std::vector<QVector3D> _line;
    QVector3D _color;
    GLsizei _vertexCount;
};
#endif // LINE_H
