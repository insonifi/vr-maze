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
    QMatrix4x4 modelViewMatrix = vMatrix * _modelMatrix;
    _prg.setUniformValue("model_view_matrix", modelViewMatrix);
    _prg.setUniformValue("projection_model_view_matrix", pMatrix * modelViewMatrix);
    _prg.setUniformValue("normal_matrix", modelViewMatrix.normalMatrix());
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _elementsCount, GL_UNSIGNED_SHORT, nullptr);
}

void Drawable::initBuffers(std::vector<QVector3D> *vertices
                           , std::vector<QVector3D> *normals
                           , std::vector<QVector2D> *texcoords
                           , std::vector<unsigned short> *indices)
{
    GLuint vao;
    GLuint positionBuf, normalBuf, texcoordBuf, indexBuf;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &positionBuf);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuf);
    glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof (QVector3D), vertices->data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &normalBuf);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuf);
    glBufferData(GL_ARRAY_BUFFER, normals->size() * sizeof (QVector3D), normals->data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &texcoordBuf);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuf);
    glBufferData(GL_ARRAY_BUFFER, texcoords->size() * sizeof (QVector2D), texcoords->data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glGenBuffers(1, &indexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(unsigned short), indices->data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &positionBuf);
    glDeleteBuffers(1, &normalBuf);
    glDeleteBuffers(1, &texcoordBuf);
    glDeleteBuffers(1, &indexBuf);

    _vao = vao;
    _elementsCount = static_cast<GLsizei> (indices->size());
}

void Drawable::addChild(std::shared_ptr<Drawable> child)
{
   _children.push_back(child);
}

void Drawable::setMaterial(const Material& m)
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

QOpenGLShaderProgram& Drawable::getShader()
{
    return _prg;
}

GLuint& Drawable::getVao()
{
    return _vao;
}

QMatrix4x4& Drawable::getModelMatrix()
{
    return _modelMatrix;
}
