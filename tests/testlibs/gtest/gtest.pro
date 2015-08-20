TARGET = gtest
TEMPLATE = lib
CONFIG += static

CONFIG -= qt

GTEST_SRC_DIR = /usr/src/gtest
GTEST_INCLUDE_DIR = /usr/include/gtest

SOURCES = $$GTEST_SRC_DIR/src/gtest-all.cc




INCLUDEPATH += $$GTEST_INCLUDE_DIR

### gcc docs claim that quoted includes should come from local directory,
### but that does not seem to work :/
QMAKE_CXXFLAGS += -iquote $$GTEST_SRC_DIR

