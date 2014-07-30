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

OTHER_FILES += \
    data/transport_initrequest_nohdr.txt \
    data/transport_initrequest.txt \
    data/toXMLtest.txt \
    data/testURI.txt \
    data/testMap.txt \
    data/testAlert.txt \
    data/syncml_resp5.txt \
    data/syncml_resp4.txt \
    data/syncml_resp3.txt \
    data/syncml_resp2.txt \
    data/syncml_resp.txt \
    data/syncml_init.txt \
    data/SyncMLHdrTest_11.txt \
    data/SyncMLHdrTest.txt \
    data/resp.txt \
    data/basicbasetransport.txt \
    data/basicbasetransport.bin \
    data/basicbasetransport2.txt \
    data/cmdhandler_put.txt \
    data/cmdhandler_get.txt \
    data/SAN01.bin \
    data/SAN02.bin \
    data/obexresp01.bin \
    data/obexresp02.bin \
    data/obexresp03.bin \
    data/respinvalid1.txt \
    data/respinvalid2.txt \
    data/respinvalid3.txt \
    data/respinvalid4.txt \
    data/respinvalid5.txt \
    data/respinvalid6.txt \
    data/testconf.xsd \
    data/testconf1.xml \
    data/devinf01.txt \
    data/devinf02.txt \
    data/subcommands01.txt \
    data/resp2.txt

# install
testdata.files = data/*
testdata.path = $${INSTALL_TESTDATADIR}
INSTALLS += testdata
testdefinition.files = tests.xml
testdefinition.path = $${INSTALL_TESTDIR}
INSTALLS += testdefinition
testwrapper.files = runstarget.sh
testwrapper.path = $${INSTALL_TESTDIR}
INSTALLS += testwrapper
