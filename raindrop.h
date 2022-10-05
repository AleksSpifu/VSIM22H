#ifndef RAINDROP_H
#define RAINDROP_H

#include "visualobject.h"
#include "regulartriangulation.h"

class RainDrop : public VisualObject
{
public:
    RainDrop(QVector3D startPos, float worldScale, RegularTriangulation* ground, class Cloud* cloud, unsigned long long index, float scale=1);
    void draw() override;
    void init(GLint matrixUniform) override;
    void Tick(float deltaTime) override;
    RegularTriangulation* mGround;
    bool EvaluateForDeletion();
private:
    unsigned long long mIndex{0};
    float mTimeAlive{0};
    float mLastReportTime{0.f};
    float mTimeToKill{60.f};
    bool mIsOnGround{false};
    bool mHasReachedGround{false};
    class Cloud* mCloud;
    float mFriction{0.5};
    float mScale{0.5};
    static inline float mWorldScale{1};
    QVector3D mAcceleration{0, 0, -9.81};
    QVector3D mVelocity{0, 0, 0};
};

#endif // RAINDROP_H
