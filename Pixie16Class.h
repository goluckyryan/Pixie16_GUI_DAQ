#ifndef PIXIE16_H
#define PIXIE16_H

#include <string>
#include <fstream>
#include "pixie16/pixie16.h"
#include "DataBlock.h"

enum CSRA_BIT{
  FAST_TRIGGER           = 0x00000001,
  M_VALIDATION           = 0x00000002,
  ENABLE_CHANNEL         = 0x00000004,
  C_VALIFATION           = 0x00000008,
  BLOCK_DAQ_DPM_FULL     = 0x00000010,
  POLARITY               = 0x00000020,
  VETO_TRIGGER           = 0x00000040,
  HIST_ENERGY            = 0x00000080,
  ENABLE_TRACE           = 0x00000100,
  ENABLE_QDC             = 0x00000200,
  ENABLE_CFD             = 0x00000400,
  REQ_M_VALIDATION       = 0x00000800,
  CAPTURE_ESUMS_BASELINE = 0x00001000,
  REQ_C_VALIDATION       = 0x00002000,
  INPUT_RELAY            = 0x00004000,
  PILEUP                 = 0x00018000,
  NO_TRACE_LARGE_PULSE   = 0x00020000,
  GROUP_TRIGGER          = 0x00040000,
  CH_VETO                = 0x00080000,
  MO_VETO                = 0x00100000,
  EXT_TIMESTAMP          = 0x00200000,
};

enum MOD_CSRB_BIT{
  WIRED_OR_TRIGGER_TO_PULLUP_RESIST  = 0x00000001,
  DIRECT_MODULUE                     = 0x00000010,
  CHASSIS_MASTER                     = 0x00000040,
  GLOBAL_FAST_TRIGGER                = 0x00000080,
  EXTERNAL_TRIGGER                   = 0x00000100,
  USE_INHIBIT                        = 0x00000400,
  DISTRIBUTE_CLOCK                   = 0x00000800,
  SORT_TIMESTAMP                     = 0x00001000,
  FAST_TRIGGER_TO_BACKPLANE          = 0x00002000,
};

class Pixie16 {
  
private:
  unsigned short   NumCrates;
  unsigned short   NumModules;
  unsigned short * PXISlotMap;
  unsigned short   OfflineMode;
  
  char ** ComFPGAConfigFile;
  char ** SPFPGAConfigFile;
  char ** TrigFPGAConfigFile;
  char ** DSPCodeFile;
  char ** DSPParFile;
  char ** DSPVarFile;
  
  
  unsigned short * ModRev;
  unsigned int   * ModSerNum;
  unsigned short * ModADCBits;
  unsigned short * ModADCMSPS;
  unsigned short * numChannels;
  unsigned short * ch2ns;
  
  unsigned short BootPattern ;
  
  int retval;
  
  bool isRunning;
  
  int CheckError(std::string operation);
  
  unsigned int nFIFOWords;
  unsigned int * ExtFIFO_Data;
  unsigned int * Statistics;
  
  double Baselines[3640], TimeStamps[3640];  ///for baseline
  unsigned short ADCTrace[8192];
    
  DataBlock * data;
  unsigned int nextWord;
  
  std::ofstream outFile;
  
public:

  Pixie16();
  ~Pixie16();

  int GetStatus() {return retval;}
  bool IsRunning() {return isRunning;}

  ///======================== startup
  void LoadConfigFile( bool verbose = false, std::string fileName = "Pixie16.config");

  void CheckDriver();
  void CheckHardware();
  
  void GetDigitizerInfo(unsigned short modID);
  void BootDigitizers();
  void CloseDigitizers();
  
  unsigned short GetCh2ns(unsigned short modID) {return ch2ns[modID];}
    
  void AdjustOffset();
  void CaptureBaseLine(unsigned short modID, unsigned short ch);
  int      GetBaslineLength()     {return 3640;}
  double * GetBasline()            {return Baselines;}
  double * GetBaselineTimestamp()  {return TimeStamps;}
  
  void CaptureADCTrace(unsigned short modID, unsigned short ch);
  int GetADCTraceLength()          {return 8192;}
  unsigned short * GetADCTrace()   {return ADCTrace;}
  
  ///========================= Setting

  unsigned int GetNumModule() {return NumModules;}
  unsigned int GetDigitizerSerialNumber(unsigned short modID) { return ModSerNum[modID];} 
  unsigned short GetDigitizerRev(unsigned short modID) { return ModRev[modID];}
  unsigned short GetDigitizerADCBits(unsigned short modID) { return ModADCBits[modID];}
  unsigned short GetDigitizerNumChannel(unsigned short modID) { return numChannels[modID];}

  unsigned int GetDigitizerSetting(std::string parName, unsigned short modID, bool verbose = false);

  void PrintDigitizerSettings(unsigned short modID);
  
  void SetDigitizerSetting(std::string parName, unsigned int val, unsigned short modID, bool verbose = false);
  void SetDigitizerSynchWait(unsigned int val, unsigned short modID)              { SetDigitizerSetting("SYNCH_WAIT", val, modID, 1);}
  void SetDigitizerInSynch(unsigned int val, unsigned short modID)                { SetDigitizerSetting("IN_SYNCH", val, modID, 1);}
  void SetDigitizerPresetRunTime(double val_in_sec, unsigned short modID)   { SetDigitizerSetting("HOST_RT_PRESET", Decimal2IEEEFloating(val_in_sec), modID, 1);}
  
  double GetChannelSetting(std::string parName, unsigned short modID, unsigned short ch, bool verbose = false);
  double GetChannelTriggerRiseTime (unsigned modID, unsigned short ch){ return GetChannelSetting("TRIGGER_RISETIME", modID, ch); }
  double GetChannelTriggerFlatTop  (unsigned modID, unsigned short ch){ return GetChannelSetting("TRIGGER_FLATTOP", modID, ch); }
  double GetChannelTriggerThreshold(unsigned modID, unsigned short ch){ return GetChannelSetting("TRIGGER_THRESHOLD", modID, ch); }
  double GetChannelEnergyRiseTime  (unsigned modID, unsigned short ch){ return GetChannelSetting("ENERGY_RISETIME", modID, ch); }
  double GetChannelEnergyFlatTop   (unsigned modID, unsigned short ch){ return GetChannelSetting("ENERGY_FLATTOP", modID, ch); }  
  double GetChannelEnergyTau       (unsigned modID, unsigned short ch){ return GetChannelSetting("TAU", modID, ch); }  
  double GetChannelTraceLength     (unsigned modID, unsigned short ch){ return GetChannelSetting("TRACE_LENGTH", modID, ch); }  
  double GetChannelTraceDelay      (unsigned modID, unsigned short ch){ return GetChannelSetting("TRACE_DELAY", modID, ch); }  
  double GetChannelVOffset         (unsigned modID, unsigned short ch){ return GetChannelSetting("VOFFSET", modID, ch); }  
  double GetChannelBaseLinePrecent (unsigned modID, unsigned short ch){ return GetChannelSetting("BASELINE_PERCENT",   modID, ch);  }  
  
  unsigned short GetCSRA(int bitwise,  unsigned short modID, unsigned short ch, bool verbose = false);
  bool GetChannelOnOff(unsigned short modID, unsigned short ch, bool verbose = false)        {return GetCSRA(CSRA_BIT::ENABLE_CHANNEL, modID, ch, verbose);}
  bool GetChannelPolarity(unsigned short modID, unsigned short ch, bool verbose = false)     {return GetCSRA(CSRA_BIT::POLARITY, modID, ch, verbose);}
  bool GetChannelTraceOnOff(unsigned short modID, unsigned short ch, bool verbose = false)   {return GetCSRA(CSRA_BIT::ENABLE_TRACE, modID, ch, verbose);}
  bool GetChannelGain(unsigned short modID, unsigned short ch, bool verbose = false)         {return GetCSRA(CSRA_BIT::INPUT_RELAY, modID, ch, verbose);}

  void PrintChannelAllSettings(unsigned short modID, unsigned short ch);
  void PrintChannelsMainSettings(unsigned short modID);
 

  void SetChannelSetting(std::string parName, double val, unsigned short modID, unsigned short ch, bool verbose = false);
  void SetChannelTriggerRiseTime (double val, unsigned short modID, unsigned short ch){ SetChannelSetting("TRIGGER_RISETIME",  val, modID, ch, 1);}
  void SetChannelTriggerFlatTop  (double val, unsigned short modID, unsigned short ch){ SetChannelSetting("TRIGGER_FLATTOP",   val, modID, ch, 1);}
  void SetChannelTriggerThreshold(double val, unsigned short modID, unsigned short ch){ SetChannelSetting("TRIGGER_THRESHOLD", val, modID, ch, 1);}
  void SetChannelEnergyRiseTime  (double val, unsigned short modID, unsigned short ch){ SetChannelSetting("ENERGY_RISETIME",   val, modID, ch, 1);}
  void SetChannelEnergyFlatTop   (double val, unsigned short modID, unsigned short ch){ SetChannelSetting("ENERGY_FLATTOP",    val, modID, ch, 1);}
  void SetChannelEnergyTau       (double val, unsigned short modID, unsigned short ch){ SetChannelSetting("TAU",               val, modID, ch, 1);}
  void SetChannelTraceLenght     (double val, unsigned short modID, unsigned short ch){ SetChannelSetting("TRACE_LENGTH",      val, modID, ch, 1);}
  void SetChannelTraceDelay      (double val, unsigned short modID, unsigned short ch){ SetChannelSetting("TRACE_DELAY",       val, modID, ch, 1);}
  void SetChannelBaseLinePrecent (double val, unsigned short modID, unsigned short ch){ SetChannelSetting("BASELINE_PERCENT",  val, modID, ch, 1);}
  void SetChannelVOffset         (double val, unsigned short modID, unsigned short ch){ SetChannelSetting("VOFFSET",           val, modID, ch, 1);}
  
  void SwitchCSRA(int bitwise, unsigned short modID, unsigned short ch);
  void SetCSRABit(int bitwise, unsigned short val, unsigned short modID, unsigned short ch);
  
  void SetChannelOnOff(bool enable, unsigned short modID, unsigned short ch)               { SetCSRABit(CSRA_BIT::ENABLE_CHANNEL, enable, modID, ch); }
  void SetChannelPositivePolarity(bool positive, unsigned short modID, unsigned short ch)  { SetCSRABit(CSRA_BIT::POLARITY, positive, modID, ch); }
  void SetChannelTraceOnOff(bool enable, unsigned short modID, unsigned short ch)          { SetCSRABit(CSRA_BIT::ENABLE_TRACE, enable, modID, ch); }
  void SetChannelGain(bool high, unsigned short modID, unsigned short ch)                  { SetCSRABit(CSRA_BIT::INPUT_RELAY, high, modID, ch); }
  
  void SaveSettings(std::string fileName);
  
  ///========================== RUN
  
  void StartRun(bool listMode);
  void StopRun();
  
  unsigned int * GetStatitics(unsigned short modID);
  double GetInputCountRate(unsigned short modID, unsigned short ch);
  double GetLiveTime(unsigned short modID, unsigned short ch);
  double GetOutputCountRate(unsigned short modID, unsigned short ch);
  double GetRealTime(unsigned short modID);
  void   PrintStatistics(unsigned short modID);

  
  void ReadData(unsigned short modID);
  
  unsigned int GetnFIFOWords() {return nFIFOWords;}
  unsigned int GetNextWord()   {return nextWord;}
  DataBlock *  GetData()       {return data;}
  bool ProcessSingleData();
  void PrintExtFIFOWords() {
    unsigned int nWords = (ExtFIFO_Data[nextWord] >> 17) & 0x3FFF;
    printf("------------------- print dataBlock, nWords = %d\n", nWords);
    int count = 0;
    for( unsigned int i = nextWord; i < nextWord + nWords + 10 ; i++){
      if( i == nextWord + nWords ) printf("===== end of dataBlock\n");
      if( count % 4 == 3 ){
        printf("%08X \n", ExtFIFO_Data[i]);
      }else{
        printf("%08X ", ExtFIFO_Data[i]);
      }
      count ++;
    }
    printf("\n------------------- \n");
  }
  
  void OpenFile(std::string fileName, bool append);
  void SaveData();
  void CloseFile();
  
  
};

#endif
