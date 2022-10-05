#ifndef REGULARTRIANGULATION_H
#define REGULARTRIANGULATION_H

#include "visualobject.h"
#include "lastextreader.h"

class RegularTriangulation : public VisualObject
{
public:
    RegularTriangulation(std::string fileName);
    void init(GLint matrixUniform) override;
    void draw() override;
    std::vector<GLuint> mIndices;
    GLuint mIBO{0};
    Las::PointCloudMesh pointCloud;
    int mResolution{0};
    float mSize{0.f};
    static inline float scale{1};
    std::vector<Vertex> MakeHeightLines(float heightInterval);
    Las::Triangle GetTriangle(float x, float y);
private:
    bool CheckTriangleHeight(QVector3D p1, QVector3D p2, QVector3D p3, float h);
    bool CheckTriangleHeight(int triIndex, float h);
};

#endif // REGULARTRIANGULATION_H
