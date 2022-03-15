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
  //lb1->SetWidth(50); lb1->ChangeOptions( kFixedSize);
  hframe->AddFrame(lb1, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

  TGNumberEntry * modIDEntry = new TGNumberEntry(hframe, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  modIDEntry->SetWidth(50);
  modIDEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, pixie->GetNumModule()-1);
  hframe->AddFrame(modIDEntry, new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
  
  ///Show Setting 
  TGVerticalFrame *hframeSettings = new TGVerticalFrame(fMain, w, 600 );
  fMain->AddFrame(hframeSettings, new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
  
  int modID = modIDEntry->GetNumber();
  int maxCh = pixie->GetDigitizerNumChannel(modID);
  
  
  //int numItems = 3;
  //TString labelText[numItems] = {"CH", "ON/off", "Gain"};
  //TGLabel ** labelItems = new TGLabel * [numItems];
  //TGVerticalFrame ** hframeItems = new TGVerticalFrame * [numItems];
  //
  //for( int j = 0; i < numItems; j++){
  //  
  //  hframeItems[j] = new TGVerticalFrame(hframeSettings, 100, 600);
  //  hframeSettings->AddFrame(hframeItems[j], new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
  //  
  //  
  //  
  //  
  //  
  //}
  
  //
  //for( int i = 0; i < 3; i++){
  //  labelItems[i] = new TGLabel(hframeLabel, Form("%s", labelText[i].Data()));
  //  hframeLabel->AddFrame(labelItems[i],new TGLayoutHints(kLHintsCenterX, 2,2,2,2) );
  //}
  
  
  TGHorizontalFrame ** hframeCh = new TGHorizontalFrame * [maxCh];
  TGLabel ** lbCh = new TGLabel * [maxCh];
  TGComboBox ** cbOnOff = new TGComboBox * [maxCh];
  TGComboBox ** cbGain = new TGComboBox * [maxCh];
  
  for( int i = 0; i < maxCh ; i ++){
    hframeCh[i] = new TGHorizontalFrame(hframeSettings, w, 600 );
    hframeSettings->AddFrame(hframeCh[i], new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
    
    lbCh[i] = new TGLabel(hframeCh[i] , Form("%02d", i));
    hframeCh[i]->AddFrame(lbCh[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));
    
    cbOnOff[i] = new TGComboBox(hframeCh[i], i);
    cbOnOff[i]->AddEntry("ON", 1);
    cbOnOff[i]->AddEntry("off", 2);
    cbOnOff[i]->Resize(50, 20);
    pixie->GetChannelOnOff(modID, i) ? cbOnOff[i]->Select(1) : cbOnOff[i]->Select(2);
    hframeCh[i]->AddFrame(cbOnOff[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    cbGain[i] = new TGComboBox(hframeCh[i], i);
    cbGain[i]->AddEntry("x1", 1);
    cbGain[i]->AddEntry("x1/4", 2);
    cbGain[i]->Resize(50, 20);
    pixie->GetChannelGain(modID, i) ? cbGain[i]->Select(1) : cbGain[i]->Select(2);
    hframeCh[i]->AddFrame(cbGain[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    
    //TGNumberEntry * modIDEntry = new TGNumberEntry(hframe, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    
  }
  
  fMain->MapSubwindows();
  fMain->Resize(fMain->GetDefaultSize());
  fMain->MapWindow();
}


MainSettings::~MainSettings(){
  
  fMain->Cleanup();
  delete fMain;
  
}
