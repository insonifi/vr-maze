/*
 * Copyright (C) 2016, 2017 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef QVR_EXAMPLE_OPENGL_HPP
#define QVR_EXAMPLE_OPENGL_HPP

#include <QObject>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>
class QImage;

#include <qvr/app.hpp>
#include <aabb.h>
#include <drawable.h>
#include <maze.h>
#include <line.h>

class Main : public QObject, public QVRApp, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    Main();

private:
    /* Data not directly relevant for rendering */
    bool _wantExit;             // do we want to exit the app?
    QElapsedTimer _timer;       // used for animation purposes

    /* Static data for rendering. Here, these are OpenGL resources that are
     * initialized per process, so there is no need to serialize them for
     * multi-process rendering support. */
    unsigned int _fbo;                // Framebuffer object to render into
    unsigned int _fboDepthTex;        // Depth attachment for the FBO
    unsigned int _floorVao;           // Vertex array object for the floor
    unsigned int _floorIndices;       // Number of indices to render for the pl.
    Material     _floorMaterial;      // Material of the floor
    unsigned int _pillarVaos[2];      // Vertex array objects for the pillar
    unsigned int _pillarIndices[2];   // Number of indices to render for the pil.
    Material     _pillarMaterial;     // Material of the pillar
    unsigned int _objectVaos[5];      // Vertex array objects for the 5 objects
    unsigned int _objectIndices[5];   // Number of indices to render for the objs
    Material     _objectMaterials[5]; // Materials of the objects
    QMatrix4x4   _objectMatrices[5];  // Base transformation matrices of the objs
    QOpenGLShaderProgram _prg;        // GLSL program for rendering
    std::shared_ptr<Maze> _root;      // Scene root
    // Data to render device models
    QVector<unsigned int> _devModelVaos;
    QVector<unsigned int> _devModelVaoIndices;
    QVector<unsigned int> _devModelTextures;
    bool _mazeInited = false;         // Flag indicated whether we have already adjusted the maze position
    QPointF _mousePos = QPointF(0, 0);
    QQuaternion _orientation;
    float _moveZAxis = 0;               // Move forward/backward
	float _moveXAxis = 0;				// Move left/right
    std::shared_ptr<Aabb> _observerBox;// Box of the observer
    std::shared_ptr<Line> _line;
    std::vector<std::shared_ptr<Aabb>> _obstacles;
    bool _obstaclesAnimated = false;

    /* Dynamic data for rendering. This needs to be serialized for multi-process
     * rendering) */
    float _objectRotationAngle; // animated object rotation

    /* Helper function for texture loading */
    unsigned int setupTex(const QString& filename);
    unsigned int setupTex(const QImage& img);
    /* Helper function for VAO setup */
    unsigned int setupVao(int vertexCount,
            const float* positions, const float* normals, const float* texcoords,
            int indexCount, const unsigned short* indices);
    /* Helper function to set materials */
    void setMaterial(const Material& m);
    /* Helper function for GL VAO rendering */
    void renderVao(const QMatrix4x4& projectionMatrix,
            const QMatrix4x4& viewMatrix, const QMatrix4x4& modelMatrix,
            unsigned int vao, unsigned int indices);

    QVector3D collisionAdjust(QVector3D position, QVector3D movement, float size = 0.5f);
    void animateObstacles(QVector3D transform);

public:
    void serializeDynamicData(QDataStream& ds) const override;
    void deserializeDynamicData(QDataStream& ds) override;

    void update(const QList<QVRObserver*>& observers) override;

    bool wantExit() override;

    bool initProcess(QVRProcess* p) override;

    void render(QVRWindow* w, const QVRRenderContext& c, const unsigned int* textures) override;

    void keyPressEvent(const QVRRenderContext& context, QKeyEvent* event) override;

	void keyReleaseEvent(const QVRRenderContext &, QKeyEvent * event);
	void deviceAnalogChangeEvent(QVRDeviceEvent * event) override;
    void mouseMoveEvent(const QVRRenderContext &context, QMouseEvent *event) override;
    void mousePressEvent(const QVRRenderContext &context, QMouseEvent *event) override;
    void mouseReleaseEvent(const QVRRenderContext &context, QMouseEvent *event) override;
public slots:
    void buttonHit();
    void drop();
    void reachedGoal();
    void stop();
};

#endif
