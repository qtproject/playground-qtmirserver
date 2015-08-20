TARGET = sharedwakelock_test

CONFIG += gtest_own_main

include (../../testcommon.pri)

QT += testlib dbus

#####################################################################################
### libqtdbustest/libqtdbusmock are external packages that depend on Qt5Core. This
### will create a circular dependency when qtmirserver is built as part of Qt.
### Therefore this autotest is not included in the default build.
#####################################################################################

PKGCONFIG += libqtdbusmock-1 libqtdbustest-1


SOURCES += sharedwakelock_test.cpp
