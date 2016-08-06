TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

win32:INCLUDEPATH += C:\develop\openssl-1.0.2h-32\inc32
win32:LIBS += C:\develop\openssl-1.0.2h-32\out32dll\libeay32.lib
win32:LIBS += C:\develop\openssl-1.0.2h-32\out32dll\ssleay32.lib
unix:LIBS += -lssl -lcrypto
macx:INCLUDEPATH += /usr/local/opt/openssl/include
macx:LIBS += -L /usr/local/opt/openssl/lib
macx:LIBS += -liconv

SOURCES += \
    src/main.cpp \
    src/oauth.cpp \
    src/tweet.cpp \
    src/sha1.c \
    src/webclient.cpp \
    src/urlencode.cpp \
    src/charvec.cpp \
    src/base64.cpp \
    src/json.cpp

HEADERS += \
    src/oauth.h \
    src/sha1.h \
    src/tweet.h \
    src/webclient.h \
    src/urlencode.h \
    src/charvec.h \
    src/base64.h \
    src/json.h
