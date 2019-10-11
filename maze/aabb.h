#ifndef AABB_H
#define AABB_H

#include <vector>
#include <QOpenGLExtraFunctions>
#include <QVector3D>
#include <QMatrix4x4>
#include <QObject>
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

class Aabb : public QObject, public Drawable
{
    Q_OBJECT
public:
    Aabb(QVector3D a = QVector3D(0.f, 0.f, 0.f)
            , QVector3D b = QVector3D(0.f, 0.f, 0.f)
            , bool renderable = false
            , QVector3D color = QVector3D(1.f, 0, 1.f)
            , QString name = ""
            , QObject* _parent = nullptr
            );

    BVec virtual getOverlap(Aabb &aabb) const;
    bool virtual hasOverlap(Aabb &aabb) const;
    BVec virtual getContain(Aabb &aabb) const;
    Bound virtual boundsPoint(QVector3D point) const;
    BoundingBox getBox() const;
    std::vector<QVector3D> getAB() const;
    bool isCollided() const;
    virtual void setCollided(bool collided);
    QString getName() const;
    bool isObstacle() const;

private:
    virtual void glRender(QMatrix4x4 &vMarix, QMatrix4x4 &pMatrix);

    BoundingBox _ab;
    std::shared_ptr<Box> _box;
    bool _collided = false;
    QString _name;

signals:
    void collided();
};

#endif // AABB_H
