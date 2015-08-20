include (testlibs/testlibs.pri)

!gtest_own_main: SOURCES += $$PWD/testlibs/testmain.cpp

CONFIG += testcase
CONFIG += c++11

QT += testlib

TARGET = tst_$$TESTNAME

DEFINES += TEST_NAME=$$TARGET

# if there is a common directory above the test, include it
exists ($$OUT_PWD/../common/common.pri) {
    include ($$OUT_PWD/../common/common.pri)
}
