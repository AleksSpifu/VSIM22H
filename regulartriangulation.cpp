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

bool RegularTriangulation::CheckTriangleHeight(QVector3D p1, QVector3D p2, QVector3D p3, float h)
{
    return !((p1.z() < h) == (p2.z() < h) == (p3.z() < h));
}

bool RegularTriangulation::CheckTriangleHeight(int triIndex, float h)
{
    QVector3D p1 = mVertices[pointCloud.indicesAndNeighbours[triIndex].indicies[0]].GetXYZ();
    QVector3D p2 = mVertices[pointCloud.indicesAndNeighbours[triIndex].indicies[1]].GetXYZ();
    QVector3D p3 = mVertices[pointCloud.indicesAndNeighbours[triIndex].indicies[2]].GetXYZ();
    return !((p1.z() < h) == (p2.z() < h) == (p3.z() < h));
}

std::vector<Vertex> RegularTriangulation::MakeHeightLines(float heightInterval)
{
    std::vector<Vertex> vertices;
    // finn maks høyde
    float maxHeight = 0;
    for (auto v : mVertices) {
        maxHeight = std::max(maxHeight, v.GetXYZ().z());
    }
    // finn ut hvor mange høydelinjer som skal tegnes
    int linesToDraw = maxHeight / heightInterval;
    std::unordered_map<int, bool> drawnTriangles;
    // for hver høyde:
    for (int i = 1; i <= linesToDraw; i++) {
        float h = heightInterval * i;
        // finn en eller annen trekant med riktig høyde
        Las::Triangle triangle;
        int firstTriangle;
        for (int ti = 0; ti < pointCloud.indicesAndNeighbours.size(); ti++) {
            auto t = pointCloud.indicesAndNeighbours[ti];
            QVector3D p1 = mVertices[t.indicies[0]].GetXYZ();
            QVector3D p2 = mVertices[t.indicies[1]].GetXYZ();
            QVector3D p3 = mVertices[t.indicies[2]].GetXYZ();
            bool isLineThroughTriangle = CheckTriangleHeight(p1, p2, p3, h);
            if (isLineThroughTriangle) {
                triangle = t;
                firstTriangle = ti;
                break;
            }
        }

        // sjekk om det er 1 eller 2 punkter som er høyere enn høydelinjen, avhengig av det blir det annerledes

        int singleIndex;
        for (int j = 0; j < 3; j++) {
            if ((mVertices[triangle.indicies[j]].GetXYZ().z() > h) &&
                    (
                        (mVertices[triangle.indicies[(j+1)%3]].GetXYZ().z() < h) ==
                        (mVertices[triangle.indicies[(j+2)%3]].GetXYZ().z() < h))
                    ) {
                singleIndex = j;
            }
        }
        drawnTriangles[firstTriangle] = true;
        QVector3D p1 = mVertices[triangle.indicies[singleIndex]].GetXYZ();
        QVector3D p2 = mVertices[triangle.indicies[(singleIndex + 1) % 3]].GetXYZ();
        QVector3D p3 = mVertices[triangle.indicies[(singleIndex + 2) % 3]].GetXYZ();
        float percentageOfHeight = (h-p2.z()) / (p1.z()-p2.z());
        Vertex v1{p2+(p1-p2)*percentageOfHeight, {1,1,1}};
        percentageOfHeight = (h-p3.z()) / (p1.z()-p3.z());
        Vertex v2{p3+(p1-p3)*percentageOfHeight, {1,1,1}};
        vertices.push_back(v1);
        vertices.push_back(v2);

        int next;
        if (CheckTriangleHeight(pointCloud.indicesAndNeighbours[firstTriangle].neighbours[0], h))
        {
            next = pointCloud.indicesAndNeighbours[firstTriangle].neighbours[0];
        }
        else if (CheckTriangleHeight(pointCloud.indicesAndNeighbours[firstTriangle].neighbours[1], h))
        {
            next = pointCloud.indicesAndNeighbours[firstTriangle].neighbours[1];
        }
        else if (CheckTriangleHeight(pointCloud.indicesAndNeighbours[firstTriangle].neighbours[2], h))
        {
            next = pointCloud.indicesAndNeighbours[firstTriangle].neighbours[2];
        }
        while (drawnTriangles[next] == false) {

        }


        // finn det første punktet til høydelinjen på trekanten

        // bruk naboinfo til å finne neste trekant som har riktig høyde
        // når vi kommer til første trekanten, altså har tatt hele runden, går vi til neste høydelinje og gjør dette på nytt
    }


    return vertices;
}






















































