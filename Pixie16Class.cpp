#ifndef PIXIE16_CPP
#define PIXIE16_CPP

#include "pixie16/pixie16.h"
//#include "pixie16app_export.h"
//#include "pixie16sys_export.h"
//#include "def21160.h"

#include <bitset>
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

void Pixie16::StartRun(bool listMode){
  
  unsigned short mode = NEW_RUN; //RESUME_RUN
  
  for( int i = 0 ; i < NumModules; i++){
    retval = Pixie16StartListModeRun(i, 0x100, mode);
    if( CheckError("Pixie16StartListModeRun") < 0 ) return;
    printf("\033[32mModule-%d run\033[0m\n", i);
    
    isRunning = true;
  }

}


void Pixie16::StopRun(){
  
  retval =  Pixie16EndRun( NumModules );
  if( CheckError("Pixie16EndRun") < 0 ) return;
  printf("\033[32mRun Stopped\033[0m\n");
  
  isRunning = false;
  
}


void Pixie16::ReadData(unsigned short modID){

  retval = Pixie16CheckExternalFIFOStatus (&nFIFOWords, modID);

  printf("number of word in module-%d FIFO : %d \n", modID, nFIFOWords);

  if(nFIFOWords > 0) {
    if( ExtFIFO_Data != NULL ) delete ExtFIFO_Data;
    ExtFIFO_Data = new unsigned int [nFIFOWords];
    retval = Pixie16ReadDataFromExternalFIFO(ExtFIFO_Data, nFIFOWords, modID);
    if( CheckError("Pixie16ReadDataFromExternalFIFO") < 0 ) return;
  }

}

void Pixie16::PrintData(){
  
  printf("----------------------------\n");
  printf("number of words read : %d \n", nFIFOWords);
  
  unsigned int word = 0;

  for( unsigned int i = 0; i < nFIFOWords; i++) printf("%5d|%X|\n", i, ExtFIFO_Data[word]);
  
  while( word < nFIFOWords ){
    data->ch           =  ExtFIFO_Data[word] & 0xF ;
    data->slot         = (ExtFIFO_Data[word] >> 4) & 0xF;
    data->crate        = (ExtFIFO_Data[word] >> 8) & 0xF;
    data->headerLength = (ExtFIFO_Data[word] >> 12) & 0x1F;
    data->eventLength  = (ExtFIFO_Data[word] >> 17) & 0x3FFF;
    data->pileup       =  ExtFIFO_Data[word] >> 31 ;
    data->time         = ((unsigned long long)(ExtFIFO_Data[word+2] & 0xFFFF) << 32) + ExtFIFO_Data[word+1];
    data->cfd          =  ExtFIFO_Data[word + 2] >> 16 ; 
    data->energy       = (ExtFIFO_Data[word + 3] & 0xFFFF ); 
    data->trace_length = (ExtFIFO_Data[word + 3] >> 16) & 0x7FFF;
    data->trace_out_of_range =  ExtFIFO_Data[word + 3] >> 31;
    
    data->Print(0);
    data->eventID ++;
        
    word += data->eventLength + 1;
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

void Pixie16::GetDigitizerSettings(unsigned short modID){
  
  printf("=========== Digitizer setting for module-%d\n", modID);
  
  unsigned int ParData;
  retval = Pixie16ReadSglModPar ((char *)"MODULE_NUMBER",        &ParData, modID); printf("             module number: %d \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"CrateID",              &ParData, modID); printf("               Crate modID: %d \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"SlotID",               &ParData, modID); printf("                Slot modID: %d \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"ModID",                &ParData, modID); printf("                 Mod modID: %d \n", ParData);
  ///retval = Pixie16ReadSglModPar ((char *)"MODULE_CSRA",          &ParData, modID); printf("channel control registor A: %X \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"MODULE_CSRB",          &ParData, modID); printf("channel control registor B: %X \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"MODULE_FORMAT",        &ParData, modID); printf("                    format: %d \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"MAX_EVENTS",           &ParData, modID); printf("                max events: %d \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"SYNCH_WAIT",           &ParData, modID); printf("               syn ch wait: %d \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"IN_SYNCH",             &ParData, modID); printf("                 in syn ch: %d \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"SLOW_FILTER_RANGE",    &ParData, modID); printf("         slow filter range: %d \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"FAST_FILTER_RANGE",    &ParData, modID); printf("         fast filter range: %d \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"FastTrigBackplaneEna", &ParData, modID); printf("fast trig Backplane enable: %X \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"TrigConfig0",          &ParData, modID); printf("             Trig config 0: %X \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"TrigConfig1",          &ParData, modID); printf("             Trig config 1: %X \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"TrigConfig2",          &ParData, modID); printf("             Trig config 2: %X \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"TrigConfig3",          &ParData, modID); printf("             Trig config 3: %X \n", ParData);
  retval = Pixie16ReadSglModPar ((char *)"HOST_RT_PRESET",       &ParData, modID); printf("      Host run time preset: %d \n", ParData);
  
  if( CheckError("GetDigitizerSettings") < 0 ) return;
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
      printf("READ | Mod : %2d, CH: %2d, %18s = %X\n", modID, ch, parName.c_str(), (int) ParData);
      printf(     "                                 32  28  24  20  16  12   8   4   0\n");
      printf(     "                                  |   |   |   |   |   |   |   |   |\n");
      std::cout <<"         Module Configuration : 0xb" << std::bitset<32>(ParData) << std::endl;     
      
      if( parName == "CHANNEL_CSRA" ){
        int CSRA = (int) ParData;
        printf("---------------------------------------------------------------------------\n");
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
  
  printf("===================== Channel setting. Mod-%d CH-%02d\n", modID, ch);
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
  GetChannelSetting("CHANNEL_CSRA",       modID, ch, true); 
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
  printf("=====================================\n");
 }

void Pixie16::PrintChannelsMainSettings(unsigned short modID){
  
  printf(" ch | En  | Trig_L | Trig_G | Threshold | Energy_L | Energy_G | Tau   | Trace   | Trace_d | Voff | BL \n");
  printf("----+-----+--------+--------+-----------+----------+----------+-------+------  -+---------+------+------ \n");
  for( int ch = 0; ch < 16; ch ++){
    printf(" %2d |", ch);
    printf(" %3s |", GetChannleOnOff(modID, ch) ? "On" : "Off" );
    printf(" %6.2f |", GetChannelTriggerRiseTime(modID, ch));
    printf(" %6.2f |", GetChannelTriggerFlatTop(modID, ch));
    printf(" %9.2f |", GetChannelTriggerThreshold(modID, ch));
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
    printf(" %4.2f \n", GetChannelBaseLinePrecent(modID, ch));
  }
}


void Pixie16::WriteChannelSetting(std::string parName, double val, unsigned short modID, unsigned short ch, bool verbose ){
  
  retval = Pixie16WriteSglChanPar( const_cast<char*> (parName.c_str()), val, modID, ch);
  if( CheckError("Pixie16WriteSglChanPar::"+parName) < 0 ) return;
  if( verbose ) GetChannelSetting(parName, modID, ch, verbose);
  
}

void Pixie16::SwitchCSRA(int bitwise, unsigned short modID, unsigned short ch){
  
  double ParData;
  retval = Pixie16ReadSglChanPar( (char *)"CHANNEL_CSRA", &ParData, modID, ch);
  if( CheckError("Pixie16ReadSglChanPar::CHANNEL_CSRA") < 0 ) return;
  
  ParData = ((int)ParData) ^ bitwise; 
  WriteChannelSetting("CHANNEL_CSRA", ParData, modID, ch);
  
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
  
  WriteChannelSetting("CHANNEL_CSRA", (temp | haha), modID, ch);
  
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
    printf(" Real (or RUN) Time : %9.3f sec \n", Pixie16ComputeRealTime (Statistics, modID));
    printf("  ch | live time (sec) | input count rate | output count rate \n");
    printf("-----+-----------------+------------------+-------------------\n");
    for( int ch = 0; ch < 16; ch ++){
      printf("  %2d |", ch);
      printf(" %15.4f |", Pixie16ComputeLiveTime(Statistics, modID, ch));
      printf(" %16.4f |", Pixie16ComputeInputCountRate(Statistics, modID, ch));
      printf(" %17.4f \n", Pixie16ComputeOutputCountRate(Statistics, modID, ch));
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

/*
void Pixie16::SaveData(char * fileName, unsigned short isEndOfRun){

  retval = Pixie16SaveExternalFIFODataToFile(fileName, &nFIFOWords, 0, isEndOfRun);
  if( CheckError("Pixie16SaveExternalFIFODataToFile") < 0 ) return;
  
}*/

  
#endif

