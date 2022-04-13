#ifndef SETTINGS_SUMMARY_H
#define SETTINGS_SUMMARY_H

#include <TQObject.h>

#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>

#include "Pixie16Class.h"
#include "global_macro.h"

class TGWindow;
class TGMainFrame;

class SettingsSummary{
private:
  TGMainFrame * fMain;
  
  TGNumberEntry * modIDEntry;
  
  TGTextEntry * teFileName;

  TString settingFileName;

  TGComboBox * cbOnOff[MAXCH] ;
  TGComboBox * cbGain[MAXCH] ;
  TGComboBox * cbPol[MAXCH] ;
  TGComboBox * cbTraceOnOff[MAXCH] ;

  TGLabel * lbCh[MAXCH];
  TGNumberEntry * neTrigL[MAXCH] ; 
  TGNumberEntry * neTrigG[MAXCH] ; 
  TGNumberEntry * neThreshold[MAXCH] ;
  TGNumberEntry * neEngL[MAXCH]  ;
  TGNumberEntry * neEngG[MAXCH]  ;
  TGNumberEntry * neTau[MAXCH]   ;
  TGNumberEntry * neTraceLength[MAXCH] ;
  TGNumberEntry * neTraceDelay[MAXCH]  ;
  TGNumberEntry * neVoff[MAXCH]  ;  
  TGNumberEntry * neBL[MAXCH];
  
  Pixie16 * pixie;
  
  Pixel_t red;
  Pixel_t black;

  int maxCh;
  


public:
  SettingsSummary(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie);
  virtual ~SettingsSummary();

  
  void CloseWindow() { printf("close SettingsSummary window\n"); delete this; }
  
  void ChangeOnOff(unsigned short ch);
  void ChangeGain(unsigned short ch);
  void ChangePol(unsigned short ch);
  void ChangeTraceOnOff(unsigned short ch);
  void ChangeTrigL(unsigned short ch);
  void ChangeTrigG(unsigned short ch);
  void ChangeThreshold(unsigned short ch);
  void ChangeEngL(unsigned short ch);
  void ChangeEngG(unsigned short ch);
  void ChangeTau(unsigned short ch);
  void ChangeTraceLenght(unsigned short ch);
  void ChangeTraceDelay(unsigned short ch);
  void ChangeVoff(unsigned short ch);
  void ChangeBL(unsigned short ch);
  
  void ChangeMod();
  void OpenFile();
  void SaveSetting();
  
  bool isOpened;
  
};


#endif
