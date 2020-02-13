isEmpty(TESTS_COMMON_PRI_INCLUDED) {
TESTS_COMMON_PRI_INCLUDED = 1

tests_subdir = $$relative_path($$dirname(_PRO_FILE_), $${PWD})
tests_subdir_r = $$relative_path($${PWD}, $$dirname(_PRO_FILE_))

QT += testlib core sql network
QT -= gui
CONFIG += link_prl link_pkgconfig

# DEFINES += QT_NO_DEBUG_OUTPUT

# use the buteosyncml library objects directly to get better debugging data
# So if remember to compile them when debugging
LIBS += -L$${OUT_PWD}/$${tests_subdir_r}/../src/

PKGCONFIG += buteosyncfw5 \
    libwbxml2
LIBS += -lbuteosyncml5

# This is needed to avoid adding the /usr/lib link directory before the
# newer version in buteosyncml
QMAKE_LIBDIR_QT = $${OUT_PWD}/$${tests_subdir_r}/../src $${QMAKE_LIBDIR_QT}

QMAKE_CXXFLAGS += -Wall -g

INCLUDEPATH = \
    $${PWD} \
    $${PWD}/../src/ \
    $${PWD}/../src/syncelements \
    $${PWD}/../src/transport \
    $${PWD}/../src/server \
    $${PWD}/../src/client \

# This way time to run qmake is reduced by ~35%
CONFIG -= depend_includepath
DEPENDPATH = \
    $${PWD}/../src/syncelements \
    $${PWD}/../src/transport \
    $${PWD}/../src/server \
    $${PWD}/../src/client \

INSTALL_TESTDIR = /opt/tests/buteo-syncml-qt5
INSTALL_TESTDATADIR = $${INSTALL_TESTDIR}/data

}
