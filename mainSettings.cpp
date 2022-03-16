#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGComboBox.h>
#include <TGTableContainer.h>

#include "mainSettings.h"


MainSettings::MainSettings(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie){
  
  this->pixie = pixie; 
  
  fMain = new TGMainFrame(p,w,h);
  fMain->SetWindowName("Pixie16 Channel Settings ");
  fMain->Connect("CloseWindow()", "MainSettings", this, "CloseWindow()");
  
  
  ///Module choose
  TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain, w, 50 );
  fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
  
  TGLabel * lb1 = new TGLabel(hframe, "Module ID : ");
  hframe->AddFrame(lb1, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

  TGNumberEntry * modIDEntry = new TGNumberEntry(hframe, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  modIDEntry->SetWidth(50);
  modIDEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, pixie->GetNumModule()-1);
  hframe->AddFrame(modIDEntry, new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
  
  TGTextButton * bSave = new TGTextButton(hframe,"&Save");
  bSave->Connect("Clicked()","MainSettings",this, "kaka()");
  hframe->AddFrame(bSave, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));
  
  /// Setting
  
  int modID = modIDEntry->GetNumber();
  int maxCh = pixie->GetDigitizerNumChannel(modID);
  
  int numItems = 14;
  TString labelText[numItems] = {"CH", "ON/off", "Gain", "Trig L", "Trig G", "Threshold", "Polarity", "Energy L", "Energy G", "Tau", "Trace Len", "Trace delay", "V offset", "BaseLine"};
  double width[numItems]     = {  50,       50,    50,     60,       60,           80,       60,       60,          60,       60,      80,           80,             70,      80};
  
  TGVerticalFrame *hframeSettings = new TGVerticalFrame(fMain, w, 600 );
  fMain->AddFrame(hframeSettings, new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
  
  TGHorizontalFrame * hframeLabel = new TGHorizontalFrame(hframeSettings, w, 50);
  hframeSettings->AddFrame(hframeLabel, new TGLayoutHints(kLHintsExpandX, 2,2,2,2)); 
  
  TGLabel ** labelItems = new TGLabel * [numItems];
 
  for(int i = 0; i < numItems; i++){
    labelItems[i] = new TGLabel(hframeLabel, Form("%s", labelText[i].Data()));
    labelItems[i]->Resize(width[i], 20);
    labelItems[i]->SetMargins(0, (i == 0 ? 0 : 10) , 0, 0);
    hframeLabel->AddFrame(labelItems[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));
  }

  TGHorizontalFrame ** hframeCh = new TGHorizontalFrame * [maxCh];
  TGLabel ** lbCh = new TGLabel * [maxCh];
  TGComboBox ** cbOnOff = new TGComboBox * [maxCh];
  TGComboBox ** cbGain = new TGComboBox * [maxCh];
  TGNumberEntry ** neTrigL = new  TGNumberEntry * [maxCh];
  TGNumberEntry ** neTrigG = new  TGNumberEntry * [maxCh];
  TGNumberEntry ** neThreshold = new  TGNumberEntry * [maxCh];
  TGComboBox ** cbPol = new TGComboBox * [maxCh];
  TGNumberEntry ** neEngL = new  TGNumberEntry * [maxCh];
  TGNumberEntry ** neEngG = new  TGNumberEntry * [maxCh];
  TGNumberEntry ** neTau  = new  TGNumberEntry * [maxCh];
  TGNumberEntry ** neTraceLength  = new  TGNumberEntry * [maxCh];
  TGNumberEntry ** neTraceDelay  = new  TGNumberEntry * [maxCh];
  TGNumberEntry ** neVoff  = new  TGNumberEntry * [maxCh];
  TGNumberEntry ** neBL  = new  TGNumberEntry * [maxCh];
  
  for( int i = 0; i < maxCh ; i ++){
    hframeCh[i] = new TGHorizontalFrame(hframeSettings, w, 600 );
    hframeSettings->AddFrame(hframeCh[i], new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
    
    int col = 0;
    lbCh[i] = new TGLabel(hframeCh[i] , Form("%02d", i));
    lbCh[i]->SetWidth(width[col]);
    hframeCh[i]->AddFrame(lbCh[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));
    
    col++;
    cbOnOff[i] = new TGComboBox(hframeCh[i], i);
    cbOnOff[i]->AddEntry("ON", 1);
    cbOnOff[i]->AddEntry("off", 2);
    cbOnOff[i]->Resize(width[col], 20);
    pixie->GetChannelOnOff(modID, i) ? cbOnOff[i]->Select(1) : cbOnOff[i]->Select(2);
    hframeCh[i]->AddFrame(cbOnOff[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    col++;
    cbGain[i] = new TGComboBox(hframeCh[i], i);
    cbGain[i]->AddEntry("x1", 1);
    cbGain[i]->AddEntry("x1/4", 2);
    cbGain[i]->Resize(width[col], 20);
    pixie->GetChannelGain(modID, i) ? cbGain[i]->Select(1) : cbGain[i]->Select(2);
    hframeCh[i]->AddFrame(cbGain[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    col++;
    neTrigL[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelTriggerRiseTime(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
    neTrigL[i]->SetWidth(width[col]);
    neTrigL[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 2.0);
    hframeCh[i]->AddFrame(neTrigL[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
    col++;
    neTrigG[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelTriggerFlatTop(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
    neTrigG[i]->SetWidth(width[col]);
    neTrigG[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 2.0);
    hframeCh[i]->AddFrame(neTrigG[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
    col++;
    neThreshold[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelTriggerThreshold(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
    neThreshold[i]->SetWidth(width[col]);
    neThreshold[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 4000);
    hframeCh[i]->AddFrame(neThreshold[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
    col++;
    cbPol[i] = new TGComboBox(hframeCh[i], i);
    cbPol[i]->AddEntry("Pos +", 1);
    cbPol[i]->AddEntry("Neg -", 2);
    cbPol[i]->Resize(width[col], 20);
    pixie->GetChannelPolarity(modID, i) ? cbPol[i]->Select(1) : cbPol[i]->Select(2);
    hframeCh[i]->AddFrame(cbPol[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));
    
    col++;
    neEngL[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelEnergyRiseTime(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
    neEngL[i]->SetWidth(width[col]);
    neEngL[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 10.0);
    hframeCh[i]->AddFrame(neEngL[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
    col++;
    neEngG[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelEnergyFlatTop(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
    neEngG[i]->SetWidth(width[col]);
    neEngG[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 10.0);
    hframeCh[i]->AddFrame(neEngG[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));

    col++;
    neTau[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelEnergyTau(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
    neTau[i]->SetWidth(width[col]);
    neTau[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 300.0);
    hframeCh[i]->AddFrame(neTau[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
  
    col++;
    double tracelen = pixie->GetChannelTraceLength(modID, i);
    if( pixie->GetChannelTraceOnOff(modID, i) == false ) tracelen = -1;
    neTraceLength[i] = new TGNumberEntry(hframeCh[i], tracelen, 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEAAnyNumber);
    neTraceLength[i]->SetWidth(width[col]);
    neTraceLength[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, -1, 20.0);
    hframeCh[i]->AddFrame(neTraceLength[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
    col++;
    double tracedel = pixie->GetChannelTraceDelay(modID, i);
    if( pixie->GetChannelTraceOnOff(modID, i) == false ) tracedel = -1;
    neTraceDelay[i] = new TGNumberEntry(hframeCh[i], tracedel, 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEAAnyNumber);
    neTraceDelay[i]->SetWidth(width[col]);
    neTraceDelay[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, -1, 20.0);
    hframeCh[i]->AddFrame(neTraceDelay[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
    col++;
    neVoff[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelVOffset(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEAAnyNumber);
    neVoff[i]->SetWidth(width[col]);
    neVoff[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, -2, 2);
    hframeCh[i]->AddFrame(neVoff[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
    col++;
    neBL[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelBaseLinePrecent(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
    neBL[i]->SetWidth(width[col]);
    neBL[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 100.0);
    //neBL[i]->Connect("Modified()", "MainSettings", this, Form("ChangeBL(==%d, =%d)",modID, i)); 
    //neBL[i]->Connect("Modified()", "MainSettings", this, "ChangeBL(0,1)"); 
    //neBL[i]->Connect("Modified()", "MainSettings", this, "kaka()"); 
    hframeCh[i]->AddFrame(neBL[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
  }
  
  
  fMain->MapSubwindows();
  fMain->Resize(fMain->GetDefaultSize());
  fMain->MapWindow();
}


MainSettings::~MainSettings(){
  
  fMain->Cleanup();
  delete fMain;
  
}

void MainSettings::ChangeBL(unsigned short mod, unsigned short ch){
  printf("mod : %d , ch : %d\n", mod, ch);
}


void MainSettings::kaka(){
  
  printf("save button is pressed.\n");
}
