#ifndef OCTAHEDRONBALL_H
#define OCTAHEDRONBALL_H
#include <QVector3D>
#include <vector>
#include <vertex.h>
#include <iostream>
#include <sstream>

class OctahedronBall
{
private:
    static void lagTriangel(std::vector<Vertex>& mVertices, const QVector3D& v1, const QVector3D& v2, const QVector3D& v3);
    static void subDivide(std::vector<Vertex>& mVertices, const QVector3D& a, const QVector3D& b, const QVector3D& c, int n);
public:
    static std::vector<Vertex> oktaederUnitBall(int n, const QVector3D& color, float scale=1);
    static std::vector<Vertex> oktaederUnitBall(int n);
    static inline bool mUsingCustomColor{false};
    static inline float mScale{1};
    static inline QVector3D mColor{0, 0, 0};
    OctahedronBall();
    ~OctahedronBall();
};

#endif // OCTAHEDRONBALL_H
