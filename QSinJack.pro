#-------------------------------------------------
#
# Project created by QtCreator 2014-05-29T19:55:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QSinJack
TEMPLATE = app

LIBS = -ljack


SOURCES += main.cpp\
				MainWindow.cpp \
		SinusOnJack.cpp

HEADERS  += MainWindow.h \
		SinusOnJack.h

FORMS    += MainWindow.ui
