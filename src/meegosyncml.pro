include(syncelements/syncelements.pro)
include(server/server.pro)
include(client/client.pro)
include(transport/transport.pro)

TEMPLATE = lib

VER_MAJ = 0
VER_MIN = 4
VER_PAT = 6

CONFIG += dll

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
	DevInfHandler.cpp

HEADERS += SyncItem.h \
	StoragePlugin.h \
	ChangeLog.h \
	SuspendLog.h \
	SyncAgent.h \
	SyncItemKey.h \
	datatypes.h \
	internals.h \
	SyncAgentConfig.h \
	SyncMLMessageParser.h \
	AuthenticationPackage.h \
	LocalChangesPackage.h \
	LocalMappingsPackage.h \
	Package.h \
	DeviceInfo.h \
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
	StorageProvider.h \
	ServerAlertedNotification.h \
    SyncMLGlobals.h \
    RequestListener.h \
    DevInfHandler.h \
    SyncAgentConfigProperties.h
    
OTHER_FILES += config/meego-syncml-conf.xsd \
               config/meego-syncml-conf.xml \
               config/nokia-syncml-conf.xml

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

QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda \
    $(OBJECTS_DIR)/*.gcno \
    $(OBJECTS_DIR)/*.gcov \
    lib*.so*

QMAKE_STRIP = strip

QMAKE_STRIPFLAGS_LIB += --strip-unneeded

headers.path  = /usr/include/libbuteosyncml
headers.files = $$HEADERS
target.path   = /usr/lib
config.path   = /etc/sync
config.files  = config/meego-syncml-conf.xsd \
                config/nokia-syncml-conf.xml
                
INSTALLS += target \
    	    headers \
    	    config

# #####################################################################
# make coverage (debug)
# #####################################################################
coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
CONFIG(debug,debug|release){
    QMAKE_EXTRA_TARGETS += cov_cxxflags \
			cov_lflags

    cov_cxxflags.target = coverage
    cov_cxxflags.depends = CXXFLAGS \
         += \
        -fprofile-arcs \
        -ftest-coverage

    cov_lflags.target = coverage
    cov_lflags.depends = LFLAGS \
        += \
        -fprofile-arcs \
	-ftest-coverage

    coverage.commands = @echo \
        "Built with coverage support..."

    build_pass|!debug_and_release : coverage.depends = all

    QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda \
        $(OBJECTS_DIR)/*.gcno \
        $(OBJECTS_DIR)/*.gcov
}
