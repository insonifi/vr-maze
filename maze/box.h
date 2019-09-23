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
            });

    virtual void glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix) override;

private:
    virtual void init();
    std::vector<QVector3D> _box;
    GLsizei _vertexCount;
};

#endif // BOX_H
