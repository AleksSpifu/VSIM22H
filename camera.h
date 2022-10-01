#ifndef CAMERA_H
#define CAMERA_H

#include <QOpenGLFunctions_4_1_Core>
#include <QMatrix4x4>
#include <QVector3D>

#include "visualobject.h"

class Camera : public QOpenGLFunctions_4_1_Core
{
private:
    GLint  mPmatrixUniform;
    GLint  mVmatrixUniform;
    QVector3D mEye;

    QMatrix4x4 mPmatrix{};

    VisualObject* mTarget{nullptr};
    float mDistanceToTarget{5.f};
    float mZoomSensitivity = 0.1f;
    float mMovementSensitivity = 0.5f;
    bool mCameraIsBusy{false};
    bool mAllowDirectionChange{false};
    bool mStaticPosition{false};
    QVector3D oldTargetPos;
    float pitch{0.f}, yaw{0.f};
public:
     QMatrix4x4 mVmatrix{};
    QVector3D GetPosition();
    Camera();
    ~Camera();
    void init(GLint pMatrixUniform, GLint vMatrixUniform);
    void perspective(float degrees, double aspect, double nearplane, double farplane);
    void lookAt(const QVector3D &at);
    void lookAt(const QVector3D& eye, const QVector3D& at, const QVector3D& up);
    void rotateAround(int degrees, const QVector3D &around);
    void rotateAround(float magnitude, const QVector3D direction);
    void followMouseMovements(float x, float y);
    void update();
    void SetTarget(VisualObject* target);
    void SetStatic(bool isStatic = true);
    void Tick(float deltaTime);
    void Zoom(float diff);
    void setAllowedToFollow(bool status);

    QVector3D Forward();
    QVector3D Right();
//    void rotate(){mVmatrix.rotate(1, {0, 0, 1});}

    QVector3D Up();
    void SetPosition(const QVector3D &newPos);
    void update(GLint pMat, GLint vMat);
    void Move(QVector3D direction, float speed);
    void firstPersonMouseMove(float x, float y);
};


#endif // CAMERA_H
