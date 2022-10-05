#ifndef CLOUD_H
#define CLOUD_H

#include "visualobject.h"
#include "raindrop.h"
#include "raintrails.h"

class Cloud : public VisualObject
{
public:
    Cloud(float scale, float worldScale, RegularTriangulation* ground);
    void draw() override;
    void init(GLint matrixUniform) override;
    void Tick(float deltaTime) override;
    std::vector<RainDrop*> mRainDrops;
    std::vector<RainTrails*> mRainTrails;
    RegularTriangulation* mGround;
    ~Cloud();
    void MakeRaindrop();
    void DeleteRainDrop(RainDrop* rd);
    int mRainAmount{0};
    unsigned long long amountOfRainSpawned{0};
private:
    float mWorldScale{1};
    float mTimer{0.f};
    float mScale{0.f};
};

#endif // CLOUD_H
