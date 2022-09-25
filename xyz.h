#ifndef XYZ_H
#define XYZ_H

#include "visualobject.h"

#include <QOpenGLFunctions_4_1_Core>
#include <QMatrix4x4>
#include <vector>
#include "vertex.h"

class XYZ : public VisualObject
{
public:


    XYZ();
    ~XYZ() override;

    void init(GLint matrixUniform) override;
    void draw() override;
};

#endif // XYZ_H
