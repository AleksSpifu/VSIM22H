#include "raindrop.h"
#include "cloud.h"
#include "octahedronball.h"

RainDrop::RainDrop(QVector3D startPos, float worldScale, RegularTriangulation* ground, Cloud* cloud, unsigned long long index)
{
    mIndex = index;
    mCloud = cloud;
    mGround = ground;
    mWorldScale = worldScale;
    mAcceleration *= mWorldScale;
    mMatrix.translate(startPos);
    mVertices = OctahedronBall::oktaederUnitBall(2, {0,0,1}, mScale);
}


void RainDrop::init(GLint matrixUniform)
{

    mMatrixUniform = matrixUniform;
    initializeOpenGLFunctions();

    glGenVertexArrays( 1, &mVAO );
    glBindVertexArray( mVAO );

    glGenBuffers( 1, &mVBO );
    glBindBuffer( GL_ARRAY_BUFFER, mVBO );

    glBufferData(
                GL_ARRAY_BUFFER,                          //what buffer type
                mVertices.size() * sizeof( Vertex ),      //how big buffer do we need
                mVertices.data(),                         //the actual vertices
                GL_STATIC_DRAW                            //should the buffer be updated on the GPU
                );

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                sizeof(Vertex),
                reinterpret_cast<GLvoid*>(0)
                );                              // array buffer offset
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
                1,
                3,
                GL_FLOAT,
                GL_FALSE,
                sizeof(Vertex),
                reinterpret_cast<GLvoid*>((3 * sizeof(GLfloat)))
                );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); //Releases binds

}

bool RainDrop::EvaluateForDeletion()
{

    bool shouldSelfDestruct{false};
    QVector3D myPos = getPosition();
    if (mTimeAlive > mTimeToKill) {
        shouldSelfDestruct = true;
    } else if (myPos.x() < 0 || myPos.y() < 0) {
        shouldSelfDestruct = true;
    } else if (myPos.x() > mGround->mSize || myPos.y() > mGround->mSize) {
        shouldSelfDestruct = true;
    } else if (myPos.z() < 0) {
        shouldSelfDestruct = true;
    }

    if (shouldSelfDestruct) {
        mCloud->DeleteRainDrop(this);
        return true;
    }
    return false;
}

void RainDrop::Tick(float deltaTime)
{
    mTimeAlive += deltaTime;
    QVector3D myPos = getPosition();
    Las::Triangle tri = mGround->GetTriangle(myPos.x(), myPos.y());

    QVector3D p1 = mGround->pointCloud.vertices[tri.indicies[0]].GetXYZ();
    QVector3D p2 = mGround->pointCloud.vertices[tri.indicies[1]].GetXYZ();
    QVector3D p3 = mGround->pointCloud.vertices[tri.indicies[2]].GetXYZ();
    QVector3D Baryc = Barycentric(myPos, p1, p2, p3);
    float groundHeight = GetBarycentricHeight(Baryc, p1, p2, p3);

    if (myPos.z() - mScale - 0.1f > groundHeight) {
        mVelocity = mVelocity + (mAcceleration * deltaTime);

        QVector3D newVelocity = mVelocity * deltaTime;

        mMatrix.translate(newVelocity);
        mIsOnGround = false;
    } else {
        float difference = groundHeight - (myPos.z() - mScale);
        if (difference > 0) {
            mMatrix.translate(0,0,difference);
        }

        mVelocity = mVelocity + (mAcceleration * deltaTime);

        QVector3D groundNormal = QVector3D::crossProduct(p2-p1, p3-p1);
        groundNormal.normalize();

        QVector3D slideAlongNormal = mVelocity - 2 * (QVector3D::dotProduct(mVelocity, groundNormal)) * groundNormal;
        mVelocity = mVelocity + slideAlongNormal;
        mVelocity = mVelocity * mFriction;
        if (!mIsOnGround) {
            mVelocity = mVelocity * 0.9;
        }
        mIsOnGround = true;
        mHasReachedGround = true;

        QVector3D newVelocity = mVelocity * deltaTime;

        mMatrix.translate(newVelocity);
    }

    if (mHasReachedGround) {
        if (mTimeAlive > mLastReportTime + 0.5) {
            mLastReportTime = mTimeAlive;
            mCloud->mRainTrails[mIndex]->ReportLocation(getPosition());
        }
    }



}

void RainDrop::draw()
{
    glBindVertexArray( mVAO );
    glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
}
