#include "light.h"
#include "renderwindow.h"
#include "octahedronball.h"

Light::Light()
{
    mVertices = OctahedronBall::oktaederUnitBall(3, {1, 1, 0});

    mMatrix.setToIdentity();
}

void Light::setOrbitPoint(QVector3D pt) {
    mOrbitPoint = pt;
    mPosition.translate(mOrbitPoint);
}

void Light::init(GLint matrixUniform)
{
    initializeOpenGLFunctions();

    //Vertex Array Object - VAO
    glGenVertexArrays( 1, &mVAO );
    glBindVertexArray( mVAO );

    //Vertex Buffer Object to hold vertices - VBO
    glGenBuffers( 1, &mVBO );
    glBindBuffer( GL_ARRAY_BUFFER, mVBO );

    glBufferData( GL_ARRAY_BUFFER, mVertices.size()*sizeof(Vertex), mVertices.data(), GL_STATIC_DRAW );

    // 1rst attribute buffer : vertices
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // 2nd attribute buffer : colors
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,  sizeof(Vertex),  (GLvoid*)(3 * sizeof(GLfloat)) );
    glEnableVertexAttribArray(1);

    // 3rd attribute buffer : uvs
    glVertexAttribPointer(2, 2,  GL_FLOAT, GL_FALSE, sizeof( Vertex ), (GLvoid*)( 6 * sizeof( GLfloat ) ));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Light::draw()
{
    glBindVertexArray( mVAO );
    glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
    //glBindVertexArray(0);
}

void Light::orbit(float dt) {
    mPosition.setToIdentity();
    mPosition.translate(mOrbitPoint);
    mRotation.rotate(dt * 45, {0, 0, 1});
    mMatrix = mPosition * mRotation;
    mMatrix.translate({50, 0, 0});

}
