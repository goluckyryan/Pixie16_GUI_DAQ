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

//TODO set MAX

///---------------------------------------------- NAME,   DIGI,   unit ... MAX
TString settingName[NUM_CHANNEL_SETTING][3] = {{"TRIGGER_RISETIME",   "2",  "us"},
                                               {"TRIGGER_FLATTOP",    "2",  "us"},
                                               {"TRIGGER_THRESHOLD",  "0",  "ADC"},
                                               {"ENERGY_RISETIME",    "1",  "us"},
                                               {"ENERGY_FLATTOP",     "1",  "us"},
                                               {"TAU",                "2",  "us"},
                                               {"TRACE_LENGTH",       "2",  "us"},
                                               {"TRACE_DELAY",        "2",  "us"},
                                               {"VOFFSET",            "2",  "V"},
                                               {"XDT",                "2",  "us"},
                                               {"BASELINE_PERCENT",   "0",  "%"},
                                               {"BASELINE_AVERAGE",   "0",   ""},
                                               {"BLCUT",              "0",   ""},
                                               {"EMIN",               "0",   ""},
                                               {"QDCLen0",            "2",  "us"},
                                               {"QDCLen1",            "2",  "us"},
                                               {"QDCLen2",            "2",  "us"},
                                               {"QDCLen3",            "2",  "us"},
                                               {"QDCLen4",            "2",  "us"},
                                               {"QDCLen5",            "2",  "us"},
                                               {"QDCLen6",            "2",  "us"},
                                               {"QDCLen7",            "2",  "us"},
                                               {"MultiplicityMaskL",  "0",  ""},
                                               {"MultiplicityMaskH",  "0",  ""},
                                               {"CHANNEL_CSRA",       "0",  ""}};
                                               
                                               ///==== missing parameters
                                               ///"PEAK_SAMPLE"
                                               ///"PEAK_SEP"
                                               ///"BINFACTOR"
                                               ///"FASTTRIGBACKLEN"
                                               ///"INTEGRATOR"
                                               ///"CFDDelay"
                                               ///"CFDScale"
                                               ///"CFDThresh"
                                               ///"ExtTrigStretch"
                                               ///"VetoStretch"
                                               ///"ExternDelayLen"
                                               ///"FtrigoutDelay"
                                               ///"ChanTrigStretch"


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
  if( pixie->GetNumModule() == 1 ) modIDEntry->SetState(false);
  hframe0->AddFrame(modIDEntry, new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));

  TGLabel * lb1 = new TGLabel(hframe0, "Channel :");
  hframe0->AddFrame(lb1, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

  chIDEntry = new TGNumberEntry(hframe0, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  chIDEntry->SetWidth(50);
  chIDEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, MAXCH-1);
  chIDEntry->Connect("Modified()", "ChannelSetting", this, "ChangeCh()"); 
  hframe0->AddFrame(chIDEntry, new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
  
  int modID = modIDEntry->GetNumber();
  int ch = chIDEntry->GetNumber();

  int width = 80;
  
  ///----------- on/off
  TGHorizontalFrame *hframeOnOff = new TGHorizontalFrame(vframe, 50, 50 );
  vframe->AddFrame(hframeOnOff, new TGLayoutHints(kLHintsRight, 2,2,2,2));
  
  cbOnOff = new TGComboBox(hframeOnOff);
  cbOnOff->AddEntry("ON", 1);
  cbOnOff->AddEntry("off", 0);
  cbOnOff->Resize(width, 20);
  pixie->GetChannelOnOff(modID, ch) ? cbOnOff->Select(1) : cbOnOff->Select(0);
  cbOnOff->Connect("Selected(Int_t, Int_t)", "ChannelSetting", this, "ChangeOnOff()");  
  hframeOnOff->AddFrame(cbOnOff,  new TGLayoutHints(kLHintsRight, 5,5,3,4));

  TGLabel * lbOnOff = new TGLabel(hframeOnOff, "On/Off");
  hframeOnOff->AddFrame(lbOnOff, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 3, 4));
  
  ///----------- Polarity
  TGHorizontalFrame *hframePol = new TGHorizontalFrame(vframe, 50, 50 );
  vframe->AddFrame(hframePol, new TGLayoutHints(kLHintsRight, 2,2,2,2));
  
  cbPolarity = new TGComboBox(hframePol);
  cbPolarity->AddEntry("Positive +", 1);
  cbPolarity->AddEntry("Negative -", 0);
  cbPolarity->Resize(width, 20);
  pixie->GetChannelPolarity(modID, ch) ? cbPolarity->Select(1) : cbPolarity->Select(0);
  cbPolarity->Connect("Selected(Int_t, Int_t)", "ChannelSetting", this, "ChangeOnOff()");  
  hframePol->AddFrame(cbPolarity,  new TGLayoutHints(kLHintsRight, 5,5,3,4));

  TGLabel * lbPol = new TGLabel(hframePol, "Polarity");
  hframePol->AddFrame(lbPol, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 3, 4));
  


  ///----- all other
  TGHorizontalFrame *hframe[NUM_CHANNEL_SETTING];
  TGLabel * lb[NUM_CHANNEL_SETTING];
  
  for( int i = 0 ; i < NUM_CHANNEL_SETTING; i++){
    hframe[i] = new TGHorizontalFrame(vframe, 50, 50 );
    vframe->AddFrame(hframe[i], new TGLayoutHints(kLHintsRight, 2,2,2,2));
  
    ///----------- Trace on/off
    if(settingName[i][0] == "TAU"){ 
      TGHorizontalFrame *hframeTraceOnOff = new TGHorizontalFrame(vframe, 50, 50 );
      vframe->AddFrame(hframeTraceOnOff, new TGLayoutHints(kLHintsRight, 2,2,2,2));
      
      cbTraceOnOff = new TGComboBox(hframeTraceOnOff);
      cbTraceOnOff->AddEntry("On", 1);
      cbTraceOnOff->AddEntry("Off", 0);
      cbTraceOnOff->Resize(width, 20);
      pixie->GetChannelTraceOnOff(modID, ch) ? cbTraceOnOff->Select(1) : cbTraceOnOff->Select(0);
      cbTraceOnOff->Connect("Selected(Int_t, Int_t)", "ChannelSetting", this, "ChangeOnOff()");  
      hframeTraceOnOff->AddFrame(cbTraceOnOff,  new TGLayoutHints(kLHintsRight, 5,5,3,4));

      TGLabel * lbTraceOnOff = new TGLabel(hframeTraceOnOff, "Trace On/Off");
      hframeTraceOnOff->AddFrame(lbTraceOnOff, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 3, 4));
    }
    ///----------- QDC sum on/off
    if( settingName[i][0] == "EMIN"){ 
      TGHorizontalFrame *hframeQDCsumOnOff = new TGHorizontalFrame(vframe, 50, 50 );
      vframe->AddFrame(hframeQDCsumOnOff, new TGLayoutHints(kLHintsRight, 2,2,2,2));
      
      cbQDCsumOnOff = new TGComboBox(hframeQDCsumOnOff);
      cbQDCsumOnOff->AddEntry("On", 1);
      cbQDCsumOnOff->AddEntry("Off", 0);
      cbQDCsumOnOff->Resize(width, 20);
      pixie->GetChannelQDCsumOnOff(modID, ch) ? cbQDCsumOnOff->Select(1) : cbQDCsumOnOff->Select(0);
      cbQDCsumOnOff->Connect("Selected(Int_t, Int_t)", "ChannelSetting", this, "ChangeQDCsumOnOff()");  
      hframeQDCsumOnOff->AddFrame(cbQDCsumOnOff,  new TGLayoutHints(kLHintsRight, 5,5,3,4));

      TGLabel * lbQDCsumOnOff = new TGLabel(hframeQDCsumOnOff, "QDC sum On/Off");
      hframeQDCsumOnOff->AddFrame(lbQDCsumOnOff, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 3, 4));
    }
    
    double temp = pixie->GetChannelSetting(settingName[i][0].Data(), modID, ch, false);
    
    TGNumberFormat::EStyle digi = TGNumberFormat::kNESInteger;
    if( settingName[i][1] == "0" ) digi = TGNumberFormat::kNESInteger;
    if( settingName[i][1] == "1" ) digi = TGNumberFormat::kNESRealOne;
    if( settingName[i][1] == "2" ) digi = TGNumberFormat::kNESRealTwo;
    if( settingName[i][1] == "3" ) digi = TGNumberFormat::kNESRealThree;
    
    entry[i] = new TGNumberEntry(hframe[i], temp, 0, 0, digi, TGNumberFormat::kNEANonNegative);
    entry[i]->Resize(width, 20);
    
    if( i >= NUM_CHANNEL_SETTING - 3 ) {
      entry[i]->SetState(false);
    }else{
      entry[i]->Connect("Modified()", "ChannelSetting", this, Form("ChangeID(=%d)", i));  
    }
    hframe[i]->AddFrame(entry[i], new TGLayoutHints(kLHintsRight, 5,5,3,4));
  
    lb[i] = new TGLabel(hframe[i], settingName[i][0] +  (settingName[i][2] != "" ? " [" + settingName[i][2] + "]" : ""));
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
  
  delete cbOnOff;
  delete cbTraceOnOff;
  delete cbPolarity;
  delete cbQDCsumOnOff;
  
  for ( int i = 0; i < NUM_CHANNEL_SETTING; i++){
    delete entry[i];
  }
 
  /// fMain must be delete last;
  fMain->Cleanup();
  delete fMain;
  
}


void ChannelSetting::ChangeMod(){
  ChangeCh();
}

void ChannelSetting::ChangeCh(){
  
  int modID = modIDEntry->GetNumber();
  int ch = chIDEntry->GetNumber();
  
  pixie->GetChannelOnOff(modID, ch) ? cbOnOff->Select(1) : cbOnOff->Select(0);
  pixie->GetChannelPolarity(modID, ch) ? cbPolarity->Select(1) : cbPolarity->Select(0);
  pixie->GetChannelTraceOnOff(modID, ch) ? cbTraceOnOff->Select(1) : cbTraceOnOff->Select(0);
  
  for( int i = 0 ; i < NUM_CHANNEL_SETTING; i++){
    double temp = pixie->GetChannelSetting(settingName[i][0].Data(), modID, ch, false);    
    entry[i]->SetNumber(temp );
  }
  
}

void ChannelSetting::ChangeOnOff(){
  short modID = modIDEntry->GetNumber();
  short ch = chIDEntry->GetNumber();
  int val = cbOnOff->GetSelected();

  pixie->SetChannelOnOff(val, modID, ch);
  pixie->SaveSettings(pixie->GetSettingFile(modIDEntry->GetNumber()));
}

void ChannelSetting::ChangePolarity(){
  short modID = modIDEntry->GetNumber();
  short ch = chIDEntry->GetNumber();
  int val = cbPolarity->GetSelected();
  
  pixie->SetChannelPositivePolarity(val, modID, ch);
  pixie->SaveSettings(pixie->GetSettingFile(modIDEntry->GetNumber()));
}

void ChannelSetting::ChangeTraceOnOff(){
  short modID = modIDEntry->GetNumber();
  short ch = chIDEntry->GetNumber();
  int val = cbTraceOnOff->GetSelected();
  
  pixie->SetChannelTraceOnOff(val, modID, ch);
  pixie->SaveSettings(pixie->GetSettingFile(modIDEntry->GetNumber()));
}

void ChannelSetting::ChangeQDCsumOnOff(){
  short modID = modIDEntry->GetNumber();
  short ch = chIDEntry->GetNumber();
  int val = cbQDCsumOnOff->GetSelected();
  
  pixie->SetChannelQDCsumOnOff(val, modID, ch);
  pixie->SaveSettings(pixie->GetSettingFile(modIDEntry->GetNumber()));
}

void ChannelSetting::ChangeID(int id){
  short modID = modIDEntry->GetNumber();
  short ch = chIDEntry->GetNumber();
  double val = entry[id]->GetNumber();
  
  pixie->SetChannelSetting(settingName[id][0].Data(), val, modID, ch, 1);
  
  pixie->SaveSettings(pixie->GetSettingFile(modIDEntry->GetNumber()));
}

