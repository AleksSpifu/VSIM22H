#include "camera.h"
#include <iostream>
QVector3D Camera::GetPosition()
{
    return mEye;
}

Camera::Camera() : mEye{3, -10, 8}
{

    mVmatrix.setToIdentity();
}

Camera::~Camera()
{

}

void Camera::init(GLint pMatrixUniform, GLint vMatrixUniform)
{

    mVmatrix.setToIdentity();

    mPmatrixUniform = pMatrixUniform;
    mVmatrixUniform = vMatrixUniform;
}

void Camera::perspective(float degrees, double aspect, double nearplane, double farplane)
{

    mPmatrix.perspective(degrees, aspect, nearplane, farplane);
    //mPmatrix.frustum(-1, 1, -1, 1, 1, 100);
}

void Camera::lookAt(const QVector3D &eye, const QVector3D &at, const QVector3D &up)
{
    mVmatrix.setToIdentity();
    mVmatrix.lookAt(eye, at, up);
}

void Camera::rotateAround(int degrees, const QVector3D &around)
{
    //    for (int i = 0; i < degrees; i++) {
    //        mEye.setY(mEye.y()+.01f);
    //        mVmatrix.translate(0, 0.01, 0);
    //        lookAt(around);
    //    }

    QMatrix4x4 rotation;
    rotation.setToIdentity();

    rotation.translate(around);
    rotation.rotate(degrees, {0, 0, 1});
    rotation.translate(-around);

    mEye = rotation * mEye;

    mVmatrix.setToIdentity();

    mVmatrix.lookAt(mEye, around, QVector3D{0, 0, 1});

}

void Camera::rotateAround(float magnitude, const QVector3D direction)
{
    if (mTarget == nullptr) return;

    QMatrix4x4 rotation;

    rotation.setToIdentity();

    QVector3D around = mTarget->getPosition();

    rotation.translate(around);
    rotation.rotate(magnitude, direction);
    rotation.translate(-around);

    mEye = rotation * mEye;

    mVmatrix.setToIdentity();

    mVmatrix.lookAt(mEye, around, QVector3D{0, 0, 1});
}

void Camera::followMouseMovements(float x, float y)
{

    QMatrix4x4 rotation;

    rotation.setToIdentity();
    QVector3D around;
    if (mTarget == nullptr) {
        around = {0, 0, 0};
    } else {
        around = mTarget->getPosition();
    }

    QVector3D oldPosition = mEye;
    QVector3D oldRight = Right();

    x *= mMovementSensitivity;
    y *= mMovementSensitivity;

    rotation.translate(around);
    rotation.rotate(y, Right());
    rotation.rotate(x * Up().z(), Up());
    rotation.translate(-around);

    mEye = rotation.map(mEye);

    if (QVector3D::dotProduct(Right(), oldRight) < 0)
        mEye = oldPosition;
    //std::cout << "X: " << Right().x() << "y: " << Right().y() << "Z: " << Right().z() << std::endl;

    mVmatrix.setToIdentity();

    mVmatrix.lookAt(mEye, around, QVector3D{0, 0, 1});
}

void Camera::lookAt(const QVector3D &at)
{
    mVmatrix.setToIdentity();
    mVmatrix.lookAt(mEye, at, QVector3D{0, 0, 1});
}

void Camera::update()
{
    initializeOpenGLFunctions();
    glUniformMatrix4fv(mPmatrixUniform, 1, GL_FALSE, mPmatrix.constData());
    glUniformMatrix4fv(mVmatrixUniform, 1, GL_FALSE, mVmatrix.constData());
}

void Camera::update(GLint pMat, GLint vMat)
{
    initializeOpenGLFunctions();
    glUniformMatrix4fv(pMat, 1, GL_FALSE, mPmatrix.constData());
    glUniformMatrix4fv(vMat, 1, GL_FALSE, mVmatrix.constData());
}

void Camera::SetTarget(VisualObject *target)
{
    mTarget = target;
}

void Camera::SetStatic(bool isStatic)
{
    mStaticPosition = isStatic;
}

void Camera::SetPosition(const QVector3D& newPos) {
    mEye = newPos;
}

void Camera::Tick(float deltaTime)
{
    //Get target position
    QVector3D targetPos;

//    for (int i = 0; i < 4; i++) {
//        std::cout << mVmatrix.row(i).x() << "\t" << mVmatrix.row(i).y() << "\t" << mVmatrix.row(i).z() << "\t" << mVmatrix.row(i).w() << "\t" <<std::endl;
//    }
//    std::cout << std::endl;

    if (mTarget != nullptr) {
        targetPos = mTarget->getPosition();
    } else {
        targetPos = QVector3D{0, 0, 0};
    }

    if (mStaticPosition) {
            lookAt(targetPos);
            return;
    }



    //Get direction from target to camera
    QVector3D direction = mEye - oldTargetPos;

    //normalize
    direction.normalize();

    //get new camera location from normalized direction and distance from target
    mEye = direction * mDistanceToTarget + targetPos;

    lookAt(mTarget != nullptr ? mTarget->getPosition() : QVector3D{0, 0, 0});

    oldTargetPos = targetPos;
}

void Camera::Zoom(float diff)
{
    diff = std::clamp(diff, -10.f, 10.f) * mZoomSensitivity;
    mDistanceToTarget = std::clamp(mDistanceToTarget - diff, 5.f, 60.f);
}

void Camera::setAllowedToFollow(bool status)
{
    mAllowDirectionChange = status;
}

QVector3D Camera::Forward()
{
    auto targetPos = mTarget != nullptr ? mTarget->getPosition() : QVector3D{0, 0, 0};

    QVector3D out = targetPos - mEye;

    out.normalize();

    //std::cout << "X: " << out.x() << "y: " << out.y() << "Z: " << out.z() << std::endl;
    return out;
}

QVector3D Camera::Right()
{
    QVector3D out = QVector3D::crossProduct(Forward(), {0, 0, 1});

    out.normalize();

    return out;
}

QVector3D Camera::Up()
{
    QVector3D out = QVector3D::crossProduct(Right(), Forward());

    out.normalize();
    //std::cout << "X: " << out.x() << "y: " << out.y() << "Z: " << out.z() << std::endl;
    return out;
}

