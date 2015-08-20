TARGET = application_manager_test

include (../../testcommon.pri)

PKGCONFIG += process-cpp

CONFIG += exceptions

SOURCES += application_manager_test.cpp
