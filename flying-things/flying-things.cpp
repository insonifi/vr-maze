/*
 * Copyright (C) 2016, 2017, 2018 Computer Graphics Group, University of Siegen
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

#include <QApplication>
#include <QKeyEvent>

#include <qvr/manager.hpp>

#include "flying-things.hpp"
#include "geometries.hpp"


FlyingThings::FlyingThings() :
    _wantExit(false),
    _pause(false),
    _elapsedTime(0),
    _ringRotationAngle(0.0f),
    _objectRotationAngle(0.0f),
    _objects(400),
    _objectLOD(7),
    _objectType(3),
    _wireframe(false),
    _frustumCulling(false),
    _backfaceCulling(false),
    _distanceLOD(false)
{
    _timer.start();
}

bool FlyingThings::initProcess(QVRProcess* /* p */)
{
    // Qt-based OpenGL function pointers
    initializeOpenGLFunctions();

    // Framebuffer object
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glGenTextures(1, &_fboDepthTex);
    glBindTexture(GL_TEXTURE_2D, _fboDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1, 1,
            0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _fboDepthTex, 0);

    // VAOs and associated buffers
    for (int i = 0; i < 4; i++) {
        glGenVertexArrays(18, _vaos[i]);
        for (int l = 0; l < 18; l++) {
            int objectLOD = 8 + l * 2;
            std::vector<float> positions;
            std::vector<float> normals;
            std::vector<float> texcoords;
            std::vector<unsigned int> indices;
            if (i == 0)
                geom_sphere(positions, normals, texcoords, indices, objectLOD, objectLOD / 2);
            else if (i == 1)
                geom_cylinder(positions, normals, texcoords, indices, objectLOD);
            else if (i == 2)
                geom_cone(positions, normals, texcoords, indices, objectLOD, objectLOD / 2);
            else
                geom_torus(positions, normals, texcoords, indices, 0.4f, objectLOD, objectLOD);
            glBindVertexArray(_vaos[i][l]);
            GLuint positionBuf;
            glGenBuffers(1, &positionBuf);
            glBindBuffer(GL_ARRAY_BUFFER, positionBuf);
            glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
            GLuint normalBuf;
            glGenBuffers(1, &normalBuf);
            glBindBuffer(GL_ARRAY_BUFFER, normalBuf);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(1);
            GLuint indexBuf;
            glGenBuffers(1, &indexBuf);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            _vaoIndices[i][l] = indices.size();
        }
    }

    // Shader program
    _prg.addShaderFromSourceFile(QOpenGLShader::Vertex, ":vertex-shader.glsl");
    _prg.addShaderFromSourceFile(QOpenGLShader::Fragment, ":fragment-shader.glsl");
    _prg.link();

    return true;
}

static float frand() // return random number in [0,1]
{
    return qrand() / static_cast<float>(RAND_MAX);
}

void FlyingThings::render(QVRWindow* /* w */,
        const QVRRenderContext& context,
        const unsigned int* textures)
{
    // Initialize random number generator to fixed value so that all processes
    // will generate the same pseudo random number sequence for all frames.
    qsrand(42);

    for (int view = 0; view < context.viewCount(); view++) {
        // Get view dimensions
        int width = context.textureSize(view).width();
        int height = context.textureSize(view).height();
        // Set up framebuffer object to render into
        glBindTexture(GL_TEXTURE_2D, _fboDepthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height,
                0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[view], 0);
        // Set up view
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        QMatrix4x4 projectionMatrix = context.frustum(view).toMatrix4x4();
        QMatrix4x4 viewMatrix = context.viewMatrix(view);
        // Set up shader program
        glUseProgram(_prg.programId());
        _prg.setUniformValue("projection_matrix", projectionMatrix);
        glEnable(GL_DEPTH_TEST);
        if (_backfaceCulling)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);

        // TODO: Compute normals of view frustum for frustum culling
        const QVRFrustum& frustum = context.frustum(view);
        QVector3D nTop = QVector3D(0.0f, frustum.nearPlane(), frustum.topPlane()).normalized();
        QVector3D nBottom = QVector3D(0.0f, 0.0f, 0.0f); // TODO
        QVector3D nRight = QVector3D(0.0f, 0.0f, 0.0f); // TODO
        QVector3D nLeft = QVector3D(0.0f, 0.0f, 0.0f); // TODO

        // Render
        for (int i = 0; i < _objects; i++) {
            // Compute model matrix
            QMatrix4x4 modelMatrix;
            modelMatrix.translate(-2.0f, 1.6f, 0.0f);
            modelMatrix.rotate(_ringRotationAngle + frand() * 360.0f, 0.0f, -1.0f, 0.0f);
            modelMatrix.translate(2.5f, 0.0f, 0.0f);
            modelMatrix.rotate(frand() * 360.0f, 0.0f, 0.0f, 1.0f);
            modelMatrix.translate(0.5f - frand(), 0.0f, 0.0f);
            modelMatrix.rotate(_objectRotationAngle + frand() * 360.0f, QVector3D(frand(), frand(), frand()));
            modelMatrix.scale(0.1f);
            // Computer Bounding Sphere IN VIEW COORDINATES
            QVector3D boundingSphereCenter = (viewMatrix * modelMatrix).column(3).toVector3D();
            float boundingSphereRadius = 0.1f;
            // TODO: Apply culling
            bool cull = false;
            if (_frustumCulling) {
                if (boundingSphereCenter.z() > (-frustum.nearPlane() + boundingSphereRadius)) {
                    // near plane
                    cull = true;
                } else if (false /* TODO */) {
                    // far plane
                    cull = true;
                } else if (false /* TODO */) {
                    // top plane
                    cull = true;
                } else if (false /* TODO */) {
                    // bottom plane
                    cull = true;
                } else if (false /* TODO */) {
                    // right plane
                    cull = true;
                } else if (false /* TODO */) {
                    // left plane
                    cull = true;
                }
            }
            // TODO: Apply LOD
            int lod = _objectLOD;
            if (_distanceLOD) {
                float distance = boundingSphereCenter.length();
                // TODO: lod = ...;
            }
            // Determine object type
            int type = _objectType;
            if (type == 4)
                type = i % 4;
            // Set color
            QVector3D color = QVector3D(0.2f + 0.8f * frand(), 0.2f + 0.8f * frand(), 0.2f + 0.8f * frand());
            _prg.setUniformValue("color", color);
            // Render
            if (!cull) {
                QMatrix4x4 modelViewMatrix = viewMatrix * modelMatrix;
                _prg.setUniformValue("modelview_matrix", modelViewMatrix);
                _prg.setUniformValue("normal_matrix", modelViewMatrix.normalMatrix());
                glBindVertexArray(_vaos[type][lod]);
                glDrawElements(GL_TRIANGLES, _vaoIndices[type][lod], GL_UNSIGNED_INT, 0);
            }
        }
    }
}

void FlyingThings::update(const QList<QVRObserver*>&)
{
    if (_pause) {
        if (_timer.isValid()) {
            _elapsedTime += _timer.elapsed();
            _timer.invalidate();
        }
    } else {
        if (!_timer.isValid()) {
            _timer.start();
        }
        float seconds = (_elapsedTime + _timer.elapsed()) / 1000.0f;
        _ringRotationAngle = seconds * 20.0f;
        _objectRotationAngle = seconds * 10.0f;
    }
}

bool FlyingThings::wantExit()
{
    return _wantExit;
}

void FlyingThings::serializeDynamicData(QDataStream& ds) const
{
    ds << _ringRotationAngle << _objectRotationAngle
        << _objects << _objectType
        << _wireframe << _frustumCulling << _backfaceCulling << _distanceLOD;
}

void FlyingThings::deserializeDynamicData(QDataStream& ds)
{
    ds >> _ringRotationAngle >> _objectRotationAngle
        >> _objects >> _objectType
        >> _wireframe >> _frustumCulling >> _backfaceCulling >> _distanceLOD;
}

void FlyingThings::keyPressEvent(const QVRRenderContext& /* context */, QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        _wantExit = true;
        break;
    case Qt::Key_Space:
        _pause = !_pause;
        break;
    case Qt::Key_Plus:
        if (_objects < 5120)
            _objects *= 2;
        break;
    case Qt::Key_Minus:
        if (_objects > 1)
            _objects /= 2;
        break;
    case Qt::Key_Greater:
        if (_objectLOD < 17)
            _objectLOD++;
        break;
    case Qt::Key_Less:
        if (_objectLOD > 0)
            _objectLOD--;
        break;
    case Qt::Key_T:
        _objectType++;
        if (_objectType > 4)
            _objectType = 0;
        break;
    case Qt::Key_P:
        _wireframe = !_wireframe;
        break;
    case Qt::Key_F:
        _frustumCulling = !_frustumCulling;
        break;
    case Qt::Key_B:
        _backfaceCulling = !_backfaceCulling;
        break;
    case Qt::Key_L:
        _distanceLOD = !_distanceLOD;
        break;
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QVRManager manager(argc, argv);

    /* First set the default surface format that all windows will use */
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);
    QSurfaceFormat::setDefaultFormat(format);

    /* Then start QVR with the app */
    FlyingThings qvrapp;
    if (!manager.init(&qvrapp)) {
        qCritical("Cannot initialize QVR manager");
        return 1;
    }

    /* Enter the standard Qt loop */
    return app.exec();
}
