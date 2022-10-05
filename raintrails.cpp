#include "raintrails.h"

RainTrails::RainTrails()
{

}

void RainTrails::init(GLint matrixUniform)
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

void RainTrails::draw()
{
    if (mVertices.size() == 0) return;
    glBindVertexArray( mVAO );
    glDrawArrays(GL_LINES, 0, mVertices.size());
}

void RainTrails::ReportLocation(QVector3D location)
{
    mVertices.push_back(Vertex(location, {0.5,0.5,1}));
    init(0);
}
