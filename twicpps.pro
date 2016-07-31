TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

win32:INCLUDEPATH += C:\develop\openssl-1.0.1h-32\inc32
win32:LIBS += C:\develop\openssl-1.0.1h-32\lib\libeay32.lib
win32:LIBS += C:\develop\openssl-1.0.1h-32\lib\ssleay32.lib
unix:LIBS += -lssl -lcrypto

SOURCES += \
    src/hash.cpp \
    src/main.cpp \
    src/oauth.cpp \
    src/tweet.cpp \
    src/sha1.c \
    src/webclient.cpp \
    src/urlencode.cpp \
    src/charvec.cpp

HEADERS += \
    src/oauth.h \
    src/sha1.h \
    src/tweet.h \
    src/webclient.h \
    src/urlencode.h \
    src/charvec.h
