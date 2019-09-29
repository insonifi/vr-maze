#include "aabb.h"

Aabb::Aabb(QVector3D a, QVector3D b, bool renderable):
    Drawable("AABB"),
    _ab(BoundingBox(
            b.length() > a.length() ? a : b
          , b.length() > a.length() ? b : a
        ))
{
    if (renderable)
        addChild(std::make_shared<Box>("AABB", std::vector<QVector3D> {a, b}));
}

BoundingBox Aabb::getBox() const
{
   return _ab;
}

std::vector<QVector3D> Aabb::getAB() const
{
    return {_ab.a, _ab.b};
}

BVec Aabb::getOverlap(Aabb &aabb) const
{
    BoundingBox box0 = getBox();
    BoundingBox box1 = aabb.getBox();
    QVector3D a = box0.a * getModelMatrix();
    QVector3D b = box0.b * getModelMatrix();


    bool overlapX = b.x() > box1.a.x()
            && box1.b.x() > a.x();
    bool overlapY = b.y() > box1.a.y()
            && box1.b.y() > a.y();
    bool overlapZ = b.z() > box1.a.z()
            && box1.b.z() > a.z();

    return BVec(overlapX, overlapY, overlapZ);
}

Bound Aabb::boundsPoint(QVector3D point) const
{
    BoundingBox box = getBox();
    BoundAxis x = BoundAxis(point.x() >= box.a.x(), point.x() <= box.b.x());
    BoundAxis y = BoundAxis(point.y() >= box.a.y(), point.y() <= box.b.y());
    BoundAxis z = BoundAxis(point.z() >= box.a.z(), point.z() <= box.b.z());

    return Bound({x, y, z});
}

BVec Aabb::getContain(Aabb &aabb) const
{
    BoundingBox box = aabb.getBox();

    Bound a = boundsPoint(box.a * getModelMatrix());
    Bound b = boundsPoint(box.b * getModelMatrix());

    return BVec(
                  (a.x.top && a.x.bottom && b.x.top && b.x.bottom)
                , (a.y.top && a.y.bottom && b.y.top && b.y.bottom)
                , (a.z.top && a.z.bottom && b.z.top && b.z.bottom)
    );
}

bool Aabb::hasOverlap(Aabb &aabb) const
{
    return getOverlap(aabb).all();
}

void Aabb::glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix)
{
}
