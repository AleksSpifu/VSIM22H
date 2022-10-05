#ifndef RAINTRAILS_H
#define RAINTRAILS_H

#include "visualobject.h"

class RainTrails : public VisualObject
{
public:
    RainTrails();
    void init(GLint matrixUniform) override;
    void draw() override;
    void ReportLocation(QVector3D location);
};

#endif // RAINTRAILS_H
