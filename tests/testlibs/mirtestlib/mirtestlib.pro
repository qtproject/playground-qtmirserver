TARGET = mirtestlib
CONFIG += c++11

TEMPLATE = lib
CONFIG += static
DEFINES = QT_MIR_NO_TRACEPOINTS

include (../../../src/modules/Unity/Application/Application.pri)
