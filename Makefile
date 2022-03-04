PLXBASE = /usr/opt/PlxSdk/PlxApi/Library/
CC = g++
CFLAGS = -c -Wall
LDFLAGS = -ldl -lm
ROOT_FLAG = `root-config --cflags --glibs`

#==== old pixie library
#APIBASE = /usr/opt/Pixie16/lib/
#INCFLAGS = -I$(APIBASE)app/ -I$(APIBASE)sys/
#INCFLAGS2 = -I$(APIBASE)inc/
#LIBS = $(APIBASE)libPixie16App.a $(APIBASE)libPixie16Sys.a $(PLXBASE)PlxApi.a
#
#all: test
#
#test : test.o Pixie16Class.o
#	$(CC) test.o Pixie16Class.o $(LIBS) -o test
#	
#test.o : test.cpp
#	$(CC) $(CFLAGS) $(INCFLAGS) $(INCFLAGS2) test.cpp $(ROOT_FLAG)
#
#Pixie16Class.o : Pixie16Class.h Pixie16Class.cpp DataBlock.h
#	$(CC) $(CFLAGS) $(INCFLAGS) $(INCFLAGS2) Pixie16Class.cpp DataBlock.h $(ROOT_FLAG)
#

#==== new pixie library
INCFLAGS = -I/usr/opt/xia/PixieSDK/ -I/usr/opt/xia/PixieSDK/include/ -I/usr/opt/xia/PixieSDK/include/pixie16/ 
APIBASE = /usr/opt/xia/PixieSDK/lib/
LIBS = $(APIBASE)libPixie16Api.so $(APIBASE)libPixieSDK.a $(PLXBASE)PlxApi.a

all: test example

example : example.o Pixie16Class.o
	$(CC) $(INCFLAGS) example.o Pixie16Class.o  $(LIBS) -o example

example.o : example.cpp
	$(CC) $(CFLAGS) $(INCFLAGS)  example.cpp 

test : test.o Pixie16Class.o
	$(CC) $(INCFLAGS) test.o Pixie16Class.o  $(LIBS) -o test  $(ROOT_FLAG)
	
test.o : test.cpp
	$(CC) $(CFLAGS) $(INCFLAGS)  test.cpp $(ROOT_FLAG)

Pixie16Class.o : Pixie16Class.h Pixie16Class.cpp DataBlock.h
	$(CC) $(CFLAGS) $(INCFLAGS) Pixie16Class.cpp DataBlock.h $(ROOT_FLAG)

#need to export LD_LIBRARY_PATH
#the pixie.ini is not needed

clean:
	rm -f *.o test


