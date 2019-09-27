#include "drawable.h"

Drawable::Drawable(std::string name): _name(name)
{
}

void Drawable::update(QMatrix4x4 modelMatrix, float elapsedMilli)
{
    _modelMatrix = modelMatrix;

    for (std::shared_ptr<Drawable> child : _children)
        child->update(modelMatrix, elapsedMilli);
}

void Drawable::render(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix)
{
    for (std::shared_ptr<Drawable> child : _children)
        child->render(vMatrix, pMatrix);

    glRender(vMatrix, pMatrix);
}

void Drawable::glRender(QMatrix4x4 &vMatrix, QMatrix4x4 &pMatrix)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    f->glUseProgram(_prg.programId());

    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, _material.diffTex);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, _material.normTex);
    f->glActiveTexture(GL_TEXTURE2);
    f->glBindTexture(GL_TEXTURE_2D, _material.specTex);

    // Projection
    QMatrix4x4 modelViewMatrix = vMatrix * _modelMatrix;
    _prg.setUniformValue("model_view_matrix", modelViewMatrix);
    _prg.setUniformValue("projection_model_view_matrix", pMatrix * modelViewMatrix);
    _prg.setUniformValue("normal_matrix", modelViewMatrix.normalMatrix());
    f->glBindVertexArray(_vao);
    f->glDrawElements(GL_TRIANGLES, _elementsCount, GL_UNSIGNED_SHORT, nullptr);
}

void Drawable::initBuffers(std::vector<QVector3D> *vertices
                           , std::vector<QVector3D> *normals
                           , std::vector<QVector2D> *texcoords
                           , std::vector<unsigned short> *indices)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    GLuint positionBuf, normalBuf, texcoordBuf, indexBuf;

    f->glGenVertexArrays(1, &_vao);
    f->glBindVertexArray(_vao);

    f->glGenBuffers(1, &positionBuf);
    f->glBindBuffer(GL_ARRAY_BUFFER, positionBuf);
    f->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr> (vertices->size() * sizeof (QVector3D)), vertices->data(), GL_STATIC_DRAW);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glEnableVertexAttribArray(0);

     f->glGenBuffers(1, &normalBuf);
     f->glBindBuffer(GL_ARRAY_BUFFER, normalBuf);
     f->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr> (normals->size() * sizeof (QVector3D)), normals->data(), GL_STATIC_DRAW);
     f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
     f->glEnableVertexAttribArray(1);

     f->glGenBuffers(1, &texcoordBuf);
     f->glBindBuffer(GL_ARRAY_BUFFER, texcoordBuf);
     f->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr> (texcoords->size() * sizeof (QVector2D)), texcoords->data(), GL_STATIC_DRAW);
     f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
     f->glEnableVertexAttribArray(2);

    f->glGenBuffers(1, &indexBuf);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr> (indices->size() * sizeof(unsigned short)), indices->data(), GL_STATIC_DRAW);

    f->glBindVertexArray(0);

    f->glDeleteBuffers(1, &positionBuf);
    f->glDeleteBuffers(1, &normalBuf);
    f->glDeleteBuffers(1, &texcoordBuf);
    f->glDeleteBuffers(1, &indexBuf);

    _elementsCount = static_cast<GLsizei> (indices->size());
}

void Drawable::addChild(std::shared_ptr<Drawable> child)
{
   _children.push_back(child);
}

unsigned int Drawable::loadTexture(const QString& filename)
{
    QImage img;
    img.load(filename);
    if (getGLES())
        img = img.scaledToWidth(img.width() / 2, Qt::SmoothTransformation);
    img = img.mirrored(false, true);
    img = img.convertToFormat(QImage::Format_RGBA8888);
    return loadTexture(img);
}

unsigned int Drawable::loadTexture(const QImage& img)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    unsigned int tex;
    f->glGenTextures(1, &tex);
    f->glBindTexture(GL_TEXTURE_2D, tex);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
            img.width(), img.height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
    f->glGenerateMipmap(GL_TEXTURE_2D);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    if (!getGLES())
        f->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);

    return tex;
}

void Drawable::setMaterial(const Material m)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    _material = m;

    f->glUseProgram(_prg.programId());

    // Material
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
}

QString Drawable::readFile(const char* fileName)
{
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    return in.readAll();
}

void Drawable::loadShader(const char* vertShaderPath, const char* fragShaderPath)
{
    QString vertexShaderSource = readFile(vertShaderPath);
    QString fragmentShaderSource  = readFile(fragShaderPath);

    if (getGLES()) {
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
}

QOpenGLShaderProgram& Drawable::getShader()
{
    return _prg;
}

GLuint Drawable::getVao()
{
    return _vao;
}

void Drawable::setVao(GLuint vao)
{
    _vao = vao;
}

QMatrix4x4& Drawable::getModelMatrix()
{
    return _modelMatrix;
}

void Drawable::setGLES(bool isGLES)
{
    Drawable::isGLES = isGLES;
}

bool Drawable::getGLES()
{
    return Drawable::isGLES;
}

bool Drawable::isGLES = false;
