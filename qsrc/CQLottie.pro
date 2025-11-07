TEMPLATE = app

TARGET = CQLottie

QT += widgets

DEPENDPATH += .

QMAKE_CXXFLAGS += \
-std=c++17

MOC_DIR = .moc

CONFIG += c++17

SOURCES += \
CQLottieMain.cpp \
CQLottie.cpp \
CQLottieCanvas.cpp \
CQLottieTree.cpp \
CQLottieSettings.cpp \
CQLottieToolBar.cpp \
CQLottieStatusBar.cpp \
CQLottieTimeLine.cpp \

HEADERS += \
CQLottieMain.h \
CQLottie.h \
CQLottieCanvas.h \
CQLottieTree.h \
CQLottieSettings.h \
CQLottieToolBar.h \
CQLottieStatusBar.h \
CQLottieTimeLine.h \

OBJECTS_DIR = ../obj

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
../include \
../../CQUtil/include \
../../CJson/include \
../../CUtil/include \
../../CMath/include \
../../CStrUtil/include \
../../COS/include \

PRE_TARGETDEPS = \
../lib/libCLottie.a \

unix:LIBS += \
-L../lib \
-L../../CQUtil/lib \
-L../../CJson/lib \
-L../../CUtil/lib \
-L../../CFile/lib \
-L../../CMath/lib \
-L../../CStrUtil/lib \
-L../../CRGBName/lib \
-L../../COS/lib \
-lCQUtil -lCLottie -lCJson -lCUtil -lCFile -lCMath -lCStrUtil -lCRGBName -lCOS
