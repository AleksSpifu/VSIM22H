#include "trianglesurface.h"
#include <cmath>
#include <iostream>

TriangleSurface::TriangleSurface()
{
    CreateGraphVertices(myEquation);
    WriteToFile("filnavn.txt");
    ReadFromFile("filnavn.txt");
    mMatrix.setToIdentity();
}

TriangleSurface::TriangleSurface(std::string fileName)
{
    ReadFromFile(fileName);
    mMatrix.setToIdentity();
}

TriangleSurface::~TriangleSurface() {}

void TriangleSurface::init(GLint matrixUniform)
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

void TriangleSurface::draw()
{
    glBindVertexArray( mVAO );
    glUniformMatrix4fv( mMatrixUniform, 1, GL_FALSE, mMatrix.constData());
    glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
}

float TriangleSurface::myEquation(float x, float y)
{
    return 0.5*cos(2*sqrt(pow(x, 2)+pow(y, 2))) - sqrt(x*x+y*y) * sqrt(x*x+y*y) * 0.015 + 0.5;
}

void TriangleSurface::WriteToFile(std::string fileName)
{
    std::ofstream file;
    file.open("../VSIM22H/txt_files/"+fileName);
    if(file.is_open())
    {
        file << mVertices.size() << std::endl;
        for(auto i = 0; i < mVertices.size(); i++)
        {
            file << mVertices[i] << std::endl;
        }
        file.close();
    }
    mVertices.clear();
}

void TriangleSurface::ReadFromFile(std::string fileName)
{
    std::ifstream file;
    file.open("../VSIM22H/txt_files/"+fileName);
    if (file.is_open())
    {
        int x{0};
        Vertex localVertex;
        file >> x;                          // First line assigned to x
        mVertices.reserve(x);               // Reserves space for x number of vertices
        for (int i=0; i<x; i++)
        {
             file >> localVertex;
             mVertices.push_back(localVertex);
        }
        file.close();
    }
}

void TriangleSurface::CreateGraphVertices(float (f)(float, float))
{

    float xmin=0.f, xmax=1.f, ymin=0.f, ymax=1.f, h=0.025f; // The size to draw
    for (auto x=xmin; x<xmax; x+=h)
        for (auto y=ymin; y<ymax; y+=h)
        {
            float xSquared = x * x;
            float ySquared = y * y;
            float combined = xSquared + ySquared;
            float lengthFromOrigin = sqrt(combined);
            if (lengthFromOrigin > 20.f) {
                continue;
            }


            float z = f(x,y);
            mVertices.push_back(Vertex{x,y,z,z/1.5f+.8f,0.5f*z/1.5f+.8f,z/1.5f+.8f, x, y});

            z = f(x+h,y);
            mVertices.push_back(Vertex{x+h,y,z,z/1.5f+.8f,0.5f*z/1.5f+.8f,z/1.5f+.8f, x, y});

            z = f(x,y+h);
            mVertices.push_back(Vertex{x,y+h,z,z/1.5f+.8f,0.5f*z/1.5f+.8f,z/1.5f+.8f, x, y});

            mVertices.push_back(Vertex{x,y+h,z,z/1.5f+.8f,0.5f*z/1.5f+.8f,z/1.5f+.8f, x, y});

            z = f(x+h,y);
            mVertices.push_back(Vertex{x+h,y,z,z/1.5f+.8f,0.5f*z/1.5f+.8f,z/1.5f+.8f, x, y});

            z = f(x+h,y+h);
            mVertices.push_back(Vertex{x+h,y+h,z,z/1.5f+.8f,0.5f*z/1.5f+.8f,z/1.5f+.8f, x, y});
        }
}
