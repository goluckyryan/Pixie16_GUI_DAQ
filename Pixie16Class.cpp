#ifndef PIXIE16_CPP
#define PIXIE16_CPP


//#include "pixie16app_export.h"
//#include "pixie16sys_export.h"
//#include "def21160.h"

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
    LoadConfigFile("");
    
    BootDigitizers();

    nFIFOWords = 0;
    ExtFIFO_Data = NULL;
    Statistics = NULL;
    
    data = new DataBlock();
    nextWord = 0;
  }
}

Pixie16::~Pixie16(){
  
  retval = Pixie16ExitSystem(NumModules);
  CheckError("Pixie16ExitSystem");
  
  delete PXISlotMap;
  
  delete ComFPGAConfigFile;
  delete SPFPGAConfigFile;
  delete TrigFPGAConfigFile;
  delete DSPCodeFile;
  delete DSPParFile;
  delete DSPVarFile;

  delete ExtFIFO_Data;
  delete Statistics;
  delete data;
  
}


bool Pixie16::LoadConfigFile(std::string fileName){
  
  printf("\033[32m======= Loading Configuration file : \033[m\n");
  
  NumModules = 1;
  OfflineMode = 0;
  PXISlotMap = new unsigned short[NumModules];
  
  ComFPGAConfigFile  = new char* [NumModules];
  SPFPGAConfigFile   = new char* [NumModules];
  TrigFPGAConfigFile = new char* [NumModules];
  DSPCodeFile        = new char* [NumModules];
  DSPParFile         = new char* [NumModules];
  DSPVarFile         = new char* [NumModules];
  
  PXISlotMap[0] = 2;
  
  ComFPGAConfigFile  [0] = (char *)"/usr/opt/Pixie16/pixie16_revf_general_12b250m_41847_2019-05-18/firmware/syspixie16_revfgeneral_adc250mhz_r33339.bin";
  SPFPGAConfigFile   [0] = (char *)"/usr/opt/Pixie16/pixie16_revf_general_12b250m_41847_2019-05-18/firmware/fippixie16_revfgeneral_12b250m_r42081.bin";
  TrigFPGAConfigFile [0] = (char *)"FPGATrig"; ///only Revision A
  DSPCodeFile        [0] = (char *)"/usr/opt/Pixie16/pixie16_revf_general_12b250m_41847_2019-05-18/dsp/Pixie16DSP_revfgeneral_12b250m_r41847.ldr";
  DSPVarFile         [0] = (char *)"/usr/opt/Pixie16/pixie16_revf_general_12b250m_41847_2019-05-18/dsp/Pixie16DSP_revfgeneral_12b250m_r41847.var";
  
  //DSPParFile         [0] = (char *)"/usr/opt/Pixie16/Pixie-16_FSU_Custom_Firmware_06022020/Configuration/Pixie16_FSU_Sample_Setup.set";
  DSPParFile         [0] = (char *)"/home/ryan/Pixie16/ryan/test_ryan.set";
  //DSPParFile         [0] = (char *)"/home/ryan/Pixie16/ryan/Pixie16_FSU_Sample_Setup.set";

  BootPattern = 0x7F;
  
  printf("Number of Module : %d \n", NumModules);
  printf("Slot Map : "); for( int i = 0; i < NumModules ; i++) printf("%d\t", PXISlotMap[i]);
  printf("\n");
  for( int i = 0; i < NumModules; i++){
    printf("--- configuration files for module-%d\n", i);
    printf("  ComFPGA : %s \n", ComFPGAConfigFile[i]);
    printf("   SPFPGA : %s \n", SPFPGAConfigFile[i]);
    printf(" DSP Code : %s \n", DSPCodeFile[i]);
    printf(" DSP Var  : %s \n", DSPVarFile[i]);
    printf(" DSP Par  : %s \n", DSPParFile[i]);
  }
  
  return true;  
}

int Pixie16::CheckError(std::string operation){
  
  if( retval < 0 ){
    printf("\033[1;31m*ERROR* \033[1;33m%s\033[m failed, retval = %d\n", operation.c_str(), retval);
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
   printf("found PLX PCI 9054, but not working. Unknown header Type.\n");
   retval = -3;
   return; 
  }
  printf("PLX PCI 9054 card does not detected problem.\n");
  retval = 1;
}


void Pixie16::GetDigitizerInfo(unsigned short modID){

  unsigned short ModRev;
  unsigned int ModSerNum;
  unsigned short ModADCBits;
  unsigned short ModADCMSPS;
  retval = Pixie16ReadModuleInfo(modID, &ModRev, &ModSerNum, &ModADCBits, &ModADCMSPS);
  
  if( CheckError("Pixie16ReadModuleInfo") < 0 ) return ;
  
  printf("------------ Module-%d \n", modID);
  printf("         Revision : %d \n", ModRev);
  printf("       Serial Num : %d \n", ModSerNum);
  printf("         ADC Bits : %d \n", ModADCBits);
  printf("ADC sampling rate : %d \n", ModADCMSPS);
  
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


void Pixie16::AdjustOffset(){
  
  retval = Pixie16AdjustOffsets(NumModules);
  if( CheckError("Pixie16AdjustOffsets") < 0 ) return;
  printf(" Adjust Offset for All modules.\n");
  
}

void Pixie16::StartRun(bool listMode){
  
  unsigned short mode = NEW_RUN; //RESUME_RUN
  
  //listmode
  if( listMode ){
    
    //SetDigitizerSynchWait(1, i);
    //SetDigitizerInSynch(0, i);
    
    retval = Pixie16StartListModeRun(NumModules, LIST_MODE_RUN, mode);
    if( CheckError("Pixie16StartListModeRun") < 0 ) return;
    printf("\033[32m LIST_MODE run\033[0m\n");
  }else{
    //MCA mode
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


void Pixie16::ReadData(unsigned short modID){

  if( Pixie16CheckRunStatus(modID) == 1){
    unsigned int oldnFIFOWords = nFIFOWords;
    retval = Pixie16CheckExternalFIFOStatus (&nFIFOWords, modID);
    if( CheckError("Pixie16CheckExternalFIFOStatus") < 0 ) return;
    if(nFIFOWords *1.0 / EXTERNAL_FIFO_LENGTH > 0.2) {
    //if(nFIFOWords  > 0) {
      printf("\033[1;31m####### READ DATA \033[m: number of word in module-%d FIFO : %d \n", modID, nFIFOWords);
      if( ExtFIFO_Data != NULL ) delete ExtFIFO_Data;
      ExtFIFO_Data = new unsigned int [nFIFOWords];
      retval = Pixie16ReadDataFromExternalFIFO(ExtFIFO_Data, nFIFOWords, modID);
      CheckError("Pixie16ReadDataFromExternalFIFO");
      nextWord = nextWord - oldnFIFOWords;
    }
  }else{
    printf("Pixie16 is not running.\n");
  }
}

void Pixie16::ProcessSingleData(){
  
  if( nextWord < nFIFOWords ){
  
    data->ch           =  ExtFIFO_Data[nextWord] & 0xF ;
    data->slot         = (ExtFIFO_Data[nextWord] >> 4) & 0xF;
    data->crate        = (ExtFIFO_Data[nextWord] >> 8) & 0xF;
    data->headerLength = (ExtFIFO_Data[nextWord] >> 12) & 0x1F;
    data->eventLength  = (ExtFIFO_Data[nextWord] >> 17) & 0x3FFF;
    data->pileup       =  ExtFIFO_Data[nextWord] >> 31 ;
    data->eventID ++;
    
    if( nextWord + data->eventLength < nFIFOWords ){
    
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
      }else{
        data->ClearTrace();
      }
    }else{
      data->time = 0;
      data->cfd = 0;
      data->energy = 0;
      data->trace_length = 0;
      data->trace_out_of_range = 0;
    }
    
    nextWord += data->eventLength ;
  }

}

void Pixie16::ProcessData(int verbose){

  if( verbose >= 2 ) for( unsigned int i = 0; i < nFIFOWords; i++) printf("%5d|%X|\n", i, ExtFIFO_Data[nextWord+i]);
  
  while( nextWord < nFIFOWords ){
    ProcessSingleData();
    if( verbose >= 1 ) data->Print(0);
    if( verbose >= 3 ) data->Print(1); /// print trace
  }
  
}

void Pixie16::GetTrace(unsigned short modID, unsigned short ch){
  
  unsigned short ADCTrace[8192];
  retval = Pixie16AcquireADCTrace (modID);
  if( CheckError("Pixie16AcquireADCTrace") < 0 ) return;
  
  retval = Pixie16ReadSglChanADCTrace (ADCTrace, 8192, modID, ch);
  if( CheckError("Pixie16ReadSglChanADCTrace") < 0 ) return;
  
  for( int i = 0; i < 8192 ; i++){
    printf("%4d, %d \n", i, ADCTrace[i]);
  }
}


void Pixie16::GetBaseLines(unsigned short modID, unsigned short ch){
  
  retval = Pixie16AcquireBaselines(modID);
  if( CheckError("Pixie16AcquireBaselines") < 0 ) return;
  

  double Baselines[3640], TimeStamps[3640];
  
  retval = Pixie16ReadSglChanBaselines(Baselines, TimeStamps, 3640, modID, ch);
  if( CheckError("Pixie16ReadSglChanBaselines") < 0 ) return;
  
  for( int i = 0; i < 3640; i++){
    printf("%4d, %.4f, %.4f \n", i, Baselines[i], TimeStamps[i]);
  }
  
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
        printf("                                        signal polarity (bit:  5) : %s \n", CSRA & CSRA_BIT::POLARITY ? "Positive" : "Negative");
        printf("                                   veto channel trigger (bit:  6) : %s \n", CSRA & CSRA_BIT::VETO_TRIGGER ? "enable" : "disable");
        printf("                                   Enable trace capture (bit:  8) : %s \n", CSRA & CSRA_BIT::ENABLE_TRACE ? "enable" : "disable");
        printf("                                 Enable QDC sum capture (bit:  9) : %s \n", CSRA & CSRA_BIT::ENABLE_QDC ? "enable" : "disable");
        printf("                                Enable CFD trigger mode (bit: 10) : %s \n", CSRA & CSRA_BIT::ENABLE_CFD ? "enable" : "disable");
        printf("                     required module validation trigger (bit: 11) : %s \n", CSRA & CSRA_BIT::REQ_M_VALIDATION ? "required" : "not required");
        printf("           Enable capture raw energy sums and baselines (bit: 12) : %s \n", CSRA & CSRA_BIT::CAPTURE_ESUMS_BASELINE ? "enable" : "disable");
        printf("                    required cahnnel validation trigger (bit: 13) : %s \n", CSRA & CSRA_BIT::REQ_C_VALIDATION ? "required" : "not required");
        printf("                                     Enable input relay (bit: 14) : %s \n", CSRA & CSRA_BIT::INPUT_RELAY ? "enable" : "disable");
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

void Pixie16::PrintChannelsMainSettings(unsigned short modID){

  printf("====+=====+========+========+===========+==========+==========+==========+========+========+=========+======+====== \n");  
  printf(" ch | En  | Trig_L | Trig_G | Threshold | Polarity | Energy_L | Energy_G | Tau    | Trace  | Trace_d | Voff | BL \n");
  printf("----+-----+--------+--------+-----------+----------+----------+----------+--------+--------+---------+------+------ \n");
  for( int ch = 0; ch < 16; ch ++){
    printf(" %2d |", ch);
    printf(" %3s |", GetChannleOnOff(modID, ch) ? "On" : "Off" );
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
    printf(" %4.2f |", GetChannelVOffset(modID, ch));
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
  
  int temp = ((int)ParData) & ~bitwise ;
  int haha = 0;
  if( bitwise == CSRA_BIT::FAST_TRIGGER           ) haha = val <<  0;
  if( bitwise == CSRA_BIT::M_VALIDATION           ) haha = val <<  1;
  if( bitwise == CSRA_BIT::ENABLE_CHANNEL         ) haha = val <<  2;
  if( bitwise == CSRA_BIT::C_VALIFATION           ) haha = val <<  3;
  if( bitwise == CSRA_BIT::BLOCK_DAQ_DPM_FULL     ) haha = val <<  4;
  if( bitwise == CSRA_BIT::POLARITY               ) haha = val <<  5;
  if( bitwise == CSRA_BIT::VETO_TRIGGER           ) haha = val <<  6;
  if( bitwise == CSRA_BIT::HIST_ENERGY            ) haha = val <<  7;
  if( bitwise == CSRA_BIT::ENABLE_TRACE           ) haha = val <<  8;
  if( bitwise == CSRA_BIT::ENABLE_QDC             ) haha = val <<  9;
  if( bitwise == CSRA_BIT::ENABLE_CFD             ) haha = val << 10;
  if( bitwise == CSRA_BIT::REQ_M_VALIDATION       ) haha = val << 11;
  if( bitwise == CSRA_BIT::CAPTURE_ESUMS_BASELINE ) haha = val << 12;
  if( bitwise == CSRA_BIT::REQ_C_VALIDATION       ) haha = val << 13;
  if( bitwise == CSRA_BIT::INPUT_RELAY            ) haha = val << 14;
  if( bitwise == CSRA_BIT::PILEUP                 ) haha = val << 15;
  if( bitwise == CSRA_BIT::NO_TRACE_LARGE_PULSE   ) haha = val << 17;
  if( bitwise == CSRA_BIT::GROUP_TRIGGER          ) haha = val << 18;
  if( bitwise == CSRA_BIT::CH_VETO                ) haha = val << 19;
  if( bitwise == CSRA_BIT::MO_VETO                ) haha = val << 20;
  if( bitwise == CSRA_BIT::EXT_TIMESTAMP          ) haha = val << 21;
  
  SetChannelSetting("CHANNEL_CSRA", (temp | haha), modID, ch);
  
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

void Pixie16::CloseFile(){
  outFile.close();
}
  
#endif

