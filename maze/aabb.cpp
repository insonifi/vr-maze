#include "aabb.h"

Aabb::Aabb(QVector3D a, QVector3D b): _ab(BoundingBox(a, b))
{
    // bool maxIsB = b.length() > a.length();
    // QVector3D min = maxIsB ? a : b;
    // QVector3D max = maxIsB ? b : a;
    // _ab = BoundingBox(min, max);
}

BoundingBox Aabb::getBox() const
{
   return _ab;
}

std::vector<QVector3D> Aabb::getAB() const
{
    return {_ab.a, _ab.b};
}

std::vector<bool> Aabb::getOverlap(Aabb aabb) const
{
    BoundingBox box0 = getBox();
    BoundingBox box1 = aabb.getBox();

    bool overlapX = box0.b.x() > box1.a.x()
            && box1.b.x() > box0.a.x();
    bool overlapY = box0.b.y() > box1.a.y()
            && box1.b.y() > box0.a.y();
    bool overlapZ = box0.b.z() > box1.a.z()
            && box1.b.z() > box0.a.z();

    return std::vector<bool> {overlapX, overlapY, overlapZ};
}

Bound Aabb::boundsPoint(QVector3D point) const
{
    BoundingBox box = getBox();
    BoundAxis x = BoundAxis(point.x() >= box.a.x(), point.x() <= box.b.x());
    BoundAxis y = BoundAxis(point.y() >= box.a.y(), point.y() <= box.b.y());
    BoundAxis z = BoundAxis(point.z() >= box.a.z(), point.z() <= box.b.z());

    return Bound({x, y, z});
}

std::vector<bool> Aabb::getContain(Aabb aabb) const
{
    BoundingBox box = aabb.getBox();

    Bound a = boundsPoint(box.a);
    Bound b = boundsPoint(box.b);

    return std::vector<bool> {
                  (a.x.top && a.x.bottom && b.x.top && b.x.bottom)
                , (a.y.top && a.y.bottom && b.y.top && b.y.bottom)
                , (a.z.top && a.z.bottom && b.z.top && b.z.bottom)
    };
}
