TESTNAME = application

include (../../testcommon.pri)

PKGCONFIG += process-cpp

CONFIG += exceptions

SOURCES += application_test.cpp
