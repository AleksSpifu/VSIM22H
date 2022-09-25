// VisualObject.cpp
#include "visualobject.h"

VisualObject::VisualObject() {
    mRotation.setToIdentity();
    mScale.setToIdentity();
    mPosition.setToIdentity();
}


VisualObject::~VisualObject() {
   glDeleteVertexArrays( 1, &mVAO );
   glDeleteBuffers( 1, &mVBO );
}

void VisualObject::move(float x, float y, float z)
{
    mMatrix.translate(x, y, z);
}

void VisualObject::setVertices(std::vector<Vertex> vertices) {
    mVertices = vertices;
}

QVector3D VisualObject::Barycentric(QVector3D object, QVector3D P1, QVector3D P2, QVector3D P3)
{
    P1.setZ(0);
    P2.setZ(0);
    P3.setZ(0);

    QVector3D P12 = (P2-P1);
    QVector3D P13 = (P3-P1);

    QVector3D n = QVector3D::crossProduct(P12, P13);
    float areal_123 = n.length();

    QVector3D baryc;

    // u
    QVector3D p = P2 - object;
    QVector3D q = P3 - object;
    n = QVector3D::crossProduct(p,q);
    baryc.setX(n.z()/areal_123);

    // v
    p = P3 - object;
    q = P1 - object;
    n = QVector3D::crossProduct(p,q);
    baryc.setY(n.z()/areal_123);

    // w
    p = P1 - object;
    q = P2 - object;
    n = QVector3D::crossProduct(p,q);
    baryc.setZ(n.z()/areal_123);

    return baryc;

}

bool VisualObject::isOverlappingTriangle(QVector3D baryc, QVector3D P1, QVector3D P2, QVector3D P3)
{
    //auto baryc = Barycentric(P1, P2, P3);
    return baryc.x() >= 0.f && baryc.y() >= 0.f && baryc.z() >= 0.f;
}

float VisualObject::GetBarycentricHeight(QVector3D baryc, QVector3D P1, QVector3D P2, QVector3D P3)
{
    //auto baryc = Barycentric(P1, P2, P3);
    return P1.z() * baryc.x() + P2.z() * baryc.y() + P3.z() * baryc.z();
}

void VisualObject::setName(std::string name)
{
    mName = name;
}

std::string VisualObject::Name()
{
    return mName;
}

QVector3D VisualObject::getPosition()
{
    auto pos = mPosition.column(3);
    return QVector3D{pos.x(), pos.y(), pos.z()};
}

