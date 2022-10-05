#include "cloud.h"

Cloud::Cloud(float scale, float worldScale, RegularTriangulation* ground) : mGround{ground}, mWorldScale{worldScale}, mScale{scale}
{
    mVertices.push_back(Vertex{0,0,mScale,1,1,1});
    mVertices.push_back(Vertex{mScale, 0,mScale,1,1,1});
    mVertices.push_back(Vertex{0, mScale,mScale,1,1,1});

    mVertices.push_back(Vertex{mScale, 0,mScale,1,1,1});
    mVertices.push_back(Vertex{0, mScale,mScale,1,1,1});
    mVertices.push_back(Vertex{mScale,mScale,mScale,1,1,1});
}

Cloud::~Cloud()
{
    for (auto p : mRainDrops) {
        delete p;
    }
}

void Cloud::draw()
{
    glBindVertexArray( mVAO );
    glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
}

void Cloud::MakeRaindrop()
{
    float x = rand() % (int)mScale;
    float y = rand() % (int)mScale;
    RainDrop* rainDrop = new RainDrop({x, y, mScale}, mWorldScale, mGround, this, amountOfRainSpawned);
    rainDrop->init(0);
    mRainDrops.push_back(rainDrop);
}

void Cloud::DeleteRainDrop(RainDrop *rd)
{
    for (int i = 0; i < mRainDrops.size(); i++) {
        if (rd == mRainDrops[i]) {
            delete rd;
            mRainDrops.erase(mRainDrops.begin() + i);
            break;
        }
    }
}

void Cloud::init(GLint matrixUniform)
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

void Cloud::Tick(float deltaTime)
{
    static float TimeWithoutRain{0};
    TimeWithoutRain += deltaTime;
    int rainDrops = TimeWithoutRain * mRainAmount;
    if (rainDrops > 0) TimeWithoutRain = 0;
    mTimer += deltaTime;

    for (int i = 0; i < rainDrops; i++) {
        MakeRaindrop();
    }

    if (mRainDrops.size() == 0) return;
    for (int i = mRainDrops.size() - 1; i >= 0; i--) {
        mRainDrops[i]->EvaluateForDeletion();
    }
    for (auto p : mRainDrops) {
        p->Tick(deltaTime);
    }
}
