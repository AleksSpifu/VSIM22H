#ifndef RAINDROP_H
#define RAINDROP_H

#include "visualobject.h"

class RainDrop : public VisualObject
{
public:
    RainDrop(QVector3D startPos, float worldScale);
    void draw() override;
    void init(GLint matrixUniform) override ;
    void Tick(float deltaTime) override;
private:
    static inline float mWorldScale{1};
    QVector3D mAcceleration{0, 0, -9.81};
    QVector3D mVelocity{0, 0, 0};
};

#endif // RAINDROP_H
