#ifndef SCALAR_PANEL_H
#define SCALAR_PANEL_H

#include <TQObject.h>
#include <TThread.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>

#include "Pixie16Class.h"
#include "global_macro.h"

class TGWindow;
class TGMainFrame;

class ScalarPanel{
private:
  TGTransientFrame * fMain;
  
  static int nMod;
  static Pixie16 * pixie;

  static bool updateFlag;
  
  static TGTextEntry * teRate[MAXMOD][MAXCH];  
  static TGTextEntry * teRealTime[MAXMOD];  
  
  TThread * thread;

public:
  ScalarPanel(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, Pixie16 * pixie);
  virtual ~ScalarPanel();

  static void * UpdateScalar(void * ptr); 
  
  void CloseWindow() { printf("close window\n"); delete this; }
  
  bool isOpened;
    

  
  
};


#endif
