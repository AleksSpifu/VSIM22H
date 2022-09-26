#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QWindow>
#include <QOpenGLFunctions_4_1_Core>
#include <QTimer>
#include <QElapsedTimer>
#include <map>
#include <unordered_map>
#include <functional>
//~
//#include <vertex.h>
#include "visualobject.h"
#include "trianglesurface.h"
#include "camera.h"
#include "texture.h"
#include "triangle.h"
#include "xyz.h"

class QOpenGLContext;
class Shader;
class MainWindow;

/// This inherits from QWindow to get access to the Qt functionality and
// OpenGL surface.
// We also inherit from QOpenGLFunctions, to get access to the OpenGL functions
// This is the same as using "glad" and "glw" from general OpenGL tutorials
class RenderWindow : public QWindow, protected QOpenGLFunctions_4_1_Core
{
    Q_OBJECT
public:
    RenderWindow(const QSurfaceFormat &format, MainWindow *mainWindow);
    ~RenderWindow() override;

    QOpenGLContext *context() { return mContext; }

    void exposeEvent(QExposeEvent *) override;  //gets called when app is shown and resized

    bool mRotate{true};     //Check if triangle should rotate
    void Setup();

    void Start();
    void Pause();

private slots:
    void render();          //the actual render - function

private:
    void init();            //initialize things we need before rendering

    QOpenGLContext *mContext{nullptr};  //Our OpenGL context
    bool mInitialized{false};

    void setupPlainShader(int shaderIndex);
    GLint  pMatrixUniform0{-1};
    GLint  vMatrixUniform0{-1};
    GLint  mMatrixUniform0{-1};

    void setupTextureShader(int shaderIndex);
    GLint  pMatrixUniform1{-1};
    GLint  vMatrixUniform1{-1};
    GLint  mMatrixUniform1{-1};
    GLint mTextureUniform{-1};

    void setupPhongShader(int shaderIndex);
    GLint  pMatrixUniform2{-1};
    GLint  vMatrixUniform2{-1};
    GLint  mMatrixUniform2{-1};

    //other light shader variables
    GLint mLightColorUniform{-1};
    GLint mObjectColorUniform{-1};
    GLint mAmbientLightStrengthUniform{-1};
    GLint mLightPositionUniform{-1};
    GLint mCameraPositionUniform{-1};
    GLint mSpecularStrengthUniform{-1};
    GLint mSpecularExponentUniform{-1};
    GLint mLightPowerUniform{-1};
    GLint mTextureUniform2{-1};



    Texture *mTexture[4]{nullptr}; //We can hold 4 textures
    Shader *mShaderProgram[4]{nullptr}; //We can hold 4 shaders


    //
    GLuint mVAO;                        //OpenGL reference to our VAO
    GLuint mVBO;                        //OpenGL reference to our VBO


    //QMatrix4x4 *mMVPmatrix{nullptr};      //  Erstatter denne med



    QTimer *mRenderTimer{nullptr};           //timer that drives the gameloop
    QElapsedTimer mTimeStart;               //time variable that reads the calculated FPS

    MainWindow *mMainWindow{nullptr};        //points back to MainWindow to be able to put info in StatusBar

    class QOpenGLDebugLogger *mOpenGLDebugLogger{nullptr};  //helper class to get some clean debug info from OpenGL
    class Logger *mLogger{nullptr};         //logger - Output Log in the application

    ///Helper function that uses QOpenGLDebugLogger or plain glGetError()
    void checkForGLerrors();

    void calculateFramerate();          //as name says

    ///Starts QOpenGLDebugLogger if possible
    void startOpenGLDebugger();

protected:
    //The QWindow that we inherit from have these functions to capture
    // - mouse and keyboard.
    // Uncomment to use (you also have to make the definitions of
    // these functions in the cpp-file to use them of course!)
    //
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;              //the only one we use now
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    std::unordered_map<int, bool> mCurrentInputs;
    void Tick(float deltaTime);

private:
    std::vector<VisualObject*> mObjects;

    Triangle* triangle{nullptr};

    XYZ* xyz{nullptr};

    class RegularTriangulation *regTriangulation{nullptr};

    Camera mCamera1;
    Camera mCamera2;
    Camera* mActiveCamera{nullptr};
    class Light *mLight{nullptr};

};

#endif // RENDERWINDOW_H
