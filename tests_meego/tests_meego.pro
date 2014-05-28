include(tests_common.pri)
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = \
    TestUtils.pro \
    generaltests.pro \
    clienttests \
    servertests \
    syncelementstests \
    transporttests \

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
    testfiles/respinvalid6.txt \
    testfiles/testconf.xsd \
    testfiles/testconf1.xml \
    testfiles/devinf01.txt \
    testfiles/devinf02.txt \
    testfiles/subcommands01.txt \
    testfiles/resp2.txt

# install
testfiles.files = testfiles/*
testfiles.path = $${INSTALL_TESTDATADIR}
INSTALLS += testfiles
testdefinition.files = tests.xml
testdefinition.path = $${INSTALL_TESTDIR}
INSTALLS += testdefinition
testwrapper.files = runstarget.sh
testwrapper.path = $${INSTALL_TESTDIR}
INSTALLS += testwrapper
