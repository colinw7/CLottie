TEMPLATE = app

TARGET = CQLottie

QT += widgets

DEPENDPATH += .

QMAKE_CXXFLAGS += \
-std=c++17

MOC_DIR = .moc

CONFIG += c++17

SOURCES += \
CQLottie.cpp \

HEADERS += \
CQLottie.h \

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
