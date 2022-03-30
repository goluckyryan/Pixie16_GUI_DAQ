#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGTableContainer.h>
#include <TGFileDialog.h>

#include "moduleSetting.h"


ModuleSetting::ModuleSetting(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie){
  
  this->pixie = pixie; 
  
  fMain = new TGMainFrame(p,w,h);
  fMain->SetWindowName("Pixie16 Channel Settings ");
  fMain->Connect("CloseWindow()", "ModuleSetting", this, "CloseWindow()");
  
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
  modIDEntry->Connect("Modified()", "ModuleSetting", this, "ChangeMod()"); 
  hframe0->AddFrame(modIDEntry, new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
  
  int modID = modIDEntry->GetNumber();
  
  TString settingName[NUM_MOD_SETTING] = {
                              "CrateID",             
                              "SlotID",              
                              "SYNCH_WAIT",          
                              "IN_SYNCH",            
                              "SLOW_FILTER_RANGE",   
                              "FAST_FILTER_RANGE",   
                              "FastTrigBackplaneEna",
                              "TrigConfig0",         
                              "TrigConfig1",         
                              "TrigConfig2",         
                              "TrigConfig3",         
                              "HOST_RT_PRESET",      
                              "MODULE_CSRB",         
                            };
  
  TGHorizontalFrame *hframe[NUM_MOD_SETTING];
  TGLabel * lb[NUM_MOD_SETTING];
  
  for( int i = 0 ; i < NUM_MOD_SETTING; i++){
    hframe[i] = new TGHorizontalFrame(vframe, 50, 50 );
    vframe->AddFrame(hframe[i], new TGLayoutHints(kLHintsRight, 2,2,2,2));
  
    int temp = pixie->GetDigitizerSetting(settingName[i].Data(), modID, false);
    entry[i] = new TGTextEntry(hframe[i], new TGTextBuffer(1));
    //if( i < 2 || i == NUM_MOD_SETTING -1 ) entry[i]->SetEnabled(false);
    entry[i]->SetEnabled(false);
    
    entry[i]->SetText( Form("%d", temp) );

    hframe[i]->AddFrame(entry[i], new TGLayoutHints(kLHintsRight, 5,5,3,4));
  
    lb[i] = new TGLabel(hframe[i], settingName[i]);
    hframe[i]->AddFrame(lb[i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 3, 4));
  
  }
  
  
  fMain->MapSubwindows();
  fMain->Resize(fMain->GetDefaultSize());
  fMain->MapWindow();
  
  isOpened = true;
}


ModuleSetting::~ModuleSetting(){
  
  isOpened = false;
  
  delete modIDEntry;
 
  for ( int i = 0; i < NUM_MOD_SETTING; i++){
    delete entry[i];
  }
 
  
  /// fMain must be delete last;
  fMain->Cleanup();
  delete fMain;
  
}


void ModuleSetting::ChangeMod(){
  
  
  
}
