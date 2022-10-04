#include "raindrop.h"
#include "octahedronball.h"

RainDrop::RainDrop(QVector3D startPos, float worldScale)
{
    mWorldScale = worldScale;
    mAcceleration *= mWorldScale;
    mMatrix.translate(startPos);
    mVertices = OctahedronBall::oktaederUnitBall(2, {0,0,1}, 1);
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

void RainDrop::Tick(float deltaTime)
{
    if (getPosition().z() < 0) {
        mMatrix.translate({0,0,200});
        mVelocity = {0,0,0};
    }

    // få tak i normalen og høyden på bakken
    mVelocity = mVelocity + (mAcceleration * deltaTime);

    //slide along normal
    //friksjon

    QVector3D newVelocity = mVelocity * deltaTime;

    mMatrix.translate(newVelocity);
}

void RainDrop::draw()
{
    glBindVertexArray( mVAO );
    glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
}
