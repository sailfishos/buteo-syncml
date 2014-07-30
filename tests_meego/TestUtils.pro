include(tests_common.pri)
TEMPLATE = lib
CONFIG += staticlib
TARGET = TestUtils

HEADERS = \
    Mock.h \
    TestUtils.h \

SOURCES = \
    TestUtils.cpp \
