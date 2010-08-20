include(doc/doc.pro)

CONFIG += ordered \
	qt \
	debug \
	thread 

# Directories
SUBDIRS += src \
           tests_meego

QT += core

# Make sure gui is not enabled.
QT -= gui

QTDIR = /usr/lib/qt4

TEMPLATE = subdirs
