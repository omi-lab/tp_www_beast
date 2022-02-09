TARGET = tp_www_beast
TEMPLATE = lib

DEFINES += TP_WWW_BEAST_LIBRARY

#SOURCES += src/Globals.cpp
HEADERS += inc/tp_www_beast/Globals.h

SOURCES += src/Context.cpp
HEADERS += inc/tp_www_beast/Context.h

SOURCES += src/Server.cpp
HEADERS += inc/tp_www_beast/Server.h

SOURCES += src/ASIOCrossThreadCallbackFactory.cpp
HEADERS += inc/tp_www_beast/ASIOCrossThreadCallbackFactory.h
