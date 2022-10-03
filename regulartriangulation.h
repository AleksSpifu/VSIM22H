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
    std::vector<Vertex> MakeHeightLines(float heightInterval);
};

#endif // REGULARTRIANGULATION_H
