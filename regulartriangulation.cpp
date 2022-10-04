#include "regulartriangulation.h"
#include "lastextreader.h"
#include <stack>

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
    float maxHeight = std::max(p1.z(), p2.z());
    maxHeight = std::max(maxHeight, p3.z());

    float minHeight = std::min(p1.z(), p2.z());
    minHeight = std::min(minHeight, p3.z());

    bool out = minHeight < h && maxHeight > h;
    return out;
}

bool RegularTriangulation::CheckTriangleHeight(int triIndex, float h)
{
    int i1 = pointCloud.indicesAndNeighbours[triIndex].indicies[0];
    int i2 = pointCloud.indicesAndNeighbours[triIndex].indicies[1];
    int i3 = pointCloud.indicesAndNeighbours[triIndex].indicies[2];
    Vertex v1 = mVertices[i1];
    Vertex v2 = mVertices[i2];
    Vertex v3 = mVertices[i3];
    QVector3D p1 = v1.GetXYZ();
    QVector3D p2 = v2.GetXYZ();
    QVector3D p3 = v3.GetXYZ();

    return CheckTriangleHeight(p1,p2,p3,h);
}

void print(std::string s) {
    std::cout << s << std::endl;
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
    std::stack<int> trianglesToDraw;

    // for hver høyde:
    for (int i = 1; i <= linesToDraw; i++) {
        float h = heightInterval * i;
        // finn en eller annen trekant med riktig høyde


        for (int ti = 0; ti < pointCloud.indicesAndNeighbours.size(); ti++) {
            auto t = pointCloud.indicesAndNeighbours[ti];

            int singleIndex{0};
            for (int j = 0; j < 3; j++) {

                // Fix me
                //loop gjennom alle 3 punktene, finn indeks som er minst, og index som er høyest, og da er den indexen som ikke er brukt enda enten over eller under høyden, og det tilsier hvilket punkt som er den single

                if (
                        ((mVertices[t.indicies[j]].GetXYZ().z() > h) &&
                         (
                             (mVertices[t.indicies[(j+1)%3]].GetXYZ().z() < h) &&
                             (mVertices[t.indicies[(j+2)%3]].GetXYZ().z() < h)
                             ))
                        ||
                        ((mVertices[t.indicies[j]].GetXYZ().z() < h) &&
                         (
                             (mVertices[t.indicies[(j+1)%3]].GetXYZ().z() > h) &&
                             (mVertices[t.indicies[(j+2)%3]].GetXYZ().z() > h)
                             ))
                        ) {
                    singleIndex = j;
//                    QVector3D p1 = mVertices[t.indicies[singleIndex]].GetXYZ();
//                    QVector3D p2 = mVertices[t.indicies[(singleIndex + 1) % 3]].GetXYZ();
//                    QVector3D p3 = mVertices[t.indicies[(singleIndex + 2) % 3]].GetXYZ();
//                    if (CheckTriangleHeight(p1, p2, p3, h)) {

//                        float percentageOfHeight = (h-p2.z()) / (p1.z()-p2.z());
//                        Vertex v1{p2+(p1-p2)*percentageOfHeight, {1,1,1}};
//                        percentageOfHeight = (h-p3.z()) / (p1.z()-p3.z());
//                        Vertex v2{p3+(p1-p3)*percentageOfHeight, {1,1,1}};
//                        if (v1.GetXYZ().x() < 0 || v1.GetXYZ().y() < 0 || v2.GetXYZ().x() < 0 || v2.GetXYZ().y() < 0) {
//                            std::cout << "something is wrong, i can feel it" << std::endl;
//                        }


//                    }
                }
            }
            QVector3D p1 = mVertices[t.indicies[singleIndex]].GetXYZ();
            QVector3D p2 = mVertices[t.indicies[(singleIndex + 1) % 3]].GetXYZ();
            QVector3D p3 = mVertices[t.indicies[(singleIndex + 2) % 3]].GetXYZ();

            if (CheckTriangleHeight(p1, p2, p3, h)) {

                float percentageOfHeight = (h-p2.z()) / (p1.z()-p2.z());
                Vertex v1{p2+(p1-p2)*percentageOfHeight, {1,1,1}};
                percentageOfHeight = (h-p3.z()) / (p1.z()-p3.z());
                Vertex v2{p3+(p1-p3)*percentageOfHeight, {1,1,1}};
                if (v1.GetXYZ().x() < 0 || v1.GetXYZ().y() < 0 || v2.GetXYZ().x() < 0 || v2.GetXYZ().y() < 0) {
                    std::cout << "something is wrong, i can feel it" << std::endl;
                }
                vertices.push_back(v1);
                vertices.push_back(v2);
            }

        }

        // sjekk om det er 1 eller 2 punkter som er høyere enn høydelinjen, avhengig av det blir det annerledes





        //        while (!drawnTriangles[next]) {

        //            int singleIndex;
        //            for (int j = 0; j < 3; j++) {
        //                if ((mVertices[pointCloud.indicesAndNeighbours[next].indicies[j]].GetXYZ().z() > h) &&
        //                        (
        //                            (mVertices[pointCloud.indicesAndNeighbours[next].indicies[(j+1)%3]].GetXYZ().z() < h) ==
        //                            (mVertices[pointCloud.indicesAndNeighbours[next].indicies[(j+2)%3]].GetXYZ().z() < h))
        //                        ) {
        //                    singleIndex = j;

        //                }
        //            }

        //            QVector3D p1 = mVertices[pointCloud.indicesAndNeighbours[next].indicies[singleIndex]].GetXYZ();
        //            QVector3D p2 = mVertices[pointCloud.indicesAndNeighbours[next].indicies[(singleIndex + 1) % 3]].GetXYZ();
        //            QVector3D p3 = mVertices[pointCloud.indicesAndNeighbours[next].indicies[(singleIndex + 2) % 3]].GetXYZ();


        //            float percentageOfHeight = (h-p2.z()) / (p1.z()-p2.z());
        //            Vertex v1{p2+(p1-p2)*percentageOfHeight, {1,1,1}};
        //            percentageOfHeight = (h-p3.z()) / (p1.z()-p3.z());
        //            Vertex v2{p3+(p1-p3)*percentageOfHeight, {1,1,1}};
        //            vertices.push_back(v1);
        //            vertices.push_back(v2);
        //            drawnTriangles[next] = true;


        //            for (int j = 0; j < 3; j++) {
        //                if (pointCloud.indicesAndNeighbours[next].neighbours[j] < 0) continue;
        //                if (!CheckTriangleHeight(pointCloud.indicesAndNeighbours[next].neighbours[j], h)) continue;
        //                if (drawnTriangles[pointCloud.indicesAndNeighbours[next].neighbours[j]]) continue;
        //                trianglesToDraw.push(pointCloud.indicesAndNeighbours[next].neighbours[j]);

        //            }
        //            next = trianglesToDraw.top();
        //            trianglesToDraw.pop();


        //        }

    }


    return vertices;
}






















































