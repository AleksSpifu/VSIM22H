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
#include "LasTextReader.h"
#include "regulartriangulation.h"

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
    mCamera2.SetPosition({43.5, -5.4, 5});
    xyz = new XYZ;
    regTriangulation = new RegularTriangulation("vestlandet.txt");
    mObjects.push_back(regTriangulation);


}

RenderWindow::~RenderWindow()
{
    glDeleteVertexArrays( 1, &mVAO );
    glDeleteBuffers( 1, &mVBO );
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
    //mTexture[2] = new Texture("../VSIM22H/Assets/hund.bmp");

    //Set the textures loaded to a texture unit (also called a texture slot)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture[0]->id());
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, mTexture[1]->id());
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
    mLight->setOrbitPoint({0, 0, 20});

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

    glUseProgram(mShaderProgram[2]->getProgram());
    mActiveCamera->update(pMatrixUniform2, vMatrixUniform2);

    glUniform3f(mLightPositionUniform, mLight->mMatrix.column(3).x(), mLight->mMatrix.column(3).y(),
                 mLight->mMatrix.column(3).z());
     glUniform3f(mCameraPositionUniform, mActiveCamera->GetPosition().x(), mActiveCamera->GetPosition().y(), mActiveCamera->GetPosition().z());
     glUniform3f(mLightColorUniform, mLight->mLightColor.x(), mLight->mLightColor.y(), mLight->mLightColor.z());
     glUniform1f(mSpecularStrengthUniform, mLight->mSpecularStrenght);

    for (auto it=mObjects.begin(); it!= mObjects.end(); it++) {
        glUniformMatrix4fv(mMatrixUniform2, 1, GL_FALSE, (*it)->mMatrix.constData());
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
}

void RenderWindow::mouseReleaseEvent(QMouseEvent *event)
{
    mCurrentInputs[event->button()] = false;
    //    if (event->button() == Qt::LeftButton) {
    //        mCamera.setStatus(false);
    //    }
}

void RenderWindow::mouseMoveEvent(QMouseEvent *event)
{
    static float previousX = event->position().x();
    static float previousY = event->position().y();

    float newX = event->position().x();
    float newY = event->position().y();

    float diffX = previousX - newX;
    float diffY = previousY - newY;

    if (mCurrentInputs[Qt::LeftButton]) {
        mActiveCamera->followMouseMovements(diffX, diffY);
    }

    previousX = newX;
    previousY = newY;
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
    mCamera1.SetPosition({35, 35, 25});
}

// Oppgave 5 og 6 ...
void RenderWindow::Start()
{
    mActiveCamera = &mCamera1;
    mMainWindow->SetStartStopButtonText("Pause");
}

void RenderWindow::Pause()
{
    mActiveCamera = &mCamera2;
    mMainWindow->SetStartStopButtonText("Start");
}
// ...

void RenderWindow::Tick(float deltaTime)
{
    for (auto p : mObjects) {
        //p->Tick(deltaTime);

    }

    QVector3D AttemptedMovement;
    if (mCurrentInputs[Qt::Key_W]) {
        auto dir = mActiveCamera->Forward();
        dir *= -1;
        AttemptedMovement += dir;
    }
    if (mCurrentInputs[Qt::Key_S]) {
        auto dir = mActiveCamera->Forward();
        AttemptedMovement += dir;
    }
    if (mCurrentInputs[Qt::Key_A]) {
        auto dir = mActiveCamera->Right();
        AttemptedMovement += dir;
    }
    if (mCurrentInputs[Qt::Key_D]) {
        auto dir = mActiveCamera->Right();
        dir *= -1;
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

    AttemptedMovement *= deltaTime * 25;
    if (mCurrentInputs[Qt::Key_Shift]) AttemptedMovement *= 2;
    mActiveCamera->mVmatrix.translate(AttemptedMovement);

    mCamera1.Tick(deltaTime);

}
