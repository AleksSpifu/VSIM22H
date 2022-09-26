#ifndef LIGHT_H
#define LIGHT_H

#include <QVector3D>
#include "visualobject.h"

///Simple Light class that has a small mesh and some light parameters
class Light : public VisualObject
{
public:
    ///Light class with mesh and light parameters
    Light();
    virtual void init(GLint matrixUniform) override;
    virtual void draw() override;

    QVector3D mOrbitPoint;

    GLfloat mAmbientStrenght{0.3f};
    QVector3D mAmbientColor{0.3f, 0.3f, 0.3f};  //Grey

    GLfloat mLightStrenght{0.7f};
    QVector3D mLightColor{0.9f, 0.9f, 0.9f};    //Yellow

    GLfloat mSpecularStrenght{0.9f};
    GLint mSpecularExponent{4};

    void setOrbitPoint(QVector3D pt);
    void orbit(float dt);
    void setVertices(std::vector<Vertex> in) override {mVertices = in;};
};

#endif // LIGHT_H
