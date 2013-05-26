TEMPLATE = app

TARGET = 
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
CQUtil.cpp \

HEADERS += \
CQZXSpectrum.h \
CZXSpectrumKeyMap.h \
CZXSpectrumTZX.h \
CZXSpectrumZ80.h \
CQUtil.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += ../include

unix:LIBS += \
-L$$LIB_DIR \
-lCArgs -lCZ80 -lCFile -lCOS -lCStrUtil
