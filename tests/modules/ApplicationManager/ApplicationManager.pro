TESTNAME = application_manager

include (../../testcommon.pri)

PKGCONFIG += process-cpp

CONFIG += exceptions

SOURCES += application_manager_test.cpp
