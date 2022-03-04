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
  unsigned short   NumModules;
  unsigned short * PXISlotMap;
  unsigned short   OfflineMode;

  char ** ComFPGAConfigFile;
  char ** SPFPGAConfigFile;
  char ** TrigFPGAConfigFile;
  char ** DSPCodeFile;
  char ** DSPParFile;
  char ** DSPVarFile;
  
  unsigned short BootPattern ;
  
  int retval;
  
  bool isRunning;
  
  int CheckError(std::string operation);
  
  unsigned int nFIFOWords;
  unsigned int * ExtFIFO_Data;
  unsigned int * Statistics;
  
  DataBlock * data;
  unsigned int nextWord;
  
  std::ofstream outFile;

  /***
  struct channelSetting{
    double trigger_risetime;
    double trigger_flattop;
    double trigger_threshold;
    double energy_risetime;
    double energy_flattop;
    double tau;
    double trace_length;
    double trace_delay;
    double V_offset;
    double XDT;
    double baseline_percent;
    double energy_minumum; //EMIN
    double baseline_cut;   //BLCUT
  };**/
  
public:

  Pixie16();
  ~Pixie16();

  int GetStatus() {return retval;}
  bool IsRunning() {return isRunning;}

  ///======================== startup
  bool LoadConfigFile(std::string fileName);

  void CheckDriver();
  void CheckHardware();
  
  void GetDigitizerInfo(unsigned short modID);
  void BootDigitizers();
    
  void AdjustOffset();
  
  ///========================= Setting

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
  
  void PrintChannelAllSettings(unsigned short modID, unsigned short ch);
  void PrintChannelsMainSettings(unsigned short modID);
  
  unsigned short GetCSRA(int bitwise,  unsigned short modID, unsigned short ch, bool verbose = false);
  bool GetChannleOnOff(unsigned short modID, unsigned short ch, bool verbose = false)        {return GetCSRA(CSRA_BIT::ENABLE_CHANNEL, modID, ch, verbose);}
  bool GetChannelPolarity(unsigned short modID, unsigned short ch, bool verbose = false)     {return GetCSRA(CSRA_BIT::POLARITY, modID, ch, verbose);}
  bool GetChannelTraceOnOff(unsigned short modID, unsigned short ch, bool verbose = false)   {return GetCSRA(CSRA_BIT::ENABLE_TRACE, modID, ch, verbose);}

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
  
  void SetChannelOnOff(bool enable, unsigned short modID, unsigned short ch)        { SetCSRABit(CSRA_BIT::ENABLE_CHANNEL, enable, modID, ch); }
  void SetChannelPositivePolarity(bool positive, unsigned short modID, unsigned short ch)  { SetCSRABit(CSRA_BIT::POLARITY, positive, modID, ch); }
  void SetChannelTraceOnOff(bool enable, unsigned short modID, unsigned short ch)          { SetCSRABit(CSRA_BIT::ENABLE_TRACE, enable, modID, ch); }
  
  void SaveSettings(std::string fileName);
  
  ///========================== RUN
  
  void StartRun(bool listMode);
  void StopRun();
  
  unsigned int * GetStatitics(unsigned short modID);
  double GetInputCountRate(unsigned short modID, unsigned short ch);
  double GetLiveTime(unsigned short modID, unsigned short ch);
  double GetOutputCountRate(unsigned short modID, unsigned short ch);
  double GetRealTime(unsigned short modID);
  void PrintStatistics(unsigned short modID);
  
  void GetTrace(unsigned short modID, unsigned short ch);
  void GetBaseLines(unsigned short modID, unsigned short ch);
  
  void ReadData(unsigned short modID);
  
  void PrintExtFIFOData(int a) { printf("%5d-%5d | %08X  %08X  %08X  %08X \n", a, a+3, ExtFIFO_Data[a], ExtFIFO_Data[a+1], ExtFIFO_Data[a+2], ExtFIFO_Data[a+3]);} 
  unsigned int GetnFIFOWords() {return nFIFOWords;}
  unsigned int GetNextWord()   {return nextWord;}
  DataBlock * GetData()        {return data;}
  void ProcessSingleData();
  void ProcessData(int verbose = 0);
  
  void OpenFile(std::string fileName, bool append);
  void SaveData();
  void CloseFile();
  
  
  //void SaveData(char * fileName, unsigned short isEndOfRun);
  
  
};

#endif
