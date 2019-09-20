#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <QOpenGLShaderProgram>

class Drawable : public QWidget
{
    Q_OBJECT
public:
    explicit Drawable(QWidget *parent = nullptr);
    void init() const;
    void update() const;
    void render() const;
private:
    QOpenGLShaderProgram _shaderProgram;
signals:

public slots:
};

#endif // DRAWABLE_H
