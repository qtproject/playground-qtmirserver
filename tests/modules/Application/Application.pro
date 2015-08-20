TARGET = application_test

include (../../testcommon.pri)

PKGCONFIG += process-cpp

CONFIG += exceptions

SOURCES += application_test.cpp
