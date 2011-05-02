include(syncelements/syncelements.pro)
include(server/server.pro)
include(client/client.pro)
include(transport/transport.pro)

TEMPLATE = lib
TARGET = meegosyncml

VER_MAJ = 0
VER_MIN = 4
VER_PAT = 6

CONFIG += dll \ 
    debug \
    create_pc \
    create_prl

INCLUDEPATH += . \
	syncelements \
	server \
	client \
	transport

DEPENDPATH += . \
	syncelements \
	server \
	client \
	transport

SOURCES += SyncItem.cpp \
	ChangeLog.cpp \
	SuspendLog.cpp \
	SyncAgent.cpp \
	SyncAgentConfig.cpp \
	SyncMLMessageParser.cpp \
	AuthenticationPackage.cpp \
	LocalChangesPackage.cpp \
	LocalMappingsPackage.cpp \
	DeviceInfo.cpp \
	RemoteDeviceInfo.cpp \
	SyncTarget.cpp \
	FinalPackage.cpp \
	AlertPackage.cpp \
	SyncResults.cpp \
	SessionHandler.cpp \
	CommandHandler.cpp \
	StorageHandler.cpp \
	ResponseGenerator.cpp \
	SyncMode.cpp \
	DevInfPackage.cpp \
	DatabaseHandler.cpp \
	ConflictResolver.cpp \
	AuthHelper.cpp \
	NonceStorage.cpp \
	ServerAlertedNotification.cpp \
	RequestListener.cpp \
    DevInfHandler.cpp \
    SyncItemPrefetcher.cpp \
    CTCap.cpp \
    DataStore.cpp \
    StorageContentFormatInfo.cpp \
    SessionAuthentication.cpp \
    SessionParams.cpp

HEADERS += SyncItem.h \
	StoragePlugin.h \
	ChangeLog.h \
	SuspendLog.h \
	SyncAgent.h \
	SyncItemKey.h \
	datatypes.h \
    Fragments.h \
	SyncAgentConfig.h \
	SyncMLMessageParser.h \
	AuthenticationPackage.h \
	LocalChangesPackage.h \
	LocalMappingsPackage.h \
	Package.h \
	DeviceInfo.h \
	RemoteDeviceInfo.h \
	SyncAgentConsts.h \
	SyncResults.h \
	SyncTarget.h \
	FinalPackage.h \
	AlertPackage.h \
	SyncMLCommand.h \
	SessionHandler.h \
	CommandHandler.h \
	StorageHandler.h \
	ResponseGenerator.h \
	SyncMode.h \
	DevInfPackage.h \
	DatabaseHandler.h \
	ConflictResolver.h \
	AuthHelper.h \
	NonceStorage.h \
	LogMacros.h \
	StorageProvider.h \
	ServerAlertedNotification.h \
    SyncMLGlobals.h \
    RequestListener.h \
    DevInfHandler.h \
    SyncAgentConfigProperties.h \
    SyncItemPrefetcher.h \
    CTCap.h \
    DataStore.h \
    StorageContentFormatInfo.h \
    LocalChanges.h \
    SessionAuthentication.h \
    SessionParams.h
    
OTHER_FILES += config/meego-syncml-conf.xsd \
               config/meego-syncml-conf.xml

LIBS += -lsqlite3 -lwbxml2 -lopenobex

QTDIR = /usr/lib/qt4

QT += network \
    xml \
    sql \
    xmlpatterns

QT -= gui

# TODO extract DEBUG
QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

QMAKE_CLEAN += lib*.so* \
    lib$${TARGET}.prl pkgconfig/*

QMAKE_STRIP = strip

QMAKE_STRIPFLAGS_LIB += --strip-unneeded

headers.path  = /usr/include/libmeegosyncml
headers.files = $$HEADERS
target.path   = /usr/lib
config.path   = /etc/sync
config.files  = config/meego-syncml-conf.xsd \
                config/meego-syncml-conf.xml
                
INSTALLS += target \
    	    headers \
    	    config

QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR  = $$target.path
QMAKE_PKGCONFIG_INCDIR  = $$headers.path
pkgconfig.files = $${TARGET}.pc

# Uncomment the following line to enable GCOV/LCOV report generation
#CONFIG += gcov

gcov {
    message(Test coverage report is ENABLED)

    QMAKE_CC = SBOX_USE_CCACHE=no $$QMAKE_CC
    QMAKE_CXX = SBOX_USE_CCACHE=no $$QMAKE_CXX

    QMAKE_CFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    LIBS += -lgcov
}

!gcov {
    message(Test coverage report is DISABLED)
}

QMAKE_CLEAN += *.gcno *.gcda *.gcov
