#include "renderwindow.h"
#include <QTimer>
#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLDebugLogger>
#include <QKeyEvent>
#include <QStatusBar>
#include <QDebug>

#include <string>
#include <chrono>
#include <cmath>

#include "shader.h"
#include "mainwindow.h"
#include "logger.h"
//~~
#include "xyz.h"
#include "texture.h"
#include "light.h"
#include "lastextreader.h"
#include "heightlines.h"
#include "regulartriangulation.h"
#include "raindrop.h"
#include "cloud.h"

// hei
//~~//

float MinsteKvadratersFunksjon(float x) {
    return ((1943.f/22032.f) * pow(x, 2)) - ((2443.f/66096.f) * x) + (8167.f/8262.f);
}

float InterpolasjonFunksjon(float x) {
    return (-5.f/12.f) * pow(x, 3) + (51.f/8.f) * pow(x, 2) - (349.f/12.f) * x + 43;
}

RenderWindow::RenderWindow(const QSurfaceFormat &format, MainWindow *mainWindow)
    : mContext(nullptr), mInitialized(false), mMainWindow(mainWindow)

{
    mActiveCamera = &mCamera2;
    //This is sent to QWindow:
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(format);
    //Make the OpenGL context
    mContext = new QOpenGLContext(this);
    //Give the context the wanted OpenGL format (v4.1 Core)
    mContext->setFormat(requestedFormat());
    if (!mContext->create()) {
        delete mContext;
        mContext = nullptr;
        qDebug() << "Context could not be made - quitting this application";
    }

    mRenderTimer = new QTimer(this);

    xyz = new XYZ;
    regTriangulation = new RegularTriangulation("vestlandet_stor.txt");
    mObjects.push_back(regTriangulation);

    heightLines = new HeightLines;
    heightLines->setVertices(regTriangulation->MakeHeightLines(20));
    //mObjects.push_back(hl);

    mCloud = new Cloud(500.f, regTriangulation->scale, regTriangulation);

    //mCloud->mRainDrops.push_back(new RainDrop({200,200,100}, regTriangulation->scale, regTriangulation));


}

RenderWindow::~RenderWindow()
{
    glDeleteVertexArrays( 1, &mVAO );
    glDeleteBuffers( 1, &mVBO );
    delete heightLines;
    delete mCloud;
    for (auto p : mObjects) {
        delete p;
    }
}


void RenderWindow::init()
{
    mLogger = Logger::getInstance();

    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(render()));

    if (!mContext->makeCurrent(this)) {
        mLogger->logText("makeCurrent() failed", LogType::REALERROR);
        return;
    }

    if (!mInitialized)
        mInitialized = true;

    initializeOpenGLFunctions();

    mLogger->logText("The active GPU and API:", LogType::HIGHLIGHT);
    std::string tempString;
    tempString += std::string("  Vendor: ") + std::string((char*)glGetString(GL_VENDOR)) + "\n" +
            std::string("  Renderer: ") + std::string((char*)glGetString(GL_RENDERER)) + "\n" +
            std::string("  Version: ") + std::string((char*)glGetString(GL_VERSION));
    mLogger->logText(tempString);

    int textureUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &textureUnits);
    tempString += std::string("  This GPU as ") + std::to_string(textureUnits) + std::string(" texture units / slots in total, \n");

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureUnits);
    tempString += std::string("  and supports ") + std::to_string(textureUnits) + std::string(" texture units pr shader");

    mLogger->logText(tempString);

    startOpenGLDebugger();

    glEnable(GL_DEPTH_TEST);
    //    glEnable(GL_CULL_FACE);       //draws only front side of models - usually what you want - test it out!
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    //set up alpha blending for textures
    glEnable(GL_BLEND);// you enable blending function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //********************** Compile shaders: **********************
    //NB: hardcoded path to files! You have to change this if you change directories for the project.
    //Qt makes a build-folder besides the project folder. That is why we go down one directory
    // (out of the build-folder) and then up into the project folder.
    mShaderProgram[0] = new Shader("../VSIM22H/plainshader.vert", "../VSIM22H/plainshader.frag");
    mLogger->logText("Plain shader program id: " + std::to_string(mShaderProgram[0]->getProgram()) );
    mShaderProgram[1]= new Shader("../VSIM22H/textureshader.vert", "../VSIM22H/textureshader.frag");
    mLogger->logText("Texture shader program id: " + std::to_string(mShaderProgram[1]->getProgram()) );
    mShaderProgram[2] = new Shader("../VSIM22H/phongshader.vert", "../VSIM22H/phongshader.frag");
    mLogger->logText("Texture shader program id: " + std::to_string(mShaderProgram[2]->getProgram()));

    setupPlainShader(0);
    setupTextureShader(1);
    setupPhongShader(2);

    //********************** Texture stuff: **********************
    //Returns a pointer to the Texture class. This reads and sets up the texture for OpenGL
    //and returns the Texture ID that OpenGL uses from Texture::id()
    mTexture[0] = new Texture();
    //mTexture[1] = new Texture("../VSIM22H/Assets/gress.bmp");
    mTexture[1] = new Texture("../VSIM22H/Assets/test2.bmp");

    //Set the textures loaded to a texture unit (also called a texture slot)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture[0]->id());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTexture[1]->id());
    //glActiveTexture(GL_TEXTURE2);
    //glBindTexture(GL_TEXTURE_2D, mTexture[2]->id());

    mCamera1.init(pMatrixUniform0, vMatrixUniform0);
    mCamera1.perspective(60, (float)width()/(float)height(), 0.1, 1000.0);

    mCamera2.init(pMatrixUniform0, vMatrixUniform0);
    mCamera2.perspective(60, (float)width()/(float)height(), 0.1, 1000.0);


    for (auto i = mObjects.begin(); i != mObjects.end(); i++)
    {
        (*i)->init(mMatrixUniform0);
    }


    mLight = new Light();
    mLight->init(2);
    mLight->setOrbitPoint({250, 250, 400});

    heightLines->init(0);

    mCloud->init(0);

    xyz->init(0);

    glBindVertexArray(0);

    Setup();

    mLight->orbit(0.1f);

}

void RenderWindow::render()
{
    mTimeStart.restart(); //restart FPS clock
    mContext->makeCurrent(this); //must be called every frame (every time mContext->swapBuffers is called)

    initializeOpenGLFunctions();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto time1 = std::chrono::system_clock::now();
    static auto time2 = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> duration = time1 - time2;
    time2 = std::chrono::system_clock::now();

    Tick(duration.count() / 1000);

    glUseProgram(mShaderProgram[0]->getProgram() );
    mActiveCamera->update(pMatrixUniform0, vMatrixUniform0);

    glUniformMatrix4fv(mMatrixUniform0, 1, GL_FALSE, mLight->mMatrix.constData());
    mLight->draw();

    if (DrawHeightLines) {
        glUniformMatrix4fv(mMatrixUniform0, 1, GL_FALSE, heightLines->mMatrix.constData());
        heightLines->draw();
    }

    for (auto i = mCloud->mRainDrops.begin(); i != mCloud->mRainDrops.end(); i++)
    {
        glUniformMatrix4fv(mMatrixUniform0, 1, GL_FALSE, (*i)->mMatrix.constData());
        (*i)->draw();
    }

    for (auto i = mCloud->mRainTrails.begin(); i != mCloud->mRainTrails.end(); i++)
    {
        glUniformMatrix4fv(mMatrixUniform0, 1, GL_FALSE, (*i)->mMatrix.constData());
        (*i)->draw();
    }

    glUniformMatrix4fv(mMatrixUniform0, 1, GL_FALSE, mCloud->mMatrix.constData());
    mCloud->draw();

    glUseProgram(mShaderProgram[2]->getProgram());
    mActiveCamera->update(pMatrixUniform2, vMatrixUniform2);

    glUniform3f(mLightPositionUniform, mLight->mMatrix.column(3).x(), mLight->mMatrix.column(3).y(),
                 mLight->mMatrix.column(3).z());
     glUniform3f(mCameraPositionUniform, mActiveCamera->GetPosition().x(), mActiveCamera->GetPosition().y(), mActiveCamera->GetPosition().z());
     glUniform3f(mLightColorUniform, mLight->mLightColor.x(), mLight->mLightColor.y(), mLight->mLightColor.z());
     glUniform1f(mSpecularStrengthUniform, mLight->mSpecularStrenght);


     glUseProgram(mShaderProgram[2]->getProgram());
     mActiveCamera->update(pMatrixUniform2, vMatrixUniform2);

    for (auto it=mObjects.begin(); it!= mObjects.end(); it++) {
        glUniformMatrix4fv(mMatrixUniform2, 1, GL_FALSE, (*it)->mMatrix.constData());
        glUniform1i(mTextureUniform2, 1);
        (*it)->draw();
    }

    calculateFramerate();

    checkForGLerrors();

    mContext->swapBuffers(this);
}

void RenderWindow::setupPlainShader(int shaderIndex)
{
    mMatrixUniform0 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform0 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform0 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "pMatrix" );
}

void RenderWindow::setupTextureShader(int shaderIndex)
{
    mMatrixUniform1 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform1 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform1 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "pMatrix" );
    mTextureUniform = glGetUniformLocation(mShaderProgram[shaderIndex]->getProgram(), "textureSampler");
}

void RenderWindow::setupPhongShader(int shaderIndex)
{
    mMatrixUniform2 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform2 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform2 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "pMatrix" );

    mLightColorUniform = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "lightColor" );
    mObjectColorUniform = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "objectColor" );
    mAmbientLightStrengthUniform = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "ambientStrengt" );
    mLightPositionUniform = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "lightPosition" );
    mSpecularStrengthUniform = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "specularStrength" );
    mSpecularExponentUniform = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "specularExponent" );
    mLightPowerUniform = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "lightPower" );
    mCameraPositionUniform = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "cameraPosition" );
    mTextureUniform2 = glGetUniformLocation(mShaderProgram[shaderIndex]->getProgram(), "textureSampler");
}

void RenderWindow::exposeEvent(QExposeEvent *)
{
    if (!mInitialized)
        init();

    const qreal retinaScale = devicePixelRatio();

    glViewport(0, 0, static_cast<GLint>(width() * retinaScale), static_cast<GLint>(height() * retinaScale));

    if (isExposed())
    {
        mRenderTimer->start(16);
        mTimeStart.start();
    }
}

void RenderWindow::calculateFramerate()
{
    long nsecElapsed = mTimeStart.nsecsElapsed();
    static int frameCount{0};

    if (mMainWindow)
    {
        ++frameCount;
        if (frameCount > 30)
        {
            mMainWindow->statusBar()->showMessage(" Time pr FrameDraw: " +
                                                  QString::number(nsecElapsed/1000000.f, 'g', 4) + " ms  |  " +
                                                  "FPS (approximated): " + QString::number(1E9 / nsecElapsed, 'g', 7));
            frameCount = 0;
        }
    }
}

void RenderWindow::checkForGLerrors()
{
    if(mOpenGLDebugLogger)
    {
        const QList<QOpenGLDebugMessage> messages = mOpenGLDebugLogger->loggedMessages();
        for (const QOpenGLDebugMessage &message : messages)
        {
            if (!(message.type() == message.OtherType))
                mLogger->logText(message.message().toStdString(), LogType::REALERROR);
        }
    }
    else
    {
        GLenum err = GL_NO_ERROR;
        while((err = glGetError()) != GL_NO_ERROR)
        {
            mLogger->logText("glGetError returns " + std::to_string(err), LogType::REALERROR);
            switch (err) {
            case 1280:
                mLogger->logText("GL_INVALID_ENUM - Given when an enumeration parameter is not a "
                                 "legal enumeration for that function.");
                break;
            case 1281:
                mLogger->logText("GL_INVALID_VALUE - Given when a value parameter is not a legal "
                                 "value for that function.");
                break;
            case 1282:
                mLogger->logText("GL_INVALID_OPERATION - Given when the set of state for a command "
                                 "is not legal for the parameters given to that command. "
                                 "It is also given for commands where combinations of parameters "
                                 "define what the legal parameters are.");
                break;
            }
        }
    }
}

void RenderWindow::startOpenGLDebugger()
{
    QOpenGLContext * temp = this->context();
    if (temp)
    {
        QSurfaceFormat format = temp->format();
        if (! format.testOption(QSurfaceFormat::DebugContext))
            mLogger->logText("This system can not use QOpenGLDebugLogger, so we revert to glGetError()",
                             LogType::HIGHLIGHT);

        if(temp->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
        {
            mLogger->logText("This system can log extended OpenGL errors", LogType::HIGHLIGHT);
            mOpenGLDebugLogger = new QOpenGLDebugLogger(this);
            if (mOpenGLDebugLogger->initialize()) // initializes in the current context
                mLogger->logText("Started Qt OpenGL debug logger");
        }
    }
}

void RenderWindow::mousePressEvent(QMouseEvent *event)
{
    mCurrentInputs[event->button()] = true;
    //    if (event->button() == Qt::LeftButton) {
    //        mCamera.setStatus(true);
    //    }
    if (event->button() == Qt::RightButton) {
        setCursor(Qt::BlankCursor);
        mMouseHeldPosition = event->pos();
    }
    if (event->button() == Qt::LeftButton) {
        QVector3D pos = ScreenToWorld(event->pos().x(), event->pos().y());
        auto rain = new RainDrop(pos, regTriangulation->pointCloud.scale, regTriangulation, mCloud, mCloud->amountOfRainSpawned, 5);
        rain->init(0);
        mCloud->mRainDrops.push_back(rain);
    }
}

void RenderWindow::mouseReleaseEvent(QMouseEvent *event)
{
    mCurrentInputs[event->button()] = false;
    //    if (event->button() == Qt::LeftButton) {
    //        mCamera.setStatus(false);
    //    }
    if (event->button() == Qt::RightButton) {
        setCursor(Qt::ArrowCursor);
    }
}

QVector3D RenderWindow::ScreenToWorld(float x, float y)
{
    float halfScreenWidth = width() / 2.f;
    float halfScreenHeight = height() / 2.f;

    QMatrix4x4 mat = (mActiveCamera->mPmatrix * mActiveCamera->mVmatrix).inverted();

    QVector4D n((x - halfScreenWidth) / halfScreenWidth, -1 * (y - halfScreenHeight) / halfScreenHeight, -1, 1);
    QVector4D f((x - halfScreenWidth) / halfScreenWidth, -1 * (y - halfScreenHeight) / halfScreenHeight, 1, 1);

    QVector4D nearResult = mat.map(n);
    QVector4D farResult = mat.map(f);

    nearResult /= nearResult.w();
    farResult /= farResult.w();

    QVector3D dir = (farResult-nearResult).toVector3D();

    dir.normalize();

    QVector3D out = mActiveCamera->GetPosition();

    for (int i = 0; i < 1500; i++) {
        out += dir * 1;
        if (out.x() < 500.f && out.y() < 500.f && out.x() > 0.f && out.y() > 0.f) {

            Las::Triangle tri = regTriangulation->GetTriangle(out.x(), out.y());

            QVector3D p1 = regTriangulation->pointCloud.vertices[tri.indicies[0]].GetXYZ();
            QVector3D p2 = regTriangulation->pointCloud.vertices[tri.indicies[1]].GetXYZ();
            QVector3D p3 = regTriangulation->pointCloud.vertices[tri.indicies[2]].GetXYZ();
            QVector3D Baryc = regTriangulation->Barycentric(out, p1, p2, p3);
            float groundHeight = regTriangulation->GetBarycentricHeight(Baryc, p1, p2, p3);
            if (groundHeight > out.z()) break;
        }
    }

    return out;

}

void RenderWindow::mouseMoveEvent(QMouseEvent *event)
{
    static float previousX = event->position().x();
    static float previousY = event->position().y();

    float newX = event->position().x();
    float newY = event->position().y();

    ScreenToWorld(newX, newY);


    float diffX = previousX - newX;
    float diffY = previousY - newY;

    if (mCurrentInputs[Qt::RightButton]) {
        mActiveCamera->firstPersonMouseMove(diffX, diffY);
        QCursor::setPos(mapToGlobal(mMouseHeldPosition));
    } else {
        previousX = newX;
        previousY = newY;
    }


}

void RenderWindow::keyPressEvent(QKeyEvent *event)
{
    mCurrentInputs[event->key()] = true;

    if (event->key() == Qt::Key_Escape)
    {
        mMainWindow->close();
    }


}

void RenderWindow::keyReleaseEvent(QKeyEvent *event)
{
    mCurrentInputs[event->key()] = false;

}

void RenderWindow::wheelEvent(QWheelEvent *event)
{
    mActiveCamera->Zoom(event->angleDelta().y());
}

void RenderWindow::Setup() {

    mCamera2.SetPosition({250,500,150});
    mCamera2.lookAt({250,250,0});

}

void RenderWindow::ResetCamera()
{
    mCamera2.SetPosition({0,0,150});
    mCamera2.lookAt({250,250,0});

}

void RenderWindow::TellCloudRainAmount(int amount)
{
    mCloud->mRainAmount = amount;
}

void RenderWindow::Tick(float deltaTime)
{
    for (auto p : mObjects) {
        //p->Tick(deltaTime);

    }

    mCloud->Tick(deltaTime);

    QVector3D AttemptedMovement;
    if (mCurrentInputs[Qt::Key_W]) {
        auto dir = mActiveCamera->Forward();
        AttemptedMovement += dir;
    }
    if (mCurrentInputs[Qt::Key_S]) {
        auto dir = mActiveCamera->Forward();
        dir *= -1;
        AttemptedMovement += dir;
    }
    if (mCurrentInputs[Qt::Key_A]) {
        auto dir = mActiveCamera->Right();
        dir *= -1;
        AttemptedMovement += dir;
    }
    if (mCurrentInputs[Qt::Key_D]) {
        auto dir = mActiveCamera->Right();
        AttemptedMovement += dir;
    }
    if (mCurrentInputs[Qt::Key_E]) {
        QVector3D dir = {0,0,-1};
        AttemptedMovement += dir;
    }
    if (mCurrentInputs[Qt::Key_Q]) {
        QVector3D dir = {0,0,1};
        AttemptedMovement += dir;
    }

    mLight->orbit(deltaTime);

    float moveSpeed = deltaTime * 50;
    if (mCurrentInputs[Qt::Key_Shift]) moveSpeed *= 2;
    mActiveCamera->Move(AttemptedMovement, moveSpeed);

    mCamera1.Tick(deltaTime);

}
