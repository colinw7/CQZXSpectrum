TEMPLATE = app

TARGET = CQZXSpectrum

DEPENDPATH += .

INCLUDEPATH += . ../include

CONFIG += debug

# Input
SOURCES += \
CQZXSpectrum.cpp \
CZXSpectrumBasic.cpp \
CZXSpectrum.cpp \
CZXSpectrumTZX.cpp \
CZXSpectrumZ80.cpp \

HEADERS += \
CQZXSpectrum.h \
CZXSpectrumKeyMap.h \
CZXSpectrumTZX.h \
CZXSpectrumZ80.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
. ../include \
../../CQUtil/include \
../../CZ80/include \
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
-L../../CQUtil/lib \
-L../../CFont/lib \
-L../../CImageLib/lib \
-L../../CConfig/lib \
-L../../CArgs/lib \
-L../../CZ80/lib \
-L../../CFile/lib \
-L../../COS/lib \
-L../../CStrUtil/lib \
-L../../CRegExp/lib \
-lCQUtil -lCFont -lCImageLib -lCConfig \
-lCArgs -lCZ80 -lCFile -lCOS -lCStrUtil -lCRegExp \
-lpng -ljpeg -ltre
