#ifndef PIXIE16_CPP
#define PIXIE16_CPP

///legacy pixie libaray
///#include "pixie16app_export.h"
///#include "pixie16sys_export.h"
///#include "def21160.h"

#include <bitset>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <array>

#include "Pixie16Class.h"

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

Pixie16::Pixie16(){

  isRunning = false;
  retval = 0;
  
  CheckDriver();
  if( retval > 0 ) CheckHardware();
    
  if( retval > 0 ){
    
    LoadConfigFile();
    
    if( retval > 0 ) {
      BootDigitizers();

      nFIFOWords = 0;
      totNumFIFOWords = 0;
      ExtFIFO_Data = NULL;
      Statistics = NULL;
      
      FIFOEnergies   = new unsigned short[MAXFIFODATABLOCK];
      FIFOChannels   = new unsigned short[MAXFIFODATABLOCK];
      FIFOMods       = new unsigned short[MAXFIFODATABLOCK];
      FIFOTimestamps = new unsigned long long[MAXFIFODATABLOCK];
      FIFONumDataBlock = 0;
      AccumulatedFIFONumDataBlock = 0;
      FIFOisUsed = false;

      data = new DataBlock();
      nextWord = 0;
    
    }
  }
}

Pixie16::~Pixie16(){
  
  CloseDigitizers();
  
  delete FIFOEnergies;
  delete FIFOChannels;
  delete FIFOMods;
  delete FIFOTimestamps;
  
  delete PXISlotMap;
  delete ch2ns;
  delete ComFPGAConfigFile;
  delete SPFPGAConfigFile;
  delete TrigFPGAConfigFile;
  delete DSPCodeFile;
  delete DSPParFile;
  delete DSPVarFile;
  
  delete ModRev;
  delete ModSerNum;
  delete ModADCBits;
  delete ModADCMSPS;
  delete numChannels;
  delete ch2ns;

  delete ExtFIFO_Data;
  delete Statistics;
  delete data;
  
}


void Pixie16::LoadConfigFile(bool verbose, std::string fileName){
  
  /************************************************************
   * this method is copy and modified from J.M. Allmond (ORNL)
   * *********************************************************/

  printf("\033[32m======= Loading Configuration file : %s\033[m\n",fileName.c_str());
  
  NumCrates = 1;
  
  short LINE_LENGTH = 300;
  char line[LINE_LENGTH];
  FILE *fprconfig;
  char configType=0;
  
  char tempcharbuf[LINE_LENGTH];
  int tempint1=0, tempint2=0, tempint3=0;
  
  char tempComFPGAConfigFile[100][LINE_LENGTH];
  char tempSPFPGAConfigFile[100][LINE_LENGTH];
  char tempDSPCodeFile[100][LINE_LENGTH];
  char tempDSPParFile[100][LINE_LENGTH];
  char tempDSPVarFile[100][LINE_LENGTH];
  unsigned short tempPXISlotMap[100];
  unsigned short tempPXIFPGAMap[100];

  if ((fprconfig = fopen(fileName.c_str(), "r")) == NULL) {
    fprintf(stderr, "Error, cannot open input file %s\n", fileName.c_str());
    retval = -1;
    return ;
  } 
  
  NumModules = 0;
  
  ///========================= Get number of modules and FPGA firmware
  while(fgets(line, LINE_LENGTH, fprconfig) != NULL){
    for(int i = 0; i < LINE_LENGTH; i++){
      if(line[i] == '#'){
        if(verbose)printf("%s", line);
        break;
      }else if(line[i] >= 0 && line[i] != ' ' && line[i] != '\n'){   

        //Slot and mode and fpga
        if (line[0] == 'S') {
          sscanf(line,"%c\t%d\t%d\t%d\n", &configType, &tempint1, &tempint2, &tempint3);
          if(verbose) printf("%c\t%d\t%d\t%d\n", configType, tempint1, tempint2, tempint3);
          if (tempint1 >= 2 && tempint2 >= 0 && tempint3 >= 0) {
            tempPXISlotMap[tempint2] = tempint1;
            tempPXIFPGAMap[tempint2] = tempint3;
            NumModules ++;
          }
          break;
        //FPGA file
        }else if (line[0] == 'F') {
          sscanf(line,"%c\t%d\t%d\t%s\n", &configType, &tempint1, &tempint2, tempcharbuf);
          if(verbose) printf("%c\t%d\t%d\t%s\n", configType, tempint1, tempint2, tempcharbuf);
          if (tempint1 >= 0 && tempint1 < 100 && tempint2 >= 0 && tempint2 < 6) {
            
            if (tempint2 == 0) strncpy(tempComFPGAConfigFile[tempint1], tempcharbuf, LINE_LENGTH);
            if (tempint2 == 1) strncpy(tempSPFPGAConfigFile[tempint1], tempcharbuf, LINE_LENGTH); 
            if (tempint2 == 2) strncpy(tempDSPCodeFile[tempint1], tempcharbuf, LINE_LENGTH);
            if (tempint2 == 3) strncpy(tempDSPVarFile[tempint1] , tempcharbuf, LINE_LENGTH);
            if (tempint2 == 4) strncpy(tempDSPParFile[tempint1], tempcharbuf, LINE_LENGTH);
             
          }
          break;
        }else {
          printf("Error in reading %s : bad id or format\n", fileName.c_str());
          retval = -2;
          return;
        }
      }else if(line[i]=='\n'){
        if(verbose) printf("\n");
        break;
      }else {
        continue;
      }
    }
    memset(line, 0, LINE_LENGTH);
  }
  fclose(fprconfig);
  
  PXISlotMap = new unsigned short [NumModules];
  
  ComFPGAConfigFile  = new char* [NumModules];
  SPFPGAConfigFile   = new char* [NumModules];
  TrigFPGAConfigFile = new char* [NumModules];
  DSPCodeFile        = new char* [NumModules];
  DSPParFile         = new char* [NumModules];
  DSPVarFile         = new char* [NumModules];
  
  ModRev = new unsigned short [NumModules];
  ModSerNum = new unsigned int [NumModules];
  ModADCBits = new unsigned short [NumModules];
  ModADCMSPS = new unsigned short [NumModules];
  numChannels = new unsigned short [NumModules];

  OfflineMode = 0;
  BootPattern = 0x7F;
  
  ch2ns = new unsigned short [NumModules];
  
  for ( int i = 0 ; i < NumModules ; i++){
    
    PXISlotMap[i] = tempPXISlotMap[i];
    
    ComFPGAConfigFile  [i] = tempComFPGAConfigFile[tempPXIFPGAMap[i]];
    SPFPGAConfigFile   [i] = tempSPFPGAConfigFile[tempPXIFPGAMap[i]];
    TrigFPGAConfigFile [i] = (char *)"FPGATrig"; ///only Revision A
    DSPCodeFile        [i] = tempDSPCodeFile[tempPXIFPGAMap[i]];
    DSPVarFile         [i] = tempDSPVarFile[tempPXIFPGAMap[i]];
    DSPParFile         [i] = tempDSPParFile[tempPXIFPGAMap[i]];
  }
  
  printf("########################## \n");
  printf("Number of Module : %d \n", NumModules);
  printf(" Slot Map : "); for( int i = 0; i < NumModules ; i++) printf("%2d\t", PXISlotMap[i]);
  printf("\n");
  printf("Module ID : "); for( int i = 0; i < NumModules ; i++) printf("%2d\t", i);
  printf("\n");
  for( int i = 0; i < NumModules; i++){
    printf("\033[0;32m--- configuration files for module-%02d (slot-%02d)\033[m\n", i, PXISlotMap[i] );
    printf("  ComFPGA : %s \n", ComFPGAConfigFile[i]);
    printf("   SPFPGA : %s \n", SPFPGAConfigFile[i]);
    printf(" DSP Code : %s \n", DSPCodeFile[i]);
    printf(" DSP Var  : %s \n", DSPVarFile[i]);
    printf(" DSP Par  : %s \n", DSPParFile[i]);
  }
  
  retval = 1;  
}

int Pixie16::CheckError(std::string operation){
  
  if( retval < 0 ){
    printf("\033[1;31m*ERROR* \033[1;33m%s\033[m failed, retval = %d\n", operation.c_str(), retval);
    char  msg[300]; 
    PixieGetReturnCodeText(retval, msg, 300);
    printf("     error massge : %s \n", msg );
    return -1;
  }
  
  return 1;
}


void Pixie16::CheckDriver(){
  printf("\033[32m======= check PLX PCI 9054 ...\033[m\n");
  //check driver is loaded
  std::string ans = exec("lsmod | grep 9054 | wc -l");
  if( ans == "0\n" ){
   printf("cannot find PLX PCI 9054 driver.\n");
   retval = -1;
   return; 
  }
  printf("Found PLX PCI 9054 driver.\n");
  retval = 1;
}  


void Pixie16::CheckHardware(){
  //check pci is there
  std::string ans = exec("lspci | grep 9054 | wc -l");
  if( ans == "0\n" ){
   printf("cannot find PLX PCI 9054.\n");
   retval = -2;
   return; 
  }
  printf("Found PLX PCI 9054 card.\n");

  //check pci is OK
  ans = exec("lspci -vv| grep \"Unknown header type\" | wc -l");
  if( atoi(ans.c_str()) > 0 ){
   printf("Found PLX PCI 9054, but not working. Unknown header Type.\n");
   retval = -3;
   return; 
  }
  printf("PLX PCI 9054 card does not detected problem.\n");
  retval = 1;
}


void Pixie16::GetDigitizerInfo(unsigned short modID){


  //retval = Pixie16ReadModuleInfo(modID, &ModRev, &ModSerNum, &ModADCBits, &ModADCMSPS, &numChannels);
  retval = Pixie16ReadModuleInfo(modID, &ModRev[modID], &ModSerNum[modID], &ModADCBits[modID], &ModADCMSPS[modID], &numChannels[modID]);
  
  if( CheckError("Pixie16ReadModuleInfo") < 0 ) return ;
  
  printf("------------ Module-%d \n", modID);
  printf("         Revision : %d \n", ModRev[modID]);
  printf("       Serial Num : %d \n", ModSerNum[modID]);
  printf("         ADC Bits : %d \n", ModADCBits[modID]);
  printf("ADC sampling rate : %d \n", ModADCMSPS[modID]);
  printf("       # channels : %d \n", numChannels[modID]);
  
  ch2ns[modID] = 1000/ModADCMSPS[modID];
  
}


void Pixie16::BootDigitizers(){
  
  // Boot Modules
  printf("\033[32m======= Booting Pixie16 System ...\033[0m\n");

  retval = Pixie16InitSystem (NumModules, PXISlotMap, OfflineMode);  
 
  printf("\033[32mInit Ok\033[0m\n");
  

  // Boot Module
  printf("\033[32mBooting module ...\033[0m\n");
  for( int i = 0 ; i < NumModules; i++){
    
    GetDigitizerInfo(i);
    
    retval = Pixie16BootModule (
                                ComFPGAConfigFile[i], 
                                SPFPGAConfigFile[i], 
                                TrigFPGAConfigFile[i], 
                                DSPCodeFile[i], 
                                DSPParFile[i], 
                                DSPVarFile[i], 
                                i, 
                                BootPattern
                              );
    
    if( CheckError("Pixie16BootModule") < 0 ) return ;
    printf("\033[32mBoot Ok\033[0m\n");
    
  }
  
}

void Pixie16::CloseDigitizers(){
  retval = Pixie16ExitSystem(NumModules);
  CheckError("Pixie16ExitSystem");
}

void Pixie16::AdjustOffset(){
  
  retval = Pixie16AdjustOffsets(NumModules);
  if( CheckError("Pixie16AdjustOffsets") < 0 ) return;
  printf(" Adjust Offset for All modules.\n");
  
}

void Pixie16::CaptureBaseLine(unsigned short modID, unsigned short ch){  
  
  retval = Pixie16AcquireBaselines(modID);
  if( CheckError("Pixie16AcquireBaselines::MOD::"+std::to_string(modID)) < 0 ) return;
  
  
  ///somehow, this change the DSPParFile
  std::string temp = DSPParFile[modID];
  
  retval = Pixie16ReadSglChanBaselines(Baselines, TimeStamps, 3640, modID, ch);
  if( CheckError("Pixie16ReadSglChanBaselines::MOD::"+std::to_string(modID) + "CH::"+std::to_string(ch)) < 0 ) return;
  
  strcpy(DSPParFile[modID], temp.c_str());

  
}

void Pixie16::CaptureADCTrace(unsigned short modID, unsigned short ch){
  
  retval = Pixie16AcquireADCTrace (modID);
  if( CheckError("Pixie16AcquireADCTrace") < 0 ) return;
  
  retval = Pixie16ReadSglChanADCTrace (ADCTrace, 8192, modID, ch);
  if( CheckError("Pixie16ReadSglChanADCTrace") < 0 ) return;
  
}

void Pixie16::StartRun(bool listMode){
  
  unsigned short mode = NEW_RUN; //RESUME_RUN
  
  if( listMode ){
    ///listmode
    totNumFIFOWords = 0;
    AccumulatedFIFONumDataBlock = 0;
    nextWord = 0;
    retval = Pixie16StartListModeRun(NumModules, LIST_MODE_RUN, mode);
    if( CheckError("Pixie16StartListModeRun") < 0 ) return;
    printf("\033[32m LIST_MODE run\033[0m\n");
    
  }else{
    ///MCA mode
    retval = Pixie16StartHistogramRun(NumModules, mode);
    if( CheckError("Pixie16StartHistogramRun") < 0 ) return;
    printf("\033[32m MCA MODE run\033[0m\n");
  }
  
  isRunning = true;
}


void Pixie16::StopRun(){
  
  retval =  Pixie16EndRun( NumModules );
  if( CheckError("Pixie16EndRun") < 0 ) return;
  printf("\033[32mRun Stopped\033[0m\n");
  
  isRunning = false;
  
}

void Pixie16::CheckExternalFIFOWords(unsigned short modID){
  if( Pixie16CheckRunStatus(modID) == 1){
    retval = Pixie16CheckExternalFIFOStatus (&nFIFOWords, modID);
    if( CheckError("Pixie16CheckExternalFIFOStatus") < 0 ) return;
  }
}

void Pixie16::ReadData(unsigned short modID){

  if( Pixie16CheckRunStatus(modID) == 1){
    retval = Pixie16CheckExternalFIFOStatus (&nFIFOWords, modID);
    if( CheckError("Pixie16CheckExternalFIFOStatus") < 0 ) return;
    ///if(nFIFOWords *1.0 / EXTERNAL_FIFO_LENGTH > 0.2) {
    if(nFIFOWords  > 0) {
      ///printf("\033[1;31m####### READ DATA \033[m: number of word in module-%d FIFO : %d (%.2f kB)\n", modID, nFIFOWords, nFIFOWords*4./1024.);
      if( ExtFIFO_Data != NULL ) delete ExtFIFO_Data;
      ExtFIFO_Data = new unsigned int [nFIFOWords];
      retval = Pixie16ReadDataFromExternalFIFO(ExtFIFO_Data, nFIFOWords, modID);
      CheckError("Pixie16ReadDataFromExternalFIFO");
      totNumFIFOWords += nFIFOWords;
      FIFONumDataBlock = 0;
    }
  }else{
    ///printf("Pixie16 is not running.\n");
  }
}

unsigned int Pixie16::ScanNumDataBlockInExtFIFO(){
  
  unsigned int nextWordtemp = nextWord;
  
  ///if( nextWordtemp < nFIFOWords )  printf("============= FIFOWord : %u \n", nFIFOWords);
  while( nextWordtemp < nFIFOWords ){
    
    unsigned short eventLen = (ExtFIFO_Data[nextWordtemp] >> 17) & 0x3FFF;
    
    FIFOEnergies[FIFONumDataBlock]   = (ExtFIFO_Data[nextWordtemp + 3] & 0xFFFF ); 
    FIFOChannels[FIFONumDataBlock]   = (ExtFIFO_Data[nextWordtemp] & 0xF ); 
    FIFOMods[FIFONumDataBlock]       = ((ExtFIFO_Data[nextWordtemp] >> 4) & 0xF) - 2;
    FIFOTimestamps[FIFONumDataBlock] = ((unsigned long long)(ExtFIFO_Data[nextWordtemp+2] & 0xFFFF) << 32) + ExtFIFO_Data[nextWordtemp+1];

    nextWordtemp  += eventLen;
    ///printf("%u | nextWordtemp %u, nextWord %u, ch %u, energy %u \n",  FIFONumDataBlock, nextWordtemp, nextWord, FIFOChannels[FIFONumDataBlock], FIFOEnergies[FIFONumDataBlock]);
    FIFONumDataBlock ++;
  }

  nextWord = nextWordtemp - nFIFOWords ;
  AccumulatedFIFONumDataBlock += FIFONumDataBlock;
  
  return FIFONumDataBlock;
}

int Pixie16::ProcessSingleData(short ch){
  
  int breakProcessLoopFlag = 0;
  
  if( nextWord < nFIFOWords ){
    data->ch           =  ExtFIFO_Data[nextWord] & 0xF ;
    data->slot         = (ExtFIFO_Data[nextWord] >> 4) & 0xF;
    data->crate        = (ExtFIFO_Data[nextWord] >> 8) & 0xF;
    data->headerLength = (ExtFIFO_Data[nextWord] >> 12) & 0x1F;
    data->eventLength  = (ExtFIFO_Data[nextWord] >> 17) & 0x3FFF;
    data->pileup       =  ExtFIFO_Data[nextWord] >> 31 ;
    data->eventID ++;
    
    if( nextWord + data->eventLength <= nFIFOWords ){
    
      data->time         = ((unsigned long long)(ExtFIFO_Data[nextWord+2] & 0xFFFF) << 32) + ExtFIFO_Data[nextWord+1];
      data->cfd          =  ExtFIFO_Data[nextWord + 2] >> 16 ; 
      data->energy       = (ExtFIFO_Data[nextWord + 3] & 0xFFFF ); 
      data->trace_length = (ExtFIFO_Data[nextWord + 3] >> 16) & 0x7FFF;
      data->trace_out_of_range =  ExtFIFO_Data[nextWord + 3] >> 31;
      
      if( data->eventLength > data->headerLength ){      
        for( int i = 0; i < data->trace_length/2 ; i++){
          data->trace[2*i+0] =  ExtFIFO_Data[nextWord + data->headerLength + i] & 0xFFFF ;
          data->trace[2*i+1] = (ExtFIFO_Data[nextWord + data->headerLength + i] >> 16 ) & 0xFFFF ;
        }
      }
    }else{
      data->time = 0;
      data->cfd = 0;
      data->energy = 0;
      data->trace_length = 0;
      data->trace_out_of_range = 0;
    }
    
    nextWord += data->eventLength ;
    
    if( nextWord == nFIFOWords ) {nextWord = 0; breakProcessLoopFlag = 1;}
    if( nextWord > nFIFOWords ) {nextWord = nextWord - nFIFOWords; breakProcessLoopFlag = 2;}
    if( data->ch == ch ) breakProcessLoopFlag = 1;
  }
  
  return breakProcessLoopFlag ; 

}


unsigned int Pixie16::GetDigitizerSetting(std::string parName, unsigned short modID, bool verbose){
  unsigned int ParData;
  retval = Pixie16ReadSglModPar (const_cast<char*> (parName.c_str()), &ParData, modID); 
  if( CheckError("Pixie16ReadSglModPar::"+parName) < 0 ) return -404;
  
  if( verbose ){
    if( parName == "MODULE_CSRA"  || parName == "FastTrigBackplaneEna" || parName == "TrigConfig0" || parName == "TrigConfig1" || parName == "TrigConfig2" || parName == "TrigConfig3" ){
      printf("READ | Mod : %2d, %20s = %X\n", modID, parName.c_str(), ParData);
    }else if(parName == "MODULE_CSRB"){
      printf("READ | Mod : %2d, %20s = %X\n", modID, parName.c_str(), ParData);
      printf("---------------------------------------------------------------------------\n");
      printf(     "                                 32  28  24  20  16  12   8   4   0\n");
      printf(     "                                  |   |   |   |   |   |   |   |   |\n");
      std::cout <<"         Module Configuration : 0xb" << std::bitset<32>(ParData) << std::endl;    
      printf(" wired-OR trigger lines to backplane to pullup resistor (bit:  0) : %s \n", ParData & MOD_CSRB_BIT::WIRED_OR_TRIGGER_TO_PULLUP_RESIST ? "Yes" : "No");
      printf("                                          Direct module (bit:  4) : %s \n", ParData & MOD_CSRB_BIT::DIRECT_MODULUE ? "Yes" : "No");
      printf("                                  Chassis Master module (bit:  6) : %s \n", ParData & MOD_CSRB_BIT::CHASSIS_MASTER ? "\033[1;33mYes\033[m" : "\033[1;31mNo\033[m");
      printf("                         Use Global Fast Trigger Source (bit:  7) : %s \n", ParData & MOD_CSRB_BIT::GLOBAL_FAST_TRIGGER ? "Yes" : "No");
      printf("                            Use external trigger source (bit:  8) : %s \n", ParData & MOD_CSRB_BIT::EXTERNAL_TRIGGER ? "Yes" : "No");
      printf("                        Control external INHIBIT signal (bit: 10) : %s \n", ParData & MOD_CSRB_BIT::USE_INHIBIT ? "INHIBIT" : "not INHIBIT");
      printf("       distribute clock and triggers in multiple crates (bit: 11) : %s \n", ParData & MOD_CSRB_BIT::DISTRIBUTE_CLOCK ? "multiple crates" : "single crate");
      printf("                                         Sort Timestamp (bit: 12) : %s \n", ParData & MOD_CSRB_BIT::SORT_TIMESTAMP ? "\033[1;33mYes\033[m" : "\033[1;31mNo\033[m");
      printf("        Enable connection of fast triggers to backplane (bit: 13) : %s \n", ParData & MOD_CSRB_BIT::FAST_TRIGGER_TO_BACKPLANE ? "enable" : "disable");
      printf("---------------------------------------------------------------------------\n");
    }else if(parName == "SYNCH_WAIT"){
      printf("READ | Mod : %2d, %20s = %d (%s)\n", modID, parName.c_str(), ParData, ParData ? "simultaneously" : "not simultaneously");      
    }else if(parName == "IN_SYNCH"){
      printf("READ | Mod : %2d, %20s = %d (%s)\n", modID, parName.c_str(), ParData, ParData ? "not simultaneously" : "simultaneously");      
    }else if(parName == "HOST_RT_PRESET"){
      printf("READ | Mod : %2d, %20s = %f sec\n", modID, parName.c_str(), IEEEFloating2Decimal(ParData));      
    }else{
      printf("READ | Mod : %2d, %20s = %d\n", modID, parName.c_str(), ParData);
    }
  }
  
  return ParData;
}


void Pixie16::PrintDigitizerSettings(unsigned short modID){
  
  printf("############################################### Digitizer setting for module-%d\n", modID);

  ///GetDigitizerSetting("MODULE_NUMBER",        modID, true);
  ///GetDigitizerSetting("ModID",                modID, true);
  ///GetDigitizerSetting("MODULE_FORMAT",        modID, true);
  

  ///GetDigitizerSetting("MODULE_CSRA",          modID, true);
  ///GetDigitizerSetting("MAX_EVENTS",           modID, true);
  GetDigitizerSetting("CrateID",              modID, true);
  GetDigitizerSetting("SlotID",               modID, true);
  GetDigitizerSetting("SYNCH_WAIT",           modID, true);
  GetDigitizerSetting("IN_SYNCH",             modID, true);
  GetDigitizerSetting("SLOW_FILTER_RANGE",    modID, true);
  GetDigitizerSetting("FAST_FILTER_RANGE",    modID, true);
  GetDigitizerSetting("FastTrigBackplaneEna", modID, true);
  GetDigitizerSetting("TrigConfig0",          modID, true);
  GetDigitizerSetting("TrigConfig1",          modID, true);
  GetDigitizerSetting("TrigConfig2",          modID, true);
  GetDigitizerSetting("TrigConfig3",          modID, true);
  GetDigitizerSetting("HOST_RT_PRESET",       modID, true);
  GetDigitizerSetting("MODULE_CSRB",          modID, true);
  
}

void Pixie16::SetDigitizerSetting(std::string parName, unsigned int val, unsigned short modID, bool verbose){
  retval = Pixie16WriteSglModPar( const_cast<char*> (parName.c_str()), val, modID);
  if( CheckError("Pixie16WriteSglModPar::"+parName) < 0 ) return;
  if( verbose ) GetDigitizerSetting(parName, modID, verbose);
}

double Pixie16::GetChannelSetting(std::string parName, unsigned short modID, unsigned short ch, bool verbose){
  
  double ParData;
  retval = Pixie16ReadSglChanPar( const_cast<char*> (parName.c_str()), &ParData, modID, ch);
  if( CheckError("Pixie16ReadSglChanPar::"+parName) < 0 ) return -404;
  
  if( verbose ) {
    if( parName == "TRIGGER_THRESHOLD" ) {
      printf("READ | Mod : %2d, CH: %2d, %18s = %5d ADC\n", modID, ch, parName.c_str(), (int) ParData);
    }else if( parName == "BASELINE_PERCENT" ) {
      printf("READ | Mod : %2d, CH: %2d, %18s = %9.3f %%\n", modID, ch, parName.c_str(), ParData);
    }else if( parName == "BLCUT" ) {
      printf("READ | Mod : %2d, CH: %2d, %18s = %5d \n", modID, ch, parName.c_str(), (int) ParData);
    }else if ( parName == "CHANNEL_CSRA" || parName == "CHANNEL_CSRB" || parName == "MultiplicityMaskL" || parName == "MultiplicityMaskH"){
      if( parName == "CHANNEL_CSRA" ) printf("---------------------------------------------------------------------------\n");
      printf("READ | Mod : %2d, CH: %2d, %18s = %X\n", modID, ch, parName.c_str(), (int) ParData);
      printf(     "                                 32  28  24  20  16  12   8   4   0\n");
      printf(     "                                  |   |   |   |   |   |   |   |   |\n");
      std::cout <<"        Channel Configuration : 0xb" << std::bitset<32>(ParData) << std::endl;     
      
      if( parName == "CHANNEL_CSRA" ){
        int CSRA = (int) ParData;
        printf("                                 fast trigger selection (bit:  0) : %s \n", CSRA & CSRA_BIT::FAST_TRIGGER ? "external" : "internal");
        printf("                     module validation signal selection (bit:  1) : %s \n", CSRA & CSRA_BIT::M_VALIDATION ? "module gate" : "global gate");
        printf("                                         channel enable (bit:  2) : %s \n", CSRA & CSRA_BIT::ENABLE_CHANNEL ? "\033[1;33mYes\033[m" : "\033[1;31mNo\033[m");
        printf("                    channal validation signal selection (bit:  3) : %s \n", CSRA & CSRA_BIT::C_VALIFATION ? "module gate" : "global gate");
        printf("Block data acquisition if trace or header DPMs are full (bit:  4) : %s \n", CSRA & CSRA_BIT::BLOCK_DAQ_DPM_FULL ? "Yes" : "No");
        printf("                                        signal polarity (bit:  5) : %s \n", CSRA & CSRA_BIT::POLARITY ? "\033[1;33mPositive\033[m" : "\033[1;31mNegative\033[m");
        printf("                                   veto channel trigger (bit:  6) : %s \n", CSRA & CSRA_BIT::VETO_TRIGGER ? "enable" : "disable");
        printf("                                   Enable trace capture (bit:  8) : %s \n", CSRA & CSRA_BIT::ENABLE_TRACE ? "enable" : "disable");
        printf("                                 Enable QDC sum capture (bit:  9) : %s \n", CSRA & CSRA_BIT::ENABLE_QDC ? "enable" : "disable");
        printf("                                Enable CFD trigger mode (bit: 10) : %s \n", CSRA & CSRA_BIT::ENABLE_CFD ? "enable" : "disable");
        printf("                     required module validation trigger (bit: 11) : %s \n", CSRA & CSRA_BIT::REQ_M_VALIDATION ? "required" : "not required");
        printf("           Enable capture raw energy sums and baselines (bit: 12) : %s \n", CSRA & CSRA_BIT::CAPTURE_ESUMS_BASELINE ? "enable" : "disable");
        printf("                    required cahnnel validation trigger (bit: 13) : %s \n", CSRA & CSRA_BIT::REQ_C_VALIDATION ? "required" : "not required");
        printf("                              Enable input relay (Gain) (bit: 14) : %s \n", CSRA & CSRA_BIT::INPUT_RELAY ? "\033[1;33mclose (no att.)\033[m" : "\033[1;31mopen (1/4 att.)\033[m");
        printf("                                        Pile-up control (bit: 15-16) : ");
                                  int pileUpVaule = (CSRA & CSRA_BIT::PILEUP);
                                   if( pileUpVaule == 0 ) printf("no energy for pile-up\n");
                                   if( pileUpVaule == 1 ) printf("reject pile-up\n");
                                   if( pileUpVaule == 2 ) printf("no trace for non-pile-up\n");
                                   if( pileUpVaule == 3 ) printf("only pile-up\n");
        printf("                        Enable no-trace-for-large-pulse (bit: 17) : %s \n", CSRA & CSRA_BIT::NO_TRACE_LARGE_PULSE ? "enable" : "disable");
        printf("                                Group trigger selection (bit: 18) : %s \n", CSRA & CSRA_BIT::GROUP_TRIGGER  ? "group trigger" : "local fast trigger");
        printf("                                 Channel veto selection (bit: 19) : %s \n", CSRA & CSRA_BIT::CH_VETO  ? "channel validation trigger" : "front pannel channel veto");
        printf("                                  Module veto selection (bit: 20) : %s \n", CSRA & CSRA_BIT::MO_VETO ? "module validation trigger" : "front pannel module veto");
        printf("                     external timestamp in event header (bit: 21) : %s \n", CSRA & CSRA_BIT::EXT_TIMESTAMP ? "enable" : "disable");
        printf("---------------------------------------------------------------------------\n");        
      }
       
    }else{
      printf("READ | Mod : %2d, CH: %2d, %18s = %9.3f us\n", modID, ch, parName.c_str(), ParData);
    }
  }
  
  return ParData;
}

unsigned short Pixie16::GetCSRA(int bitwise,  unsigned short modID, unsigned short ch, bool verbose){
  double ParData;
  retval = Pixie16ReadSglChanPar( (char *)"CHANNEL_CSRA", &ParData, modID, ch);
  if( CheckError("Pixie16ReadSglChanPar::CHANNEL_CSRA") < 0 ) return false;
  if( verbose ) printf("Mod-%d CH-%02d %X: %s\n", modID, ch, bitwise, (((int)ParData) & bitwise ) ? "Positive" : "Negative" );
  return (((int)ParData) & bitwise );
}

void Pixie16::PrintChannelAllSettings(unsigned short modID, unsigned short ch){
  
  printf("######################################################## Channel setting. Mod-%d CH-%02d\n", modID, ch);
  GetChannelSetting("TRIGGER_RISETIME",   modID, ch, true); 
  GetChannelSetting("TRIGGER_FLATTOP",    modID, ch, true); 
  GetChannelSetting("TRIGGER_THRESHOLD",  modID, ch, true); 
  GetChannelSetting("ENERGY_RISETIME",    modID, ch, true); 
  GetChannelSetting("ENERGY_FLATTOP",     modID, ch, true); 
  GetChannelSetting("TAU",                modID, ch, true); 
  GetChannelSetting("TRACE_LENGTH",       modID, ch, true); 
  GetChannelSetting("TRACE_DELAY",        modID, ch, true); 
  GetChannelSetting("VOFFSET",            modID, ch, true); 
  GetChannelSetting("XDT",                modID, ch, true); 
  GetChannelSetting("BASELINE_PERCENT",   modID, ch, true); 
  GetChannelSetting("BASELINE_AVERAGE",   modID, ch, true); 
  GetChannelSetting("BLCUT",              modID, ch, true); 
  GetChannelSetting("EMIN",               modID, ch, true); 
  ///GetChannelSetting("CHANNEL_CSRB",       modID, ch, true); //CSRB is reserved to be zero
  GetChannelSetting("QDCLen0",            modID, ch, true); 
  GetChannelSetting("QDCLen1",            modID, ch, true); 
  GetChannelSetting("QDCLen2",            modID, ch, true); 
  GetChannelSetting("QDCLen3",            modID, ch, true); 
  GetChannelSetting("QDCLen4",            modID, ch, true); 
  GetChannelSetting("QDCLen5",            modID, ch, true); 
  GetChannelSetting("QDCLen6",            modID, ch, true); 
  GetChannelSetting("QDCLen7",            modID, ch, true); 
  GetChannelSetting("MultiplicityMaskL",  modID, ch, true); 
  GetChannelSetting("MultiplicityMaskH",  modID, ch, true); 
  GetChannelSetting("CHANNEL_CSRA",       modID, ch, true); 
  printf("=====================================\n");
 }

void Pixie16::PrintChannelSettingsSummary(unsigned short modID){

  printf("====+=====+======+========+========+===========+==========+==========+==========+=======+=========+=========+=======+====== \n");  
  printf(" ch | En  | Gain | Trig_L | Trig_G | Threshold | Polarity | Energy_L | Energy_G | Tau   | Trace   | Trace_d |  Voff | BL \n");
  printf("----+-----+------+--------+--------+-----------+----------+----------+----------+-------+---------+---------+-------+------ \n");
  for( int ch = 0; ch < 16; ch ++){
    printf(" %2d |", ch);
    printf(" %3s |", GetChannelOnOff(modID, ch) ? "On" : "Off" );
    printf(" %4s |", GetChannelGain(modID, ch) ? "x1" : "1/4" );
    printf(" %6.2f |", GetChannelTriggerRiseTime(modID, ch));
    printf(" %6.2f |", GetChannelTriggerFlatTop(modID, ch));
    printf(" %9.2f |", GetChannelTriggerThreshold(modID, ch));
    printf(" %8s |", GetChannelPolarity(modID, ch) ? "Pos" : "Neg");
    printf(" %8.2f |", GetChannelEnergyRiseTime(modID, ch));
    printf(" %8.2f |", GetChannelEnergyFlatTop(modID, ch));
    printf(" %5.2f |", GetChannelEnergyTau(modID, ch));
    if( GetChannelTraceOnOff(modID, ch) ){
      printf(" %7.2f |", GetChannelTraceLength(modID, ch));
      printf(" %7.2f |", GetChannelTraceDelay(modID, ch));        
    }else{
      printf(" %7s |", "Off");
      printf(" %7s |", "Off");
    }    
    printf(" %5.2f |", GetChannelVOffset(modID, ch));
    printf(" %4.2f %% \n", GetChannelBaseLinePrecent(modID, ch));
  }
}


void Pixie16::SetChannelSetting(std::string parName, double val, unsigned short modID, unsigned short ch, bool verbose ){
  
  retval = Pixie16WriteSglChanPar( const_cast<char*> (parName.c_str()), val, modID, ch);
  if( CheckError("Pixie16WriteSglChanPar::"+parName) < 0 ) return;
  if( verbose ) GetChannelSetting(parName, modID, ch, verbose);
  
}

void Pixie16::SwitchCSRA(int bitwise, unsigned short modID, unsigned short ch){
  
  double ParData;
  retval = Pixie16ReadSglChanPar( (char *)"CHANNEL_CSRA", &ParData, modID, ch);
  if( CheckError("Pixie16ReadSglChanPar::CHANNEL_CSRA") < 0 ) return;
  
  ParData = ((int)ParData) ^ bitwise; 
  SetChannelSetting("CHANNEL_CSRA", ParData, modID, ch);
  
}

void Pixie16::SetCSRABit(int bitwise, unsigned short val, unsigned short modID, unsigned short ch){
  
  double ParData;
  retval = Pixie16ReadSglChanPar( (char *)"CHANNEL_CSRA", &ParData, modID, ch);
  if( CheckError("Pixie16ReadSglChanPar::CHANNEL_CSRA") < 0 ) return;
  
  std::cout << std::bitset<32>(ParData) << std::endl;     
  std::cout << std::bitset<32>(~bitwise) << std::endl;     
  
  int temp = ((int)ParData) & ~bitwise ;
  int haha = 0;
  if( bitwise == CSRA_BIT::FAST_TRIGGER           ) haha = ((val & 0x1) <<  0);
  if( bitwise == CSRA_BIT::M_VALIDATION           ) haha = ((val & 0x1) <<  1);
  if( bitwise == CSRA_BIT::ENABLE_CHANNEL         ) haha = ((val & 0x1) <<  2);
  if( bitwise == CSRA_BIT::C_VALIFATION           ) haha = ((val & 0x1) <<  3);
  if( bitwise == CSRA_BIT::BLOCK_DAQ_DPM_FULL     ) haha = ((val & 0x1) <<  4);
  if( bitwise == CSRA_BIT::POLARITY               ) haha = ((val & 0x1) <<  5);
  if( bitwise == CSRA_BIT::VETO_TRIGGER           ) haha = ((val & 0x1) <<  6);
  if( bitwise == CSRA_BIT::HIST_ENERGY            ) haha = ((val & 0x1) <<  7);
  if( bitwise == CSRA_BIT::ENABLE_TRACE           ) haha = ((val & 0x1) <<  8);
  if( bitwise == CSRA_BIT::ENABLE_QDC             ) haha = ((val & 0x1) <<  9);
  if( bitwise == CSRA_BIT::ENABLE_CFD             ) haha = ((val & 0x1) << 10);
  if( bitwise == CSRA_BIT::REQ_M_VALIDATION       ) haha = ((val & 0x1) << 11);
  if( bitwise == CSRA_BIT::CAPTURE_ESUMS_BASELINE ) haha = ((val & 0x1) << 12);
  if( bitwise == CSRA_BIT::REQ_C_VALIDATION       ) haha = ((val & 0x1) << 13);
  if( bitwise == CSRA_BIT::INPUT_RELAY            ) haha = ((val & 0x1) << 14);
  if( bitwise == CSRA_BIT::PILEUP                 ) haha = ((val & 0x1) << 15);
  if( bitwise == CSRA_BIT::NO_TRACE_LARGE_PULSE   ) haha = ((val & 0x1) << 17);
  if( bitwise == CSRA_BIT::GROUP_TRIGGER          ) haha = ((val & 0x1) << 18);
  if( bitwise == CSRA_BIT::CH_VETO                ) haha = ((val & 0x1) << 19);
  if( bitwise == CSRA_BIT::MO_VETO                ) haha = ((val & 0x1) << 20);
  if( bitwise == CSRA_BIT::EXT_TIMESTAMP          ) haha = ((val & 0x1) << 21);
  
  std::cout << std::bitset<32>(temp) << std::endl;     
  std::cout << std::bitset<32>(haha) << std::endl;     
  
  std::cout << std::bitset<32>(temp | haha) << std::endl;       
  
  SetChannelSetting("CHANNEL_CSRA", (temp | haha), modID, ch, true);
  
}

unsigned int * Pixie16::GetStatitics(unsigned short modID){
  int ss = Pixie16GetStatisticsSize();
  if( Statistics != NULL ) delete Statistics;
  Statistics = new unsigned int [ss];
  retval = Pixie16ReadStatisticsFromModule (Statistics, modID);
  if( CheckError("Pixie16ReadStatisticsFromModule") < 0 ) return NULL;
  return Statistics;
}

void Pixie16::PrintStatistics(unsigned short modID){
  
  GetStatitics(modID);
  if( retval >= 0 ){
    double realTime = Pixie16ComputeRealTime (Statistics, modID);
    printf(" Real (or RUN) Time : %9.3f sec \n", realTime);
    printf("  ch | live time (sec) | input count rate | output count rate | trigger | events \n");
    printf("-----+-----------------+------------------+-------------------+---------+--------\n");
    for( int ch = 0; ch < 16; ch ++){
      printf("  %2d |", ch);
      double liveTime = Pixie16ComputeLiveTime(Statistics, modID, ch);
      double ICR = Pixie16ComputeInputCountRate(Statistics, modID, ch);
      double OCR = Pixie16ComputeOutputCountRate(Statistics, modID, ch);
      printf(" %15.4f |", liveTime);
      printf(" %16.4f |", ICR);
      printf(" %17.4f |", OCR);
      printf(" %7d |", (int) (ICR * liveTime) );
      printf(" %7d \n", (int) (OCR * realTime) );
    }
  }
}

double Pixie16::GetInputCountRate(unsigned short modID, unsigned short ch){
  int ss = Pixie16GetStatisticsSize();
  unsigned int statistics[ss];
  retval = Pixie16ReadStatisticsFromModule (statistics, modID);
  if( CheckError("Pixie16ReadStatisticsFromModule") < 0 ) return -404;
  return Pixie16ComputeInputCountRate (Statistics, modID, ch);
}
double Pixie16::GetLiveTime(unsigned short modID, unsigned short ch){
  int ss = Pixie16GetStatisticsSize();
  unsigned int statistics[ss];
  retval = Pixie16ReadStatisticsFromModule (statistics, modID);
  if( CheckError("Pixie16ReadStatisticsFromModule") < 0 ) return -404;
  return Pixie16ComputeLiveTime (statistics, modID, ch);
}

double Pixie16::GetOutputCountRate(unsigned short modID, unsigned short ch){
  int ss = Pixie16GetStatisticsSize();
  unsigned int statistics[ss];
  retval = Pixie16ReadStatisticsFromModule (statistics, modID);
  if( CheckError("Pixie16ReadStatisticsFromModule") < 0 ) return -404;
  return Pixie16ComputeOutputCountRate (statistics, modID, ch);
}
double Pixie16::GetRealTime(unsigned short modID){
  int ss = Pixie16GetStatisticsSize();
  unsigned int statistics[ss];
  retval = Pixie16ReadStatisticsFromModule (statistics, modID);
  if( CheckError("Pixie16ReadStatisticsFromModule") < 0 ) return -404;
  return Pixie16ComputeRealTime (statistics, modID);
}

void Pixie16::SaveSettings(std::string fileName){
  
  retval = Pixie16SaveDSPParametersToFile((char *)fileName.c_str());
  if( CheckError("Pixie16SaveDSPParametersToFile") < 0 ) {
    return;
  }else{
    printf("saved setting to %s\n", fileName.c_str());
  }
}

void Pixie16::OpenFile(std::string fileName, bool append){
  if( !outFile.is_open() ) {
    if( append ) {
      outFile.open(fileName, std::ios::out | std::ios::binary | std::ios::app);
    }else{
      outFile.open(fileName, std::ios::out | std::ios::binary);
    }
  }
}

void Pixie16::SaveData(){
  if( outFile.is_open() ) outFile.write(reinterpret_cast<char*>(ExtFIFO_Data), nFIFOWords * sizeof(uint32_t));
}

long int Pixie16::GetFileSize(){
  
  if( outFile.is_open() ){
    return outFile.tellg();
  }else{
    return 0; 
  }
  
}

void Pixie16::CloseFile(){
  outFile.close();
}
  
#endif

