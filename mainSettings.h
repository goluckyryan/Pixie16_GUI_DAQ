#ifndef MAIN_SETTING_H
#define MAIN_SETTING_H

#include <TQObject.h>

#include "Pixie16Class.h"

class TGWindow;
class TGMainFrame;

class MainSettings{
private:
  TGMainFrame * fMain;
  
  Pixie16 * pixie;

public:
  MainSettings(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie);
  virtual ~MainSettings();
  
  
  void CloseWindow() { printf("close window\n"); delete this; }
  
};


#endif
