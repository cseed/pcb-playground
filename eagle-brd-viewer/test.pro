TEMPLATE = app
TARGET = test

CONFIG += qt c++11 debug
QT += widgets

# DEFINES += "_LIBCPP_DEBUG"

HEADERS += netlist.hpp main.hpp 
SOURCES += main.cpp netlist.cpp
LIBS += -lpugixml
