TEMPLATE = app

TARGET = CQZXSpectrum

QT += widgets

DEPENDPATH += .

INCLUDEPATH += . ../include

QMAKE_CXXFLAGS += -std=c++11

CONFIG += debug

# Input
SOURCES += \
CQZXSpectrum.cpp \
CZXSpectrum.cpp \
CZXSpectrumBasic.cpp \
CZXSpectrumTZX.cpp \
CZXSpectrumZ80.cpp \

HEADERS += \
CQZXSpectrum.h \
../include/CZXSpectrum.h \
CZXSpectrumKeyMap.h \
CZXSpectrumTZX.h \
CZXSpectrumZ80.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
. ../include \
../../CZ80/include \
../../CQZ80Dbg/include \
../../CQUtil/include \
../../CArgs/include \
../../CFont/include \
../../CImageLib/include \
../../CFile/include \
../../COS/include \
../../CUtil/include \
../../CMath/include \
../../CStrUtil/include \

unix:LIBS += \
-L$$LIB_DIR \
-L../../CZ80/lib \
-L../../CQZ80Dbg/lib \
-L../../CQUtil/lib \
-L../../CFont/lib \
-L../../CImageLib/lib \
-L../../CConfig/lib \
-L../../CArgs/lib \
-L../../CFileUtil/lib \
-L../../CFile/lib \
-L../../CMath/lib \
-L../../COS/lib \
-L../../CStrUtil/lib \
-L../../CRegExp/lib \
-L../../CUtil/lib \
-lCZ80 -lCQZ80Dbg -lCQUtil -lCFont -lCImageLib -lCConfig \
-lCArgs -lCFileUtil -lCFile -lCMath -lCUtil -lCOS -lCStrUtil -lCRegExp \
-lpng -ljpeg -ltre
