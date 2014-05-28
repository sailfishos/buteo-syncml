include(tests_common.pri)
TEMPLATE = subdirs
SUBDIRS = \
    AlertPackageTest.pro \
    AuthenticationPackageTest.pro \
    AuthHelperTest.pro \
    CommandHandlerTest.pro \
    ConflictResolverTest.pro \
    DevInfHandlerTest.pro \
    DevInfPackageTest.pro \
    FinalPackageTest.pro \
    ChangeLogTest.pro \
    LocalChangesPackageTest.pro \
    LocalMappingsPackageTest.pro \
    NonceStorageTest.pro \
    ResponseGeneratorTest.pro \
    SANTest.pro \
    SessionHandlerTest.pro \
    StorageHandlerTest.pro \
    SyncAgentConfigTest.pro \
    SyncAgentTest.pro \
    SyncItemPrefetcherTest.pro \
    SyncModeTest.pro \
    SyncResultTest.pro \
    SyncTargetTest.pro \

    # Dead code?
    #ChangeLogHandlerTest.pro
