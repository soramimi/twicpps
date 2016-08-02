
TARGET=tweet

CFLAGS=-O2
CXXFLAGS=-O2 -std=c++11

all: $(TARGET)

OBJS = \
	src/base64.o \
	src/json.o \
	src/charvec.o \
	src/urlencode.o \
	src/oauth.o \
	src/sha1.o \
	src/webclient.o \
	src/tweet.o \
	src/main.o

$(TARGET): $(OBJS)
	g++ -o $(TARGET) $(OBJS) -lssl -lcrypto
	strip $(TARGET)

clean:
	-rm $(TARGET)
	-rm *.o
