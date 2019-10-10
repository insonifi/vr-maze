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

#include <Windows.h>

#define MAZE 1
#define CUSTOM_NAV true
#define WALK_SPEED 2.f
#define SIZE 0.2f

#include <QGuiApplication>
#include <QKeyEvent>
#include <QImage>

#include <qvr/observer.hpp>
#include <qvr/manager.hpp>
#include <qvr/window.hpp>
#include <qvr/device.hpp>

#include "main.hpp"

#include "geometries.hpp"


static bool isGLES = false; // is this OpenGL ES or plain OpenGL?

Main::Main() :
    _wantExit(false),
    _objectRotationAngle(0.0f)
{
    _timer.start();
}

unsigned int Main::setupTex(const QString& filename)
{
    QImage img;
    img.load(filename);
    if (isGLES)
        img = img.scaledToWidth(img.width() / 2, Qt::SmoothTransformation);
    img = img.mirrored(false, true);
    img = img.convertToFormat(QImage::Format_RGBA8888);
    return setupTex(img);
}

unsigned int Main::setupTex(const QImage& img)
{
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
            img.width(), img.height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (!isGLES)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
    return tex;
}

unsigned int Main::setupVao(int vertexCount,
        const float* positions, const float* normals, const float* texcoords,
        int indexCount, const unsigned short* indices)
{
    GLuint vao;
    GLuint positionBuf, normalBuf, texcoordBuf, indexBuf;
    std::vector<QVector3D> v, n;
    std::vector<QVector2D> t;

    for (int i = 0; i < vertexCount; i++)
    {
        v.push_back(QVector3D(
                        positions[i * 3 + 0]
                        , positions[i * 3 + 1]
                        , positions[i * 3 + 2]
                    ));
        n.push_back(QVector3D(
                        normals[i * 3 + 0]
                        , normals[i * 3 + 1]
                        , normals[i * 3 + 2]
                    ));
        t.push_back(QVector2D(
                        texcoords[i * 2 + 0]
                        , texcoords[i * 2 + 1]
                    ));
    }


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &positionBuf);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuf);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(QVector3D), v.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &normalBuf);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuf);
    glBufferData(GL_ARRAY_BUFFER, n.size() * sizeof(QVector3D), n.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &texcoordBuf);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuf);
    glBufferData(GL_ARRAY_BUFFER, t.size() * sizeof (QVector2D), t.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glGenBuffers(1, &indexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned short), indices, GL_STATIC_DRAW);
    return vao;
}

void Main::setMaterial(const Material& m)
{
    _prg.setUniformValue("material_color", m.r, m.g, m.b);
    _prg.setUniformValue("material_kd", m.kd);
    _prg.setUniformValue("material_ks", m.ks);
    _prg.setUniformValue("material_shininess", m.shininess);
    _prg.setUniformValue("material_has_diff_tex", m.diffTex == 0 ? 0 : 1);
    _prg.setUniformValue("material_diff_tex", 0);
    _prg.setUniformValue("material_has_norm_tex", m.normTex == 0 ? 0 : 1);
    _prg.setUniformValue("material_norm_tex", 1);
    _prg.setUniformValue("material_has_spec_tex", m.specTex == 0 ? 0 : 1);
    _prg.setUniformValue("material_spec_tex", 2);
    _prg.setUniformValue("material_tex_coord_factor", m.texCoordFactor);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m.diffTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m.normTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m.specTex);
}

void Main::renderVao(const QMatrix4x4& projectionMatrix,
        const QMatrix4x4& viewMatrix, const QMatrix4x4& modelMatrix,
        unsigned int vao, unsigned int indices)
{
    QMatrix4x4 modelViewMatrix = viewMatrix * modelMatrix;
    _prg.setUniformValue("model_view_matrix", modelViewMatrix);
    _prg.setUniformValue("projection_model_view_matrix", projectionMatrix * modelViewMatrix);
    _prg.setUniformValue("normal_matrix", modelViewMatrix.normalMatrix());
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_SHORT, nullptr);
}

void Main::serializeDynamicData(QDataStream& ds) const
{
    ds << _objectRotationAngle;
}

void Main::deserializeDynamicData(QDataStream& ds)
{
    ds >> _objectRotationAngle;
}

void Main::update(const QList<QVRObserver*>& observerList)
{
    float seconds = _timer.elapsed() / 1000.0f;
    _timer.restart();

    // Trigger a haptic pulse on devices that support it
    for (int i = 0; i < QVRManager::deviceCount(); i++) {
        const QVRDevice& device = QVRManager::device(i);

        if (device.supportsHapticPulse()
                && device.hasAnalog(QVR_Analog_Trigger)
                && device.analogValue(QVR_Analog_Trigger) > 0.0f) {
            int microseconds = device.analogValue(QVR_Analog_Trigger) * 3999;
            device.triggerHapticPulse(microseconds);
        }
    }

    QVRObserver* observer = observerList.first();
    QVector3D positionTracking = observer->trackingPosition();

    /** Initial observer placement and maze position adjustment */
    if (!_mazeInited)
    {
        QMatrix4x4 initMazeTransform = QMatrix4x4();

        initMazeTransform.translate(positionTracking);
        initMazeTransform.translate(-_root->getRandomPos());
        _root->update(initMazeTransform, 0);
        _mazeInited = true;
    }

    QVector3D position = observer->navigationPosition();

    QMatrix4x4 translation = QMatrix4x4();
    translation.translate(position);
    QMatrix4x4 observerTransform = translation;

    observerTransform.translate(positionTracking);
    _observerBox->update(observerTransform, seconds);

    position = _root->collision(
                position
                , _orientation.rotatedVector(QVector3D(WALK_SPEED * _moveXAxis * seconds, 0, WALK_SPEED * _moveZAxis * seconds))
                , _observerBox->getBox()
                );

    observer->setNavigation(position , _orientation);
}

bool Main::wantExit()
{
    return _wantExit;
}

// Helper function: read a complete file into a QString (without error checking)
static QString readFile(const char* fileName)
{
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    return in.readAll();
}

#if(MAZE)
bool Main::initProcess(QVRProcess* /* p */)
{
    // Qt-based OpenGL function pointers
    initializeOpenGLFunctions();

    // FBO
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glGenTextures(1, &_fboDepthTex);
    glBindTexture(GL_TEXTURE_2D, _fboDepthTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1,
            0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _fboDepthTex, 0);

     // Device model data
     for (int i = 0; i < QVRManager::deviceModelVertexDataCount(); i++) {
         _devModelVaos.append(setupVao(
                     QVRManager::deviceModelVertexCount(i),
                     QVRManager::deviceModelVertexPositions(i),
                     QVRManager::deviceModelVertexNormals(i),
                     QVRManager::deviceModelVertexTexCoords(i),
                     QVRManager::deviceModelVertexIndexCount(i),
                     QVRManager::deviceModelVertexIndices(i)));
         _devModelVaoIndices.append(QVRManager::deviceModelVertexIndexCount(i));
     }
     for (int i = 0; i < QVRManager::deviceModelTextureCount(); i++) {
         _devModelTextures.append(setupTex(QVRManager::deviceModelTexture(i)));
     }

     std::shared_ptr<Maze> maze = std::make_shared<Maze>(16, 16);


     for (unsigned short i = 0; i < 10; i++)
     {
         QVector3D pos = maze->getRandomPos();
         std::shared_ptr<Aabb> obstacle = std::make_shared<Aabb>(
                     pos - QVector3D(0.2f, 0, 0.2f)
                     , pos + QVector3D(0.2f, 1.0f, 0.5f)
                     , true);
         maze->addObstacle(obstacle);
     }


     // maze->addChild(
     //            std::make_shared<Box>(
     //                "box"
     //                , std::vector<QVector3D>({QVector3D(0, 0, 0), QVector3D(1, 1, 1)})
     //                )
     //            );

    _root = maze;
    _observerBox = std::make_shared<Aabb> (
                QVector3D(-SIZE, -SIZE, -SIZE)
                , QVector3D(SIZE, SIZE, SIZE)
                , true
                );

   return true;
}

void Main::render(QVRWindow* /* w */,
        const QVRRenderContext& context, const unsigned int* textures)
{
    for (int view = 0; view < context.viewCount(); view++) {
        // Get view dimensions
        int width = context.textureSize(view).width();
        int height = context.textureSize(view).height();
        // Set up framebuffer object to render into
        glBindTexture(GL_TEXTURE_2D, _fboDepthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height,
                0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[view], 0);
        // Set up view
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        QMatrix4x4 projectionMatrix = context.frustum(view).toMatrix4x4();
        QMatrix4x4 viewMatrix = context.viewMatrix(view);
        // Set up shader program
        glEnable(GL_DEPTH_TEST);
        // Render scene

        _observerBox->render(viewMatrix, projectionMatrix);
        _root->render(viewMatrix, projectionMatrix);

        // Render device models (optional)
        for (int i = 0; i < QVRManager::deviceCount(); i++) {
            const QVRDevice& device = QVRManager::device(i);
            for (int j = 0; j < device.modelNodeCount(); j++) {
                QMatrix4x4 nodeMatrix = device.matrix();
                nodeMatrix.translate(device.modelNodePosition(j));
                nodeMatrix.rotate(device.modelNodeOrientation(j));
                int vertexDataIndex = device.modelNodeVertexDataIndex(j);
                int textureIndex = device.modelNodeTextureIndex(j);
                Material material(1.0f, 1.0f, 1.0f,
                        1.0f, 0.0f, 0.0f,
                        _devModelTextures[textureIndex], 0, 0,
                        1.0f);
                setMaterial(material);
                renderVao(projectionMatrix, context.viewMatrixPure(view), nodeMatrix,
                        _devModelVaos[vertexDataIndex],
                        _devModelVaoIndices[vertexDataIndex]);
            }
        }
        // Invalidate depth attachment (to help OpenGL ES performance)
        const GLenum fboInvalidations[] = { GL_DEPTH_ATTACHMENT };
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, fboInvalidations);
    }
}
#endif

void Main::keyPressEvent(const QVRRenderContext& /* context */, QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        _wantExit = true;
        break;
	case Qt::Key_W:
		_moveZAxis = -1;
		break;
	case Qt::Key_S:
		_moveZAxis = 1;
		break;
	case Qt::Key_D:
		_moveXAxis = 1;
		break;
	case Qt::Key_A:
		_moveXAxis = -1;
		break;
    }
}

void Main::keyReleaseEvent(const QVRRenderContext& /* context */, QKeyEvent* event) {
	_moveZAxis = 0;
	_moveXAxis = 0;
}

void Main::deviceAnalogChangeEvent(QVRDeviceEvent *event) {
	switch (event->analog()) {
		case QVR_Analog_Axis_Y:
			_moveZAxis = event->device().analogValue(QVR_Analog_Axis_Y);
			break;
		case QVR_Analog_Axis_X:
			_moveXAxis = event->device().analogValue(QVR_Analog_Axis_X);
			break;
	}
}

void Main::mouseMoveEvent(const QVRRenderContext &context, QMouseEvent *event)
{
    QPointF current = event->pos();
    float yaw = -current.x() / context.windowGeometry().width();
    float pitch = -current.y() / context.windowGeometry().height();

    _orientation = QQuaternion::fromEulerAngles(pitch * 90 + 45, yaw * 360, 0.f);
}

void Main::mousePressEvent(const QVRRenderContext &context, QMouseEvent *event)
{
    switch (event->buttons()) {
    case Qt::LeftButton:
        _moveZAxis = -1;
        break;
    case Qt::RightButton:
        _moveZAxis = 1;
        break;
    }
}

void Main::mouseReleaseEvent(const QVRRenderContext &context, QMouseEvent *event)
{
    _moveZAxis = 0;
}

#if(!MAZE)
bool Main::initProcess(QVRProcess* /* p */)
{
    /* Initialize per-process OpenGL resources and state here */
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned short> indices;

    // Qt-based OpenGL function pointers
    initializeOpenGLFunctions();

    // FBO
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glGenTextures(1, &_fboDepthTex);
    glBindTexture(GL_TEXTURE_2D, _fboDepthTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1,
            0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _fboDepthTex, 0);

    // Floor
    geom_quad(positions, normals, texcoords, indices);
    _floorVao = setupVao(positions.size() / 3, positions.data(), normals.data(), texcoords.data(),
            indices.size(), indices.data());
    _floorIndices = indices.size();
    _floorMaterial = Material(0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            setupTex(":floor-diff.jpg"), isGLES ? 0 : setupTex(":floor-norm.jpg"), 0, 10.0f);

    // Pillar
    geom_cylinder(positions, normals, texcoords, indices, isGLES ? 20 : 40);
    _pillarVaos[0] = setupVao(positions.size() / 3, positions.data(), normals.data(), texcoords.data(),
            indices.size(), indices.data());
    _pillarIndices[0] = indices.size();
    geom_disk(positions, normals, texcoords, indices, 0.0f, isGLES ? 20 : 40);
    _pillarVaos[1] = setupVao(positions.size() / 3, positions.data(), normals.data(), texcoords.data(),
            indices.size(), indices.data());
    _pillarIndices[1] = indices.size();
    _pillarMaterial = Material(0.5f, 0.5f, 0.3f, 0.5f, 0.5f, 100.0f,
            setupTex(":pillar-diff.jpg"), isGLES ? 0 : setupTex(":pillar-norm.jpg"), isGLES ? 0 : setupTex(":pillar-spec.jpg"));

    // Object
    geom_cube(positions, normals, texcoords, indices);
    _objectVaos[0] = setupVao(positions.size() / 3, positions.data(), normals.data(), texcoords.data(),
            indices.size(), indices.data());
    _objectIndices[0] = indices.size();
    _objectMaterials[0] = Material(0.8f, 0.3f, 0.3f, 0.8f, 0.2f, 20.0f);
    _objectMatrices[0].rotate(15.0f, 1.0f, 1.0f, 0.0f);
    _objectMatrices[0].scale(0.5f);
    geom_cone(positions, normals, texcoords, indices, isGLES ? 20 : 40, isGLES ? 10 : 20);
    _objectVaos[1] = setupVao(positions.size() / 3, positions.data(), normals.data(), texcoords.data(),
            indices.size(), indices.data());
    _objectIndices[1] = indices.size();
    _objectMaterials[1] = Material(0.8f, 0.6f, 0.3f, 0.8f, 0.2f, 20.0f);
    _objectMatrices[1].rotate(15.0f, 1.0f, 1.0f, 0.0f);
    _objectMatrices[1].scale(0.5f);
    geom_torus(positions, normals, texcoords, indices, 0.4f, isGLES ? 20 : 40, isGLES ? 20 : 40);
    _objectVaos[2] = setupVao(positions.size() / 3, positions.data(), normals.data(), texcoords.data(),
            indices.size(), indices.data());
    _objectIndices[2] = indices.size();
    _objectMaterials[2] = Material(0.4f, 0.8f, 0.3f, 0.8f, 0.2f, 20.0f);
    _objectMatrices[2].rotate(15.0f, 1.0f, 1.0f, 0.0f);
    _objectMatrices[2].scale(0.5f);
    geom_teapot(positions, normals, texcoords, indices);
    _objectVaos[3] = setupVao(positions.size() / 3, positions.data(), normals.data(), texcoords.data(),
            indices.size(), indices.data());
    _objectIndices[3] = indices.size();
    _objectMaterials[3] = Material(0.3f, 0.3f, 0.8f, 0.8f, 0.2f, 20.0f);
    _objectMatrices[3].rotate(15.0f, 1.0f, 1.0f, 0.0f);
    geom_cylinder(positions, normals, texcoords, indices, isGLES ? 20 : 40);
    _objectVaos[4] = setupVao(positions.size() / 3, positions.data(), normals.data(), texcoords.data(),
            indices.size(), indices.data());
    _objectIndices[4] = indices.size();
    _objectMaterials[4] = Material(0.3f, 0.8f, 0.8f, 0.8f, 0.2f, 20.0f);
    _objectMatrices[4].rotate(15.0f, 1.0f, 1.0f, 0.0f);
    _objectMatrices[4].scale(0.5f);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Shader program
    QString vertexShaderSource = readFile(":vertex-shader.glsl");
    QString fragmentShaderSource  = readFile(":fragment-shader.glsl");
    if (isGLES) {
        vertexShaderSource.prepend("#version 300 es\n");
        fragmentShaderSource.prepend("#version 300 es\n");
        fragmentShaderSource.replace("$WITH_NORMAL_MAPS", "0");
        fragmentShaderSource.replace("$WITH_SPEC_MAPS", "0");
    } else {
        vertexShaderSource.prepend("#version 330\n");
        fragmentShaderSource.prepend("#version 330\n");
        fragmentShaderSource.replace("$WITH_NORMAL_MAPS", "1");
        fragmentShaderSource.replace("$WITH_SPEC_MAPS", "1");
    }
    _prg.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    _prg.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    _prg.link();

    // Device model data
    for (int i = 0; i < QVRManager::deviceModelVertexDataCount(); i++) {
        _devModelVaos.append(setupVao(
                    QVRManager::deviceModelVertexCount(i),
                    QVRManager::deviceModelVertexPositions(i),
                    QVRManager::deviceModelVertexNormals(i),
                    QVRManager::deviceModelVertexTexCoords(i),
                    QVRManager::deviceModelVertexIndexCount(i),
                    QVRManager::deviceModelVertexIndices(i)));
        _devModelVaoIndices.append(QVRManager::deviceModelVertexIndexCount(i));
    }
    for (int i = 0; i < QVRManager::deviceModelTextureCount(); i++) {
        _devModelTextures.append(setupTex(QVRManager::deviceModelTexture(i)));
    }

    return true;
}

void Main::render(QVRWindow* /* w */,
        const QVRRenderContext& context, const unsigned int* textures)
{
    for (int view = 0; view < context.viewCount(); view++) {
        // Get view dimensions
        int width = context.textureSize(view).width();
        int height = context.textureSize(view).height();
        // Set up framebuffer object to render into
        glBindTexture(GL_TEXTURE_2D, _fboDepthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height,
                0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[view], 0);
        // Set up view
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        QMatrix4x4 projectionMatrix = context.frustum(view).toMatrix4x4();
        QMatrix4x4 viewMatrix = context.viewMatrix(view);
        // Set up shader program
        glUseProgram(_prg.programId());
        glEnable(GL_DEPTH_TEST);
        // Render scene
        setMaterial(_floorMaterial);
        QMatrix4x4 groundMatrix;
        groundMatrix.scale(5.0f);
        groundMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
        renderVao(projectionMatrix, viewMatrix, groundMatrix, _floorVao, _floorIndices);
        for (int i = 0; i < 5; i++) {
            setMaterial(_pillarMaterial);
            QMatrix4x4 pillarMatrix, pillarDiskMatrix, objectMatrix;
            pillarMatrix.rotate(18.0f + (i + 1) * 72.0f, 0.0f, 1.0f, 0.0f);
            pillarMatrix.translate(2.0f, 0.0f, 0.0f);
            pillarDiskMatrix = pillarMatrix;
            objectMatrix = pillarMatrix;
            pillarMatrix.translate(0.0f, 0.8f, 0.0f);
            pillarMatrix.scale(0.2f, 0.8f, 0.2f);
            renderVao(projectionMatrix, viewMatrix, pillarMatrix, _pillarVaos[0], _pillarIndices[0]);
            pillarDiskMatrix.translate(0.0f, 1.6f, 0.0f);
            pillarDiskMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
            pillarDiskMatrix.scale(0.2f);
            renderVao(projectionMatrix, viewMatrix, pillarDiskMatrix, _pillarVaos[1], _pillarIndices[1]);
            setMaterial(_objectMaterials[i]);
            objectMatrix.translate(0.0f, 1.75f, 0.0f);
            objectMatrix.scale(0.2f);
            objectMatrix.rotate(_objectRotationAngle, 0.0f, 1.0f, 0.0f);
            objectMatrix *= _objectMatrices[i];
            renderVao(projectionMatrix, viewMatrix, objectMatrix, _objectVaos[i], _objectIndices[i]);
        }
        // Render device models (optional)
        for (int i = 0; i < QVRManager::deviceCount(); i++) {
            const QVRDevice& device = QVRManager::device(i);
            for (int j = 0; j < device.modelNodeCount(); j++) {
                QMatrix4x4 nodeMatrix = device.matrix();
                nodeMatrix.translate(device.modelNodePosition(j));
                nodeMatrix.rotate(device.modelNodeOrientation(j));
                int vertexDataIndex = device.modelNodeVertexDataIndex(j);
                int textureIndex = device.modelNodeTextureIndex(j);
                Material material(1.0f, 1.0f, 1.0f,
                        1.0f, 0.0f, 0.0f,
                        _devModelTextures[textureIndex], 0, 0,
                        1.0f);
                setMaterial(material);
                renderVao(projectionMatrix, context.viewMatrixPure(view), nodeMatrix,
                        _devModelVaos[vertexDataIndex],
                        _devModelVaoIndices[vertexDataIndex]);
            }
        }
        // Invalidate depth attachment (to help OpenGL ES performance)
        const GLenum fboInvalidations[] = { GL_DEPTH_ATTACHMENT };
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, fboInvalidations);
    }
}
#endif

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QVRManager manager(argc, argv);

    isGLES = (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES);
    Drawable::setGLES(isGLES);

    /* First set the default surface format that all windows will use */
    QSurfaceFormat format;
    if (isGLES) {
        format.setVersion(3, 0);
    } else {
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setVersion(3, 3);
    }
    format.setOption(QSurfaceFormat::DebugContext);
    QSurfaceFormat::setDefaultFormat(format);

    /* Then start QVR with your app */
    Main qvrapp;
    if (!manager.init(&qvrapp, CUSTOM_NAV)) {
        qCritical("Cannot initialize QVR manager");
        return 1;
    }

    /* Enter the standard Qt loop */
    return app.exec();
}
