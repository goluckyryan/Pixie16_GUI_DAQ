#ifndef CHANNEL_SETTING_H
#define CHANNEL_SETTING_H

#include <TQObject.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>

#include "Pixie16Class.h"

class TGWindow;
class TGMainFrame;

#define MAXCH 16
#define NUM_CHANNEL_SETTING 25

class ChannelSetting{
private:
  TGMainFrame * fMain;
  
  TGNumberEntry * modIDEntry, * chIDEntry;
  
  TGTextEntry * entry[NUM_CHANNEL_SETTING];
  
  Pixie16 * pixie;

public:
  ChannelSetting(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie);
  virtual ~ChannelSetting();

  void CloseWindow() { printf("close ChannelSetting window\n"); delete this; }

  void ChangeMod();
  void ChangeCh();

  bool isOpened;
  
};


#endif
