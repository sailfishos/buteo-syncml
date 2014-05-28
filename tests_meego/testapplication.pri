include(tests_common.pri)

pro_file_basename = $$basename(_PRO_FILE_)
pro_file_basename ~= s/\\.pro$//

TEMPLATE = app
TARGET = $${pro_file_basename}

HEADERS = $${pro_file_basename}.h
SOURCES = $${pro_file_basename}.cpp

LIBS += -L$${OUT_PWD}/$${tests_subdir_r} -lTestUtils

target.path = $${INSTALL_TESTDIR}/$${tests_subdir}
INSTALLS += target
