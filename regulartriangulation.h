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
};

#endif // REGULARTRIANGULATION_H
