#ifndef MODULE_SETTING_H
#define MODULE_SETTING_H

#include <TQObject.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>

#include "Pixie16Class.h"

class TGWindow;
class TGMainFrame;

#define MAXCH 16
#define NUM_MOD_SETTING 13

class ModuleSetting{
private:
  TGMainFrame * fMain;
  
  TGNumberEntry * modIDEntry;
  
  TGTextEntry * entry[NUM_MOD_SETTING];
  
  Pixie16 * pixie;

public:
  ModuleSetting(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie);
  virtual ~ModuleSetting();

  void CloseWindow() { printf("close window\n"); delete this; }

  void ChangeMod();

  bool isOpened;
  
};


#endif
