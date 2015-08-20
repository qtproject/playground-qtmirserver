include (testlibs/testlibs.pri)

SOURCES += $$PWD/testlibs/testmain.cpp

CONFIG += testcase
CONFIG += c++11

# if there is a common directory above the test, include it
exists ($$OUT_PWD/../common/common.pri) {
    include ($$OUT_PWD/../common/common.pri)
}
