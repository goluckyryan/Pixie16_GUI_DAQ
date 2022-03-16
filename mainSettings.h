#ifndef MAIN_SETTING_H
#define MAIN_SETTING_H

#include <TQObject.h>
///#include <RQ_OBJECT.h>

#include <TGNumberEntry.h>
#include <TGComboBox.h>

#include "Pixie16Class.h"

class TGWindow;
class TGMainFrame;

#define MAXCH 16

class MainSettings{
///   RQ_OBJECT("MainSettings")
private:
  TGMainFrame * fMain;
  
  TGNumberEntry * modIDEntry;
  
  TGTextEntry * teFileName;

  TString settingFileName;

  TGComboBox * cbOnOff[MAXCH] ;
  TGComboBox * cbGain[MAXCH] ;
  TGComboBox * cbPol[MAXCH] ;

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
  
  int maxCh;

public:
  MainSettings(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie);
  virtual ~MainSettings();

  
  void CloseWindow() { printf("close window\n"); delete this; }
  
  void ChangeOnOff(unsigned short ch);
  void ChangeGain(unsigned short ch);
  void ChangePol(unsigned short ch);
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
  
};


#endif
