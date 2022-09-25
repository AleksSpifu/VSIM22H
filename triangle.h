#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <QOpenGLFunctions_4_1_Core>
#include <vector>
#include <QMatrix4x4>
#include "visualobject.h"

class Triangle : public VisualObject
{
public:
    Triangle();
    ~Triangle() override;

    void init(GLint mat) override;
    void draw() override;
};

#endif // TRIANGLE_H
