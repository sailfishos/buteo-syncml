include(../tests_common.pri)
TEMPLATE = subdirs
SUBDIRS = \
    BaseTransportTest.pro \
    ClientWorkerTest.pro \
    HTTPTransportTest.pro \
    OBEXTransportTest.pro \
    ServerWorkerTest.pro \

# Dead code?
#SocketPair.cpp
#SocketPair.h
