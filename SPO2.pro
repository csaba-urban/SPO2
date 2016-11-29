TEMPLATE = app
TARGET = SPO2

INCLUDEPATH += .

QT += sql widgets widgets

HEADERS = \
include\mainform.h \
include\serialport.h \
include\sqldatabase.h

SOURCES = \
source\main.cpp \
source\mainform.cpp \
source\serialport.cpp \
source\sqldatabase.cpp

FORMS = \
mainform.ui


QT+=widgets
