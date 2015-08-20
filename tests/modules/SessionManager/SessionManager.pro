TESTNAME = session_manager

include (../../testcommon.pri)

PKGCONFIG += process-cpp
CONFIG += exceptions

SOURCES += session_test.cpp \
        session_manager_test.cpp
