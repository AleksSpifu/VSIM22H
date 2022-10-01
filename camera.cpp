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

void Camera::Move(QVector3D direction, float speed)
{
    mEye += (-direction * speed);
    mVmatrix.translate(direction * speed);
}

void Camera::init(GLint pMatrixUniform, GLint vMatrixUniform)
{

    mVmatrix.setToIdentity();

    mPmatrixUniform = pMatrixUniform;
    mVmatrixUniform = vMatrixUniform;
    lookAt({0,0,0});
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

void Camera::firstPersonMouseMove(float x, float y)
{
    mVmatrix.translate(mEye);
    mVmatrix.rotate(-x * mMovementSensitivity,0,0,1);
    mVmatrix.rotate(y * mMovementSensitivity, {Right()});
    mVmatrix.translate(-mEye);

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

    QVector3D vec = mVmatrix.row(2).toVector3D();


    //std::cout << "X: " << out.x() << "y: " << out.y() << "Z: " << out.z() << std::endl;
    return vec;
}

QVector3D Camera::Right()
{
    QVector3D vec = mVmatrix.row(0).toVector3D();


    //std::cout << "X: " << out.x() << "y: " << out.y() << "Z: " << out.z() << std::endl;
    return -vec;
}

QVector3D Camera::Up()
{
    QVector3D out = QVector3D::crossProduct(Right(), Forward());

    out.normalize();
    //std::cout << "X: " << out.x() << "y: " << out.y() << "Z: " << out.z() << std::endl;
    return out;
}

