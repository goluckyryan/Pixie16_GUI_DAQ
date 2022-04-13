#ifndef MODULE_SETTING_H
#define MODULE_SETTING_H

#include <TQObject.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>

#include "Pixie16Class.h"
#include "global_macro.h"

class TGWindow;
class TGMainFrame;

class ModuleSetting{
private:
  TGMainFrame * fMain;
  
  TGNumberEntry * modIDEntry;
  
  TGTextEntry * entry[NUM_MOD_SETTING];
  
  TGComboBox * cbSyncWait;
  TGComboBox * cbInSych;
  
  TGComboBox * cbSlowFilterRange;
  TGComboBox * cbFastFilterRange;
  
  
  Pixie16 * pixie;

public:
  ModuleSetting(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie);
  virtual ~ModuleSetting();

  void CloseWindow() { printf("close ModuleSetting window\n"); delete this; }

  void ChangeMod();
  
  void ChangeSyncWait();
  void ChangeInSync();
  void ChangeSlowFilterRange();

  bool isOpened;
  
};


#endif
