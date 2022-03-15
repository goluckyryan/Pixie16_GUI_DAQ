#ifndef MAIN_SETTING_H
#define MAIN_SETTING_H

#include <TQObject.h>

class TGWindow;
class TGMainFrame;

class MainSettings{
private:
  TGMainFrame * fMain;

public:
  MainSettings(const TGWindow *p, UInt_t w, UInt_t h);
  virtual ~MainSettings();
  
};


#endif
