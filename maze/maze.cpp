#include "maze.h"

Maze::Maze(unsigned short width, unsigned short height) :
    Drawable("Maze"), _width(width), _height(height)
{
    initMaze();
    Drawable::loadShader(
                ":vertex-shader.glsl"
                , ":fragment-shader_dbg.glsl"
                );
    Drawable::setMaterial(
                Material(0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
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

    _maze.assign(static_cast<unsigned short> (_width * _height), false);
    generate();
    generateGeometry();
    printMaze();
    // generateAabb();
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
                _aabb_list.push_back(
                            Aabb(QVector3D(front.at(0) - 0.5f, -0.5f, front.at(1) - 0.5f), QVector3D((x - 1) + 0.5f, 0.5f, y + 0.5f))
                            );
                front.clear();
            }
            if (!block && bk && back.size() == 0)
            {
                back = {x, y};
            }
            if ((block || (!block && !bk)) && back.size() > 0)
            {
                _aabb_list.push_back(
                            Aabb(QVector3D(back.at(0) - 0.5f, -0.5f, back.at(1) - 0.5f), QVector3D((x - 1) + 0.5f, 0.5f, y + 0.5f))
                            );
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
                _aabb_list.push_back(
                            Aabb(QVector3D(front.at(0) - 0.5f, -0.5f, front.at(1) - 0.5f), QVector3D(x + 0.5f, 0.5f, (y - 1) + 0.5f))
                            );
                front.clear();
            }
            if (!block && bk && back.size() == 0)
            {
                back = {x, y};
            }
            if ((block || (!block && !bk)) && back.size() > 0)
            {
                _aabb_list.push_back(
                            Aabb(QVector3D(back.at(0) - 0.5f, -0.5f, back.at(1) - 0.5f), QVector3D(x + 0.5f, 0.5f, (y - 1) + 0.5f))
                            );
                back.clear();
            }
        }
    }

    /** Outer Wall */// TODO: other sides
    _aabb_list.push_back(
                Aabb(QVector3D(-1 - 0.5f, -0.5f, 0 - 0.5f), QVector3D(- 0.5f, 0.5f, _height + 0.5f))
                );

    /** Floor */
    _aabb_list.push_back(
                Aabb(QVector3D(0 - 0.5f, -2.f, 0 - 0.5f), QVector3D(_width + 0.5f, -0.5f, _height + 0.5f))
                );

    /** Ceiling */
    // _aabb_list.push_back(
    //             Aabb(QVector3D(0 - 0.5f, 0.5f, 0 - 0.5f), QVector3D(_width + 0.5f, 2.0f, _height + 0.5f))
    //             );

    // for (Aabb aabb : _aabb_list)
    //     addChild(std::make_shared<Box>("box", aabb.getAB()));
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
                t0.translate(QVector3D(x, 0.f, y));

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
    return _maze.at(y * _width + x);
}
