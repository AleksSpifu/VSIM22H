#ifndef HEIGHTLINES_H
#define HEIGHTLINES_H

#include "visualobject.h"

class HeightLines : public VisualObject
{
public:
    HeightLines();
    void init(GLint matrixUniform) override;
    void draw() override;
};

#endif // HEIGHTLINES_H
