TARGET = gmock
TEMPLATE = lib
CONFIG += static

CONFIG -= qt

GMOCK_SRC_DIR = /usr/src/gmock
GMOCK_INCLUDE_DIR = /usr/include/gmock

SOURCES = $$GMOCK_SRC_DIR/src/gmock-all.cc




INCLUDEPATH += $$GMOCK_INCLUDE_DIR

### gcc docs claim that quoted includes should come from local directory,
### but that does not seem to work :/
QMAKE_CXXFLAGS += -iquote $$GMOCK_SRC_DIR

