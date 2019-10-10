#include "aabb.h"

Aabb::Aabb(QVector3D a, QVector3D b, bool renderable):
    Drawable("AABB"),
    _ab(BoundingBox(
            b.lengthSquared() > a.lengthSquared() ? a : b
          , b.lengthSquared() > a.lengthSquared() ? b : a
        ))
{
    if (renderable)
    {
        _box = std::make_shared<Box>("AABB", std::vector<QVector3D> {a, b});
        addChild(_box);
    }
}

BoundingBox Aabb::getBox() const
{
   return BoundingBox(
               getModelMatrix() * _ab.a
               , getModelMatrix() * _ab.b
               );
}

std::vector<QVector3D> Aabb::getAB() const
{
    return {
         getModelMatrix() * _ab.a
        , getModelMatrix() * _ab.b
        };
}

BVec Aabb::getOverlap(Aabb &aabb) const
{
    BoundingBox box0 = getBox();
    BoundingBox box1 = aabb.getBox();
    QVector3D a = box0.a;
    QVector3D b = box0.b;


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

    Bound a = boundsPoint(box.a);
    Bound b = boundsPoint(box.b);

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

void Aabb::setCollided(bool collided)
{
    _collided = collided;

    if (_box)
        _box->setLines(!collided);
}

bool Aabb::isCollided() const
{
    return _collided;
}
