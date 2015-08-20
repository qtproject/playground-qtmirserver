TARGET = taskcontroller_test

include (../../testcommon.pri)

PKGCONFIG += process-cpp
CONFIG += exceptions

SOURCES += taskcontroller_test.cpp
