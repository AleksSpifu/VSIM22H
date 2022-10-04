#include "octahedronball.h"

OctahedronBall::OctahedronBall()
{

}

OctahedronBall::~OctahedronBall()
{
   //
}

void OctahedronBall::lagTriangel(std::vector<Vertex>& mVertices, const QVector3D& v1, const QVector3D& v2,
const QVector3D& v3)
{
    if (mUsingCustomColor) {

   Vertex v(v1.x(), v1.y(), v1.z(), mColor.x(), mColor.y(), mColor.z());
   mVertices.push_back(v);
   v = Vertex{v2.x(), v2.y(), v2.z(), mColor.x(), mColor.y(), mColor.z()};
   mVertices.push_back(v);
   v = Vertex{v3.x(), v3.y(), v3.z(), mColor.x(), mColor.y(), mColor.z()};
   mVertices.push_back(v);
    }
        else {

        Vertex v(v1.x(), v1.y(), v1.z(), v1.x(), v1.y(), v1.z());
        mVertices.push_back(v);
        v = Vertex{v2.x(), v2.y(), v2.z(), v2.x(), v2.y(), v2.z()};
        mVertices.push_back(v);
        v = Vertex{v3.x(), v3.y(), v3.z(), v3.x(), v3.y(), v3.z()};
        mVertices.push_back(v);
    }
}

void OctahedronBall::subDivide(std::vector<Vertex>& mVertices, const QVector3D& a, const QVector3D& b, const QVector3D& c, int n)
{
   if (n>0) {
       QVector3D v1 = a+b; v1.normalize(); v1 *= mScale;
       QVector3D v2 = a+c; v2.normalize(); v2 *= mScale;
       QVector3D v3 = c+b; v3.normalize(); v3 *= mScale;
       subDivide(mVertices, a, v1, v2, n-1);
       subDivide(mVertices, c, v2, v3, n-1);
       subDivide(mVertices, b, v3, v1, n-1);
       subDivide(mVertices, v3, v2, v1, n-1);
   } else {
       lagTriangel(mVertices, a, b, c);
   }
}

std::vector<Vertex> OctahedronBall::oktaederUnitBall(int n, const QVector3D& color, float scale)
{
    mColor = color;
    mScale = scale;
    mUsingCustomColor = true;
    int m_rekursjoner = n;
    std::vector<Vertex> mVertices;
    mVertices.reserve(3 * 8 * pow(4, m_rekursjoner));
    QVector3D v0{0, 0, 1}; v0 *= mScale;
    QVector3D v1{1, 0, 0}; v1 *= mScale;
    QVector3D v2{0, 1, 0}; v2 *= mScale;
    QVector3D v3{-1, 0, 0}; v3 *= mScale;
    QVector3D v4{0, -1, 0}; v4 *= mScale;
    QVector3D v5{0, 0, -1}; v5 *= mScale;

    subDivide(mVertices, v0, v1, v2, m_rekursjoner);
    subDivide(mVertices, v0, v2, v3, m_rekursjoner);
    subDivide(mVertices, v0, v3, v4, m_rekursjoner);
    subDivide(mVertices, v0, v4, v1, m_rekursjoner);
    subDivide(mVertices, v5, v2, v1, m_rekursjoner);
    subDivide(mVertices, v5, v3, v2, m_rekursjoner);
    subDivide(mVertices, v5, v4, v3, m_rekursjoner);
    subDivide(mVertices, v5, v1, v4, m_rekursjoner);

    return mVertices;
}

std::vector<Vertex> OctahedronBall::oktaederUnitBall(int n)
{
    mUsingCustomColor = false;
    int m_rekursjoner = n;
    std::vector<Vertex> mVertices;
    mVertices.reserve(3 * 8 * pow(4, m_rekursjoner));
    QVector3D v0{0, 0, 1};
    QVector3D v1{1, 0, 0};
    QVector3D v2{0, 1, 0};
    QVector3D v3{-1, 0, 0};
    QVector3D v4{0, -1, 0};
    QVector3D v5{0, 0, -1};

    subDivide(mVertices, v0, v1, v2, m_rekursjoner);
    subDivide(mVertices, v0, v2, v3, m_rekursjoner);
    subDivide(mVertices, v0, v3, v4, m_rekursjoner);
    subDivide(mVertices, v0, v4, v1, m_rekursjoner);
    subDivide(mVertices, v5, v2, v1, m_rekursjoner);
    subDivide(mVertices, v5, v3, v2, m_rekursjoner);
    subDivide(mVertices, v5, v4, v3, m_rekursjoner);
    subDivide(mVertices, v5, v1, v4, m_rekursjoner);

    return mVertices;
}


