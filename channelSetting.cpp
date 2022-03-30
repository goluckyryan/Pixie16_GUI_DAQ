#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGTableContainer.h>
#include <TGFileDialog.h>

#include "channelSetting.h"


ChannelSetting::ChannelSetting(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie){
  
  this->pixie = pixie; 
  
  fMain = new TGMainFrame(p,w,h);
  fMain->SetWindowName("Pixie16 Channel Settings ");
  fMain->Connect("CloseWindow()", "ChannelSetting", this, "CloseWindow()");
  
  TGVerticalFrame * vframe = new TGVerticalFrame(fMain);
  fMain->AddFrame(vframe, new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
  
  
  ///==========Module choose
  TGHorizontalFrame *hframe0 = new TGHorizontalFrame(vframe, w, 50 );
  vframe->AddFrame(hframe0, new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
  
  TGLabel * lb0 = new TGLabel(hframe0, "Module ID :");
  hframe0->AddFrame(lb0, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

  modIDEntry = new TGNumberEntry(hframe0, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  modIDEntry->SetWidth(50);
  modIDEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, pixie->GetNumModule()-1);
  modIDEntry->Connect("Modified()", "ChannelSetting", this, "ChangeMod()"); 
  hframe0->AddFrame(modIDEntry, new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));

  TGLabel * lb1 = new TGLabel(hframe0, "Channel :");
  hframe0->AddFrame(lb1, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

  chIDEntry = new TGNumberEntry(hframe0, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  chIDEntry->SetWidth(50);
  chIDEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, MAXCH);
  chIDEntry->Connect("Modified()", "ChannelSetting", this, "ChangeCh()"); 
  hframe0->AddFrame(chIDEntry, new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
  
  int modID = modIDEntry->GetNumber();
  int ch = chIDEntry->GetNumber();
  
  TString settingName[NUM_CHANNEL_SETTING] = {
                                               "TRIGGER_RISETIME",   
                                               "TRIGGER_FLATTOP",    
                                               "TRIGGER_THRESHOLD",  
                                               "ENERGY_RISETIME",    
                                               "ENERGY_FLATTOP",     
                                               "TAU",                
                                               "TRACE_LENGTH",       
                                               "TRACE_DELAY",        
                                               "VOFFSET",            
                                               "XDT",                
                                               "BASELINE_PERCENT",   
                                               "BASELINE_AVERAGE",   
                                               "BLCUT",              
                                               "EMIN",               
                                               "QDCLen0",            
                                               "QDCLen1",            
                                               "QDCLen2",            
                                               "QDCLen3",            
                                               "QDCLen4",            
                                               "QDCLen5",            
                                               "QDCLen6",            
                                               "QDCLen7",            
                                               "MultiplicityMaskL",  
                                               "MultiplicityMaskH",  
                                               "CHANNEL_CSRA",             
                            };
  
  TGHorizontalFrame *hframe[NUM_CHANNEL_SETTING];
  TGLabel * lb[NUM_CHANNEL_SETTING];
  
  for( int i = 0 ; i < NUM_CHANNEL_SETTING; i++){
    hframe[i] = new TGHorizontalFrame(vframe, 50, 50 );
    vframe->AddFrame(hframe[i], new TGLayoutHints(kLHintsRight, 2,2,2,2));
  
    double temp = pixie->GetChannelSetting(settingName[i].Data(), modID, ch, false);
    entry[i] = new TGTextEntry(hframe[i], new TGTextBuffer(1));
    //if( i < 2 || i == NUM_CHANNEL_SETTING -1 ) entry[i]->SetEnabled(false);
    entry[i]->SetEnabled(false);
    
    entry[i]->SetText( Form("%f", temp) );

    hframe[i]->AddFrame(entry[i], new TGLayoutHints(kLHintsRight, 5,5,3,4));
  
    lb[i] = new TGLabel(hframe[i], settingName[i]);
    hframe[i]->AddFrame(lb[i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 3, 4));
  
  }
  
  
  fMain->MapSubwindows();
  fMain->Resize(fMain->GetDefaultSize());
  fMain->MapWindow();
  
  isOpened = true;
}


ChannelSetting::~ChannelSetting(){
  
  isOpened = false;
  
  delete modIDEntry;
 
  for ( int i = 0; i < NUM_CHANNEL_SETTING; i++){
    delete entry[i];
  }
 
  
  /// fMain must be delete last;
  fMain->Cleanup();
  delete fMain;
  
}


void ChannelSetting::ChangeMod(){
  
  
  
}

void ChannelSetting::ChangeCh(){
  
  
  
}
