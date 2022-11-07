PLXBASE = /usr/opt/PlxSdk/PlxApi/Library/
CC = g++
CFLAGS = -c -Wall -lpthread
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


OBJS = startStopDialog.o pixieDAQ.o Pixie16Class.o settingsSummary.o scalarPanel.o moduleSetting.o channelSetting.o


all: testing/test testing/example  pixieDAQ

#--------------------------
pixieDAQ :  pixieDict.cxx $(OBJS)
	@echo "-------- making pixieDAQ "
	$(CC) $(PIXIE_LIB_PATH) $(OBJS) pixieDict.cxx $(LIBS) -o pixieDAQ $(ROOT_FLAG)

#--------------------------#need to export LD_LIBRARY_PATH
Pixie16Class.o : Pixie16Class.h Pixie16Class.cpp DataBlock.h
	@echo "-------- making Pixie16Class.o "
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) Pixie16Class.cpp DataBlock.h $(ROOT_FLAG)

pixieDict.cxx : pixieDAQ.h  pixieDAQLinkDef.h 
	@echo "--------- creating pcm and cxx "
	@rootcling -f pixieDict.cxx -c pixieDAQ.h -p $(PIXIE_LIB_PATH) pixieDAQLinkDef.h

pixieDAQ.o : pixieDict.cxx pixieDAQ.cpp pixieDAQ.h  global_macro.h
	@echo "--------- creating pixieDAQ.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) pixieDAQ.cpp Pixie16Class.cpp pixieDict.cxx $(ROOT_FLAG)

settingsSummary.o : settingsSummary.cpp settingsSummary.h global_macro.h
	@echo "--------- creating settingsSummary.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) settingsSummary.cpp $(ROOT_FLAG)

scalarPanel.o : scalarPanel.cpp scalarPanel.h global_macro.h
	@echo "--------- creating scalarPanel.o.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) scalarPanel.cpp $(ROOT_FLAG)

moduleSetting.o : moduleSetting.cpp moduleSetting.h global_macro.h
	@echo "--------- creating moduleSetting.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) moduleSetting.cpp $(ROOT_FLAG)

channelSetting.o : channelSetting.cpp channelSetting.h global_macro.h
	@echo "--------- creating channelSetting.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) channelSetting.cpp $(ROOT_FLAG)

startStopDialog.o : startStopDialog.cpp startStopDialog.h
	@echo "--------- creating startStopDialog.o"
	$(CC) $(CFLAGS) $(PIXIE_LIB_PATH) startStopDialog.cpp $(ROOT_FLAG)



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


