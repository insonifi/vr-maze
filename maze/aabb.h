#ifndef AABB_H
#define AABB_H

#include <vector>
#include <QOpenGLExtraFunctions>
#include <QVector3D>
#include <QMatrix4x4>
#include <box.h>
#include <bvec.hpp>

struct BoundingBox {
    QVector3D a;
    QVector3D b;
    BoundingBox(QVector3D a, QVector3D b):
        a(a), b(b)
    {}
};

struct BoundAxis {
    bool bottom;
    bool top;
    BoundAxis(bool bottom, bool top):
        bottom(bottom), top(top)
    {}
};

struct Bound {
    BoundAxis x;
    BoundAxis y;
    BoundAxis z;
    Bound(BoundAxis x, BoundAxis y, BoundAxis z):
        x(x), y(y), z(z)
    {}

};

class Aabb : public Drawable
{
public:
    Aabb(QVector3D a = QVector3D(0.f, 0.f, 0.f)
            , QVector3D b = QVector3D(0.f, 0.f, 0.f)
            , bool renderable = false
            );

    BVec virtual getOverlap(Aabb &aabb) const;
    bool virtual hasOverlap(Aabb &aabb) const;
    BVec virtual getContain(Aabb &aabb) const;
    Bound virtual boundsPoint(QVector3D point) const;
    BoundingBox getBox() const;
    std::vector<QVector3D> getAB() const;

private:
    virtual void glRender(QMatrix4x4 &vMarix, QMatrix4x4 &pMatrix);

    BoundingBox _ab;
};

#endif // AABB_H
