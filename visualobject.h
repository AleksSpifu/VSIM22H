#ifndef VISUALOBJECT_H
#define VISUALOBJECT_H

#include <QOpenGLFunctions_4_1_Core>
#include <QMatrix4x4>
#include <vector>
#include <string>
#include "vertex.h"

class VisualObject : public QOpenGLFunctions_4_1_Core {
public:
    VisualObject();
    ~VisualObject();
    virtual void init(GLint matrixUniform)=0;
    virtual void draw()=0;
    virtual void Tick(float deltaTime){};
    virtual void move(float x, float y, float z);
    virtual void move(float dt) { };
    virtual void setVertices(std::vector<Vertex> vertices);
    virtual QVector3D getPosition();
    void setName(std::string name);
    std::string Name();
    bool nameContains(std::string test){return mName.compare(test) == 1;};
    QMatrix4x4 mMatrix;
    QVector3D Barycentric(QVector3D object, QVector3D P1, QVector3D P2, QVector3D P3);
    bool isOverlappingTriangle(QVector3D baryc, QVector3D P1, QVector3D P2, QVector3D P3);
    float GetBarycentricHeight(QVector3D baryc, QVector3D P1, QVector3D P2, QVector3D P3);
protected:
    friend class InteractiveObject;
    std::vector<Vertex> mVertices;
    GLuint mVAO{0};
    GLuint mVBO{0};
    GLint mMatrixUniform{0};

    QMatrix4x4 mRotation;
    QMatrix4x4 mPosition;
    QMatrix4x4 mScale;
    QVector3D mVelocity{.1f, 0.f, 0.f};
    std::string mName;
};
#endif // VISUALOBJECT_H
