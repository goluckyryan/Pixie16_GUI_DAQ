PLXBASE = /usr/opt/PlxSdk/PlxApi/Library/
CC = g++
CFLAGS = -c -Wall
LDFLAGS = -ldl -lm
ROOT_FLAG = `root-config --cflags --glibs`

#==== old pixie library
#APIBASE = /usr/opt/Pixie16/lib/
#PIXIE_LIB_PATH = -I$(APIBASE)app/ -I$(APIBASE)sys/
#PIXIE_LIB_PATH2 = -I$(APIBASE)inc/
#LIBS = $(APIBASE)libPixie16App.a $(APIBASE)libPixie16Sys.a $(PLXBASE)PlxApi.a
#
#all: test
#
#test : test.o Pixie16Class.o
#	$(CC) test.o Pixie16Class.o $(LIBS) -o test
#	
#test.o : test.cpp
#	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) $(PIXIE_LIB_PATH2) test.cpp $(ROOT_FLAG)
#
#Pixie16Class.o : Pixie16Class.h Pixie16Class.cpp DataBlock.h
#	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) $(PIXIE_LIB_PATH2) Pixie16Class.cpp DataBlock.h $(ROOT_FLAG)
#

#==== new pixie library
#the pixie.ini is not needed
PIXIE_LIB_PATH = -I/usr/opt/xia/PixieSDK/ -I/usr/opt/xia/PixieSDK/include/ -I/usr/opt/xia/PixieSDK/include/pixie16/ 
APIBASE = /usr/opt/xia/PixieSDK/lib/
LIBS = $(APIBASE)libPixie16Api.so $(APIBASE)libPixieSDK.a $(PLXBASE)PlxApi.a


all: testing/test testing/example  pixieDAQ

#--------------------------
pixieDAQ : pixieDAQ.o pixieDict.o pixieDict.cxx Pixie16Class.o mainSettings.o
	@echo "-------- making pixieDAQ "
	$(CC) $(PIXIE_LIB_PATH) pixieDAQ.o Pixie16Class.o mainSettings.o pixieDict.cxx $(LIBS) -o pixieDAQ $(ROOT_FLAG)

#--------------------------#need to export LD_LIBRARY_PATH
Pixie16Class.o : Pixie16Class.h Pixie16Class.cpp DataBlock.h
	@echo "-------- making Pixie16Class.o "
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) Pixie16Class.cpp DataBlock.h $(ROOT_FLAG)

pixieDict.cxx : pixieDAQ.h  pixieDAQLinkDef.h 
	@echo "--------- creating pcm and cxx "
	@rootcling -f pixieDict.cxx -c pixieDAQ.h -p $(PIXIE_LIB_PATH) pixieDAQLinkDef.h

pixieDAQ.o : pixieDict.cxx pixieDAQ.cpp pixieDAQ.h  
	@echo "--------- creating pixieDAQ.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) pixieDAQ.cpp Pixie16Class.cpp pixieDict.cxx $(ROOT_FLAG)

mainSettings.o : mainSettings.cpp mainSettings.h
	@echo "--------- creating mainSettings.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) Pixie16Class.o mainSettings.cpp $(ROOT_FLAG)




#--------------------------
testing/example : testing/example.o 
	@echo "-------- making example"
	$(CC) $(PIXIE_LIB_PATH) testing/example.o  $(LIBS) -o testing/example

testing/example.o : testing/example.cpp
	@echo "-------- making example.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH)  testing/example.cpp -o testing/example.o

#--------------------------
testing/test : testing/test.o Pixie16Class.o
	@echo "-------- making test"
	$(CC) $(PIXIE_LIB_PATH) testing/test.o Pixie16Class.o  $(LIBS) -o testing/test  $(ROOT_FLAG)

testing/test.o : testing/test.cpp
	@echo "-------- making test.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) testing/test.cpp $(ROOT_FLAG) -o testing/test.o


#origin root example
#pixieDAQ: 
#	@rootcling -f pixieDict.cxx -c pixieDAQ.h pixieDAQLinkDef.h
#	$(CC) $(CFLAGS) pixieDAQ.cpp  pixieDict.cxx $(ROOT_FLAG)


clean:
	rm -f *.o testing/test testing/example testing/*.o *.pcm testing/example pixieDAQ *.gch *.cxx


