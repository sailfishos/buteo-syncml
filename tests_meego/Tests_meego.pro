TEMPLATE = app
TARGET = libmeegosyncml-tests
include(syncelementstests/syncelementstests.pro)
include(servertests/servertests.pro)
include(clienttests/clienttests.pro)
include(transporttests/transporttests.pro)
INCLUDEPATH += . \
    ../src/ \
    ../src/syncelements \
    ../src/transport \
    ../src/server \
    ../src/client \
    ../testapp/common \
    /usr/include/libsynccommon \
    syncelementstests \
    servertests \
    clienttests \
    transporttests
DEPENDPATH += . \
    ../testapp/common \
    syncelementstests \
    servertests \
    clienttests \
    transporttests
CONFIG += qtestlib \
    link_prl \
    debug \
    silent

# DEFINES += QT_NO_DEBUG_OUTPUT
# use the meegosyncml library objects directly to get better debugging data
# So if remember to compile them when debugging
LIBS += -lmeegosyncml \
    -L../src/
#         -lsynccommon \
#     -L../../sync-fw/libsynccommon/ \    

# This is needed to avoid adding the /usr/lib link directory before the
# newer version in meegosyncml
QMAKE_LIBDIR_QT = ../src \
    $$QMAKE_LIBDIR_QT
QT += testlib \
    core \
    sql \
    network
QT -= gui

# Input
HEADERS += SyncAgentTest.h \
    ChangeLogTest.h \
    Mock.h \
    TestRunner.h \
    TestLoader.h \
    AlertPackageTest.h \
    AuthenticationPackageTest.h \
    DevInfPackageTest.h \
    FinalPackageTest.h \
    LocalChangesPackageTest.h \
    LocalMappingsPackageTest.h \
    StorageHandlerTest.h \
    SessionHandlerTest.h \
    ConflictResolverTest.h \
    SyncTargetTest.h \
    SyncResultTest.h \
    SyncAgentConfigTest.h \
    SyncModeTest.h \
    AuthHelperTest.h \
    ResponseGeneratorTest.h \
    NonceStorageTest.h \
    CommandHandlerTest.h \
    TestUtils.h \
    SANTest.h 
SOURCES += Tests.cpp \
    SyncAgentTest.cpp \
    ChangeLogTest.cpp \
    TestRunner.cpp \
    AlertPackageTest.cpp \
    AuthenticationPackageTest.cpp \
    DevInfPackageTest.cpp \
    FinalPackageTest.cpp \
    LocalChangesPackageTest.cpp \
    LocalMappingsPackageTest.cpp \
    StorageHandlerTest.cpp \
    SessionHandlerTest.cpp \
    ConflictResolverTest.cpp \
    SyncTargetTest.cpp \
    SyncResultTest.cpp \
    SyncAgentConfigTest.cpp \
    SyncModeTest.cpp \
    AuthHelperTest.cpp \
    ResponseGeneratorTest.cpp \
    NonceStorageTest.cpp \
    CommandHandlerTest.cpp \
#    ../testapp/common/Logger.cpp \
    TestUtils.cpp \
    SANTest.cpp 
QMAKE_CLEAN += tests-junit.xml \
    changelog-test.db \
    changelog-test2.db \
    Storagetest.db \
    results/* \
    changelog.db \
    moc_* \
    *.o
OTHER_FILES += testfiles/transport_initrequest_nohdr.txt \
    testfiles/transport_initrequest.txt \
    testfiles/toXMLtest.txt \
    testfiles/testURI.txt \
    testfiles/testMap.txt \
    testfiles/testAlert.txt \
    testfiles/syncml_resp5.txt \
    testfiles/syncml_resp4.txt \
    testfiles/syncml_resp3.txt \
    testfiles/syncml_resp2.txt \
    testfiles/syncml_resp.txt \
    testfiles/syncml_init.txt \
    testfiles/SyncMLHdrTest_11.txt \
    testfiles/SyncMLHdrTest.txt \
    testfiles/resp.txt \
    testfiles/basicbasetransport.txt \
    testfiles/basicbasetransport.bin \
    testfiles/basicbasetransport2.txt \
    testfiles/cmdhandler_put.txt \
    testfiles/cmdhandler_get.txt \
    testfiles/SAN01.bin \
    testfiles/SAN02.bin \
    testfiles/obexresp01.bin \
    testfiles/obexresp02.bin \
    testfiles/obexresp03.bin \
    testfiles/respinvalid1.txt \
    testfiles/respinvalid2.txt \
    testfiles/respinvalid3.txt \
    testfiles/respinvalid4.txt \
    testfiles/respinvalid5.txt \
    testfiles/respinvalid6.txt
    
QMAKE_CXXFLAGS += -Wall \
    -g

# install
testfiles.files = testfiles/*
tests.files = tests.xml \
    runstarget.sh \
    libmeegosyncml-tests.ref
tests.path = /usr/share/libmeegosyncml-tests/
target.path = /usr/bin/
testfiles.path = /usr/share/libmeegosyncml-tests/testfiles/
INSTALLS += tests \
    target \
    testfiles
