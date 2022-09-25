QT          += core gui widgets opengl

TEMPLATE    = app
CONFIG      += c++17

TARGET      = 3D-programmering

SOURCES += main.cpp \
    camera.cpp \
    light.cpp \
    logger.cpp \
    mainwindow.cpp \
    octahedronball.cpp \
    renderwindow.cpp \
    shader.cpp \
    texture.cpp \
    triangle.cpp \
    trianglesurface.cpp \
    vertex.cpp \
    visualobject.cpp \
    xyz.cpp

HEADERS += \
    light.h \
    camera.h \
    logger.h \
    mainwindow.h \
    octahedronball.h \
    renderwindow.h \
    shader.h \
    texture.h \
    triangle.h \
    trianglesurface.h \
    vertex.h \
    visualobject.h \
    xyz.h

FORMS += \
    mainwindow.ui

DISTFILES += \
    phongshader.frag \
    phongshader.vert \
    plainshader.frag \
    plainshader.vert \
    textureshader.frag \
    textureshader.vert
