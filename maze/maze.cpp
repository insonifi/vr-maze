#include <bvec.hpp>
#include <maze.h>

Maze::Maze(unsigned short width, unsigned short height) :
    Drawable("Maze"), _width(width), _height(height)
{
    initMaze();
    Drawable::loadShader(
                ":vertex-shader.glsl"
                , ":fragment-shader.glsl"
                );
    Drawable::setMaterial(
                Material(0.5f, 0.5f, 0.5f, 1.0f, 0.2f, 0.1f,
                         loadTexture(":floor-diff.jpg")
                         , getGLES() ? 0 : loadTexture(":floor-norm.jpg"), 0, 10.0f
                         )
                );
}

void Maze::initMaze()
{
    std::cout << "initialise maze "
              << _width << "×" << _height
              << std::endl;

    _maze.assign(static_cast<unsigned short> (_width * _height + 1), false);
    generate();
    generateGeometry();
    printMaze();
    generateAabb();
}

void Maze::addRandomLoop()
{
    unsigned short xa = static_cast<unsigned short> (rand()) % _width / 2;
    unsigned short xb = static_cast<unsigned short> (rand()) % _width / 2 + xa;
    unsigned short ya = static_cast<unsigned short> (rand()) % _height / 2;
    unsigned short yb = static_cast<unsigned short> (rand()) % _height / 2 + ya;

    for (unsigned short x = xa; x <= xb; x++)
    {
        mazeBlockAt(x, ya) = true;
        mazeBlockAt(x, yb) = true;
    }
    for (unsigned short y = ya; y <= yb; y++)
    {
        mazeBlockAt(xa, y) = true;
        mazeBlockAt(xb, y) = true;
    }
}

void Maze::printMaze()
{
    for (unsigned short x = 0; x < _width; x++)
    {
        for (unsigned short y = 0; y < _height; y++)
            std::cout << (mazeBlockAt(x, y) ? "##" : "  ");
        std::cout << std::endl;
    }
}

void Maze::generate()
{
    int it = (_width + _height) / 6;

    for (int i = 0; i < it; i++)
        addRandomLoop();
}

void Maze::generateAabb()
{
    // Aabb(QVector3D(xa - 0.5f, -0.5f, ya - 0.5f), QVector3D(xa + 0.5f, 0.5f, yb + 0.5f));
    /** horizontal boxes */
    for (unsigned short y = 0; y < _height; y++)
    {
        std::vector<unsigned short> front;
        std::vector<unsigned short> back;

        for (unsigned short x = 0; x < _width; x++)
        {
            bool block = mazeBlockAt(x, y);
            bool fw = mazeBlockAt(x, y + 1);
            bool bk = mazeBlockAt(x, y - 1);

            if (!block && fw && front.size() == 0)
            {
                front = {x, y};
            }
            if ((block || (!block && !fw)) && front.size() > 0)
            {
                _aabb_list.push_back(std::make_shared<Aabb>(
                                         QVector3D(front.at(0) - 0.5f, -0.5f, front.at(1) - 0.5f)
                                         , QVector3D((x - 1) + 0.5f, 0.5f, y + 0.5f)
                                         , true
                                         ));
                front.clear();
            }
            if (!block && bk && back.size() == 0)
            {
                back = {x, y};
            }
            if ((block || (!block && !bk)) && back.size() > 0)
            {
                _aabb_list.push_back(std::make_shared<Aabb>(
                                         QVector3D(back.at(0) - 0.5f, -0.5f, back.at(1) - 0.5f)
                                         , QVector3D((x - 1) + 0.5f, 0.5f, y + 0.5f)
                                         , true
                                         ));
                back.clear();
            }
        }
    }

    /** vertical boxes */
    for (unsigned short x = 0; x < _width; x++)
    {
        std::vector<unsigned short> front;
        std::vector<unsigned short> back;

        for (unsigned short y = 0; y < _height; y++)
        {
            bool block = mazeBlockAt(x, y);
            bool fw = mazeBlockAt(x + 1, y);
            bool bk = mazeBlockAt(x - 1, y);

            if (!block && fw && front.size() == 0)
            {
                front = {x, y};
            }
            if ((block || (!block && !fw)) && front.size() > 0)
            {
                _aabb_list.push_back(std::make_shared<Aabb>(
                                         QVector3D(front.at(0) - 0.5f, -0.5f, front.at(1) - 0.5f)
                                         , QVector3D(x + 0.5f, 0.5f, (y - 1) + 0.5f)
                                         , true));
                front.clear();
            }
            if (!block && bk && back.size() == 0)
            {
                back = {x, y};
            }
            if ((block || (!block && !bk)) && back.size() > 0)
            {
                _aabb_list.push_back(std::make_shared<Aabb>(
                                         QVector3D(back.at(0) - 0.5f, -0.5f, back.at(1) - 0.5f)
                                         , QVector3D(x + 0.5f, 0.5f, (y - 1) + 0.5f)
                                         , true));
                back.clear();
            }
        }
    }

    /** Outer Wall */// TODO: other sides
    _aabb_list.push_back(std::make_shared<Aabb>(
                             QVector3D(-1 - 0.5f, -0.5f, 0 - 0.5f)
                             , QVector3D(- 0.5f, 0.5f, _height + 0.5f)
                             , true));

    /** Floor */
    _aabb_list.push_back(std::make_shared<Aabb>(
                             QVector3D(0 - 0.5f, -2.f, 0 - 0.5f)
                             , QVector3D(_width + 0.5f, -0.5f, _height + 0.5f)
                             , true));

    for (std::shared_ptr<Aabb> aabb : _aabb_list)
        addChild(aabb);
}

void Maze::genFace(
        std::vector<QVector3D> *vertices,
        std::vector<QVector3D> *normals,
        std::vector<QVector2D> *texcoords,
        std::vector<unsigned short> *indices,
        QMatrix4x4 transform
        )
{

    // 1   2
    //   c(x, y)
    // 4   3

    QVector3D a = (transform * QVector3D(+ 0.5f, - 0.5, - 0.5f));
    QVector3D b = (transform * QVector3D(+ 0.5f, - 0.5, + 0.5f));
    QVector3D c = (transform * QVector3D(- 0.5f, - 0.5, + 0.5f));
    QVector3D d = (transform * QVector3D(- 0.5f, - 0.5, - 0.5f));

    QVector2D ta = QVector2D(1.0f, 0.f);
    QVector2D tb = QVector2D(1.0f, 1.f);
    QVector2D tc = QVector2D(0.0f, 1.f);
    QVector2D td = QVector2D(0.0f, 0.f);

    QVector3D normal = (transform * QVector3D(0.f, 1.f, 0.f));

    normal.normalize();

    vertices->push_back(a);// 1
    vertices->push_back(b);// 2
    vertices->push_back(c);// 3
    vertices->push_back(d);// 4
    normals->push_back(normal);
    normals->push_back(normal);
    normals->push_back(normal);
    normals->push_back(normal);
    texcoords->push_back(ta);
    texcoords->push_back(tb);
    texcoords->push_back(tc);
    texcoords->push_back(td);

    unsigned short idx = static_cast<unsigned short> (vertices->size() - 1);

    indices->push_back(idx - 1);
    indices->push_back(idx - 2);
    indices->push_back(idx - 3);

    indices->push_back(idx - 1);
    indices->push_back(idx - 3);
    indices->push_back(idx - 0);
}

void Maze::generateGeometry() {

    std::vector<QVector3D> vertices;
    std::vector<QVector3D> normals;
    std::vector<QVector2D> texcoords;
    std::vector<unsigned short> indices;

    for (unsigned short x = 0; x < _width; x++)
        for (unsigned short y = 0; y < _height; y++)
            if (mazeBlockAt(x, y))
            {
                /** Floor  **/
                QMatrix4x4 t0 = QMatrix4x4();
                t0.translate(QVector3D(x, 0, y));

                genFace(&vertices, &normals, &texcoords, &indices, t0);

                 /**  Walls **/
                if (y + 1 > _height || !mazeBlockAt(x, y + 1))
                {
                    QMatrix4x4 t = QMatrix4x4(t0);
                    t.rotate(-90.f, QVector3D(1, 0, 0));

                    genFace(&vertices, &normals, &texcoords, &indices, t);
                }
                if (int(y) - 1 < 0 || !mazeBlockAt(x, y - 1))
                {
                    QMatrix4x4 t = QMatrix4x4(t0);
                    t.rotate(90.0f, QVector3D(1, 0, 0));

                    genFace(&vertices, &normals, &texcoords, &indices, t);
                }
                if (x + 1 > _width || !mazeBlockAt(x + 1, y))
                {
                    QMatrix4x4 t = QMatrix4x4(t0);
                    t.rotate(90.0f, QVector3D(0, 0, 1));

                    genFace(&vertices, &normals, &texcoords, &indices, t);
                }
                if (int(x) - 1 < 0 || !mazeBlockAt(x - 1, y))
                {
                    QMatrix4x4 t = QMatrix4x4(t0);
                    t.rotate(-90.0f, QVector3D(0, 0, 1));

                    genFace(&vertices, &normals, &texcoords, &indices, t);
                }

                // {
                //     QMatrix4x4 t = QMatrix4x4(t0);
                //     t.rotate(-180.0f, QVector3D(0, 1, 0));

                //	genFace(&vertices, &normals, &texcoords, &indices, t);
                // }
            }
    Drawable::initBuffers(&vertices, &normals, &texcoords, &indices);
}

std::vector<bool>::reference Maze::mazeBlockAt(unsigned short x, unsigned short y)
{
    unsigned short idx = y * _width + x;

    if (idx < 0 || idx >= _maze.size())
        return _maze.at(_maze.size() - 1);

    return _maze.at(idx);
}

QVector3D Maze::getRandomPos() const
{
   unsigned int idx = 0;

   for (;_maze.at(idx) != true; idx = static_cast<unsigned int>(rand()) % _maze.size())
   {
   }

   QVector3D position = QVector3D(idx % _width, 0, idx / _width);

   std::cout << "Random position: "
             << position.x() << ", "
             << position.y() << ", "
             << position.x() << std::endl;


   return position;
}

QVector3D Maze::collision(QVector3D position, QVector3D _movement, float _size)
{
    QVector3D shift = QVector3D(_movement.x(), _movement.y(), _movement.z());
    //glm::vec3 shift = movement;
    QVector3D size = QVector3D(_size, _size, _size);
    QVector3D position_f = position + shift;
    std::shared_ptr<Aabb> pos0_aabb =
            std::make_shared<Aabb>(position - size, position + size);
    std::shared_ptr<Aabb> pos1_aabb =
            std::make_shared<Aabb>(position_f - size, position_f + size);

    bool collides = false;
    std::vector<std::shared_ptr<Aabb>> aabb_collided;

    for (std::shared_ptr<Aabb> box : _aabb_list)
    {
        bool overlaps = box->hasOverlap(*pos1_aabb);

        collides |= overlaps;

        if (overlaps)
            aabb_collided.push_back(box);
    }

    if (!collides)
        return position + shift;

    std::cout << "collision" << std::endl;
    BVec overlap = BVec(true);

    for (std::shared_ptr<Aabb> box : aabb_collided)
    {
        BVec b = box->getOverlap(*pos0_aabb);
        overlap &= b;
    }

    shift = QVector3D(
                  (overlap.x ? shift.x() : 0)
                , (overlap.y ? shift.y() : 0)
                , (overlap.z ? shift.z() : 0)
                );


    return position + shift;
}
