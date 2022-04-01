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

TString modSettingName[NUM_MOD_SETTING] = { "CrateID",             
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
                                         "MODULE_CSRB"};


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

  int comboBoxWidth = 135;
  int comboBoxHeigth = 30;
  
  TGHorizontalFrame *hframe[NUM_MOD_SETTING];
  TGLabel * lb[NUM_MOD_SETTING];
  
  for( int i = 0 ; i < NUM_MOD_SETTING; i++){
    hframe[i] = new TGHorizontalFrame(vframe, 50, 50 );
    vframe->AddFrame(hframe[i], new TGLayoutHints(kLHintsRight, 2,2,2,2));
  
    int temp = pixie->GetDigitizerSetting(modSettingName[i].Data(), modID, false);
    
    if( i < 2 || i > 4){    
      entry[i] = new TGTextEntry(hframe[i], new TGTextBuffer(1));
      entry[i]->SetEnabled(false);
      entry[i]->SetText( Form("%d", temp) );
      
      if( i == 5 )  entry[i]->SetText( Form("%d (only 0 supported)", temp) );
      
      hframe[i]->AddFrame(entry[i], new TGLayoutHints(kLHintsRight, 5,5,3,4));
    }
    
    if( i == 2 ){
      cbSyncWait = new TGComboBox(hframe[i]);
      cbSyncWait->SetWidth(comboBoxWidth);
      cbSyncWait->SetHeight(comboBoxHeigth);
      
      cbSyncWait->AddEntry("Sync", 1);
      cbSyncWait->AddEntry("not Sync", 0);
      cbSyncWait->Select(temp);
      cbSyncWait->Connect("Selected(Int_t, Int_t)", "ModuleSetting", this, "ChangeSyncWait()"); 
      
      hframe[i]->AddFrame(cbSyncWait, new TGLayoutHints(kLHintsRight, 5,5,3,4));
    }

    if( i == 3 ){
      cbInSych = new TGComboBox(hframe[i]);
      cbInSych->SetWidth(comboBoxWidth);
      cbInSych->SetHeight(comboBoxHeigth);
      
      cbInSych->AddEntry("Clock clear", 0);
      cbInSych->AddEntry("Common Clock", 1);
      cbInSych->Select(temp);
      cbInSych->Connect("Selected(Int_t, Int_t)", "ModuleSetting", this, "ChangeInSync()"); 
      
      hframe[i]->AddFrame(cbInSych, new TGLayoutHints(kLHintsRight, 5,5,3,4));
    }

    if( i == 4 ){
      
      cbSlowFilterRange = new TGComboBox(hframe[i]);
      cbSlowFilterRange->SetWidth(comboBoxWidth);
      cbSlowFilterRange->SetHeight(comboBoxHeigth);
      
      unsigned short sampleRate = pixie->GetDigitizerSamplingRate(modID);
      if( sampleRate == 100 || sampleRate == 500 ){ 
        cbSlowFilterRange->AddEntry("off", 0);
        cbSlowFilterRange->AddEntry("0.02 us", 1);
        cbSlowFilterRange->AddEntry("0.04 us", 2);
        cbSlowFilterRange->AddEntry("0.08 us", 3);
        cbSlowFilterRange->AddEntry("0.16 us", 4);
        cbSlowFilterRange->AddEntry("0.32 us", 5);
        cbSlowFilterRange->AddEntry("0.64 us", 6);
      }else{
        cbSlowFilterRange->AddEntry("off", 0);
        cbSlowFilterRange->AddEntry("0.016 us", 1);
        cbSlowFilterRange->AddEntry("0.032 us", 2);
        cbSlowFilterRange->AddEntry("0.064 us", 3);
        cbSlowFilterRange->AddEntry("0.128 us", 4);
        cbSlowFilterRange->AddEntry("0.256 us", 5);
        cbSlowFilterRange->AddEntry("0.512 us", 6);        
      }
      cbSlowFilterRange->Select(temp);
      cbSlowFilterRange->Connect("Selected(Int_t, Int_t)", "ModuleSetting", this, "ChangeSlowFilterRange()"); 
      
      hframe[i]->AddFrame(cbSlowFilterRange, new TGLayoutHints(kLHintsRight, 5,5,3,4));
    }
  
    lb[i] = new TGLabel(hframe[i], modSettingName[i]);
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
    if( i < 2 || i > 5 ) delete entry[i];
  }
  
  delete cbSyncWait;
  delete cbInSych;
  delete cbSlowFilterRange;
 
  /// fMain must be delete last;
  fMain->Cleanup();
  delete fMain;
  
}


void ModuleSetting::ChangeMod(){
  int modID = modIDEntry->GetNumber();
  
  for( int i = 0 ; i < NUM_MOD_SETTING; i++){  
    int temp = pixie->GetDigitizerSetting(modSettingName[i].Data(), modID, false);
    
    if( i < 2 || i > 4){
      entry[i]->SetText( Form("%d", temp) );
    }
    
    if( i == 2 ) cbSyncWait->Select(temp);
    if( i == 3 ) cbInSych->Select(temp);
    
    if( i == 4 ) {
      
      cbSlowFilterRange->RemoveAll();
      unsigned short sampleRate = pixie->GetDigitizerSamplingRate(modID);
      if( sampleRate == 100 || sampleRate == 500 ){ 
        cbSlowFilterRange->AddEntry("off", 0);
        cbSlowFilterRange->AddEntry("0.02 us", 1);
        cbSlowFilterRange->AddEntry("0.04 us", 2);
        cbSlowFilterRange->AddEntry("0.08 us", 3);
        cbSlowFilterRange->AddEntry("0.16 us", 4);
        cbSlowFilterRange->AddEntry("0.32 us", 5);
        cbSlowFilterRange->AddEntry("0.64 us", 6);
      }else{
        cbSlowFilterRange->AddEntry("off", 0);
        cbSlowFilterRange->AddEntry("0.016 us", 1);
        cbSlowFilterRange->AddEntry("0.032 us", 2);
        cbSlowFilterRange->AddEntry("0.064 us", 3);
        cbSlowFilterRange->AddEntry("0.128 us", 4);
        cbSlowFilterRange->AddEntry("0.256 us", 5);
        cbSlowFilterRange->AddEntry("0.512 us", 6);    
      }
      cbSlowFilterRange->Select(temp);
      
    }
  }  
}

void ModuleSetting::ChangeSyncWait(){
  pixie->SetDigitizerSynchWait(cbSyncWait->GetSelected(), modIDEntry->GetNumber());
  
  ///printf("%s \n", pixie->GetSettingFile(modIDEntry->GetNumber()).c_str());
  pixie->SaveSettings(pixie->GetSettingFile(modIDEntry->GetNumber()));
}

void ModuleSetting::ChangeInSync(){
  pixie->SetDigitizerInSynch(cbInSych->GetSelected(), modIDEntry->GetNumber());  
  pixie->SaveSettings(pixie->GetSettingFile(modIDEntry->GetNumber()));
}

void ModuleSetting::ChangeSlowFilterRange(){
  pixie->SetDigitizerSlowFilterRange(cbInSych->GetSelected(), modIDEntry->GetNumber());  
  pixie->SaveSettings(pixie->GetSettingFile(modIDEntry->GetNumber()));
}

