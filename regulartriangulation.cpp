#include "regulartriangulation.h"
#include "lastextreader.h"

RegularTriangulation::RegularTriangulation(std::string fileName)
{
    mResolution = 500;
    mSize = 500.f;
    if (!Las::LasTextReader::ReadFromFile(fileName, pointCloud))
    {
        std::cout << "did not find finished files, generating..." << std::endl;
        pointCloud = Las::LasTextReader::GenerateVerticesFromFile(fileName, mResolution, mSize);
    }
    mVertices = pointCloud.vertices;
    mIndices.reserve(pointCloud.indicesAndNeighbours.size() * 3);
    for (unsigned int i = 0; i < pointCloud.indicesAndNeighbours.size(); i++) {
        mIndices.push_back(pointCloud.indicesAndNeighbours[i].indicies[0]);
        mIndices.push_back(pointCloud.indicesAndNeighbours[i].indicies[1]);
        mIndices.push_back(pointCloud.indicesAndNeighbours[i].indicies[2]);
    }
    std::cout << mIndices.size() << std::endl;
}

void RegularTriangulation::init(GLint matrixUniform)
{
    mMatrixUniform = matrixUniform;
    initializeOpenGLFunctions();

    //Vertex Array Object - VAO
    glGenVertexArrays( 1, &mVAO );
    glBindVertexArray( mVAO );

    //Vertex Buffer Object to hold vertices - VBO
    glGenBuffers( 1, &mVBO );
    glBindBuffer( GL_ARRAY_BUFFER, mVBO );

    glBufferData(
                GL_ARRAY_BUFFER,
                mVertices.size()*sizeof(Vertex),
                mVertices.data(),
                GL_STATIC_DRAW
                );

    // 1rst attribute buffer : vertices
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                sizeof(Vertex),
                reinterpret_cast<const void*>(0)
                );
    glEnableVertexAttribArray(0);

    // 2nd attribute buffer : colors
    glVertexAttribPointer(
                1,
                3,
                GL_FLOAT,
                GL_FALSE,
                sizeof(Vertex),
                reinterpret_cast<const void*>(3 * sizeof(GLfloat))
                );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2,  GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)( 6 * sizeof(GLfloat)) );
    glEnableVertexAttribArray(2);

    //enable the matrixUniform
    // mMatrixUniform = glGetUniformLocation( matrixUniform, "matrix" );
    glGenBuffers(1, &mIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size()*sizeof(GLuint), mIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void RegularTriangulation::draw()
{
    initializeOpenGLFunctions();
    glBindVertexArray( mVAO );
    // GL_FALSE for QMatrix4x4
    //glUniformMatrix4fv( mMatrixUniform, 1, GL_TRUE, mMatrix.constData());
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, reinterpret_cast<const void*>(0));//mVertices.size());
}

std::vector<Vertex> RegularTriangulation::MakeHeightLines(float heightInterval)
{
    // finn maks høyde
    // finn ut hvor mange høydelinjer som skal tegnes
    // for hver høyde:
        // finn en eller annen trekant med riktig høyde
        // lagre den første sin posisjon, husk på den
        // sjekk om det er 1 eller 2 punkter som er høyere enn høydelinjen, avhengig av det blir det annerledes
        // finn det første punktet til høydelinjen på trekanten
        // bruk naboinfo til å finne neste trekant som har riktig høyde
        // når vi kommer til første trekanten, altså har tatt hele runden, går vi til neste høydelinje og gjør dette på nytt
    std::vector<Vertex> vertices;
    for (auto t : pointCloud.indicesAndNeighbours) {
        QVector3D p1 = mVertices[t.indicies[0]].GetXYZ();
        QVector3D p2 = mVertices[t.indicies[1]].GetXYZ();
        QVector3D p3 = mVertices[t.indicies[2]].GetXYZ();
    }

    return vertices;
}






















































