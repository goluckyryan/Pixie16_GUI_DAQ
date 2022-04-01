#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>
#include <TGTableContainer.h>
#include <TGFileDialog.h>

#include "scalarPanel.h"

///declaration of static variables
Pixie16 * ScalarPanel::pixie = NULL;
bool ScalarPanel::updateFlag = true;
int ScalarPanel::nMod = 0;
TGTextEntry  * ScalarPanel::teRate[][MAXCH] = {NULL};
TGTextEntry  * ScalarPanel::teRealTime[MAXMOD] = {NULL};

int updateTime = 500; // msec

ScalarPanel::ScalarPanel(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie){
  
  this->pixie = pixie; 
  
  nMod = pixie->GetNumModule();
  
  fMain = new TGMainFrame(p,w,h);
  fMain->SetWindowName("Pixie16 Scalar Panel");
  fMain->Connect("CloseWindow()", "ScalarPanel", this, "CloseWindow()");

  ///Module choose
  TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain, w, 50 );
  fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,2,2));
  
  int width = 30;
  
  ///------ Channel labels
  TGVerticalFrame * hChannelLabels = new TGVerticalFrame(hframe);
  hframe->AddFrame(hChannelLabels, new TGLayoutHints(kLHintsCenterX  | kLHintsCenterY, 2,2,2,2));
  TGLabel * labelRealTime = new TGLabel(hChannelLabels, "Read Time");
  labelRealTime->SetWidth(width);
  labelRealTime->Resize(width, 50);
  hChannelLabels->AddFrame(labelRealTime, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,5,5));
  
  TGLabel * labelCh[MAXCH];
  for( int ch = 0; ch < MAXCH; ch ++){
    labelCh[ch] = new TGLabel(hChannelLabels, Form("%02d", ch));
    labelCh[ch]->SetWidth(width);
    labelCh[ch]->Resize(width, 50);
    hChannelLabels->AddFrame(labelCh[ch], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,5,5));
  }
  
  ///------- Rate
  TGVerticalFrame * hScalarMod[nMod];

  for ( int mod = 0 ; mod < nMod ; mod ++){
    hScalarMod[mod] = new TGVerticalFrame(hframe);
    hframe->AddFrame(hScalarMod[mod], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,2,2));
  
    teRealTime[mod] = new TGTextEntry(hScalarMod[mod], new TGTextBuffer(10));
    teRealTime[mod]->SetEnabled(false);
    teRealTime[mod]->SetAlignment(kTextRight);
    hScalarMod[mod]->AddFrame(teRealTime[mod], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,2,2));
    
    for( int ch = 0 ; ch < MAXCH ; ch ++){
      teRate[mod][ch] = new TGTextEntry(hScalarMod[mod], new TGTextBuffer(10));
      teRate[mod][ch]->SetEnabled(false);
      teRate[mod][ch]->SetAlignment(kTextRight);
      hScalarMod[mod]->AddFrame(teRate[mod][ch], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,2,2));
    }
  }
  
  TGLabel * labelFootNote = new TGLabel(fMain, Form("Update every %d msec.", updateTime));
  fMain->AddFrame(labelFootNote, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 2,2,5,5));
  
  fMain->MapSubwindows();
  fMain->Resize(fMain->GetDefaultSize());
  fMain->MapWindow();
  
  isOpened = true;
  updateFlag = true;
  thread = new TThread("kakaka", UpdateScalar, (void *) 0);
  thread->Run();
}


ScalarPanel::~ScalarPanel(){
  
  updateFlag = false;
  isOpened = false;
  
  for( int i = 0; i < MAXMOD; i++){
    for( int j = 0; j < MAXCH; j++ ){
      delete teRate[i][j];
    }
    
    delete teRealTime[i];
  }
  
  /// fMain must be delete last;
  fMain->Cleanup();
  delete fMain;
  
}

void * ScalarPanel::UpdateScalar(void * ptr){

  while(updateFlag){
  
    int ss = Pixie16GetStatisticsSize();
    unsigned int statistics[ss];

    for( int mod = 0; mod < nMod; mod++){
      
      Pixie16ReadStatisticsFromModule (statistics, mod);
       
      double realTime = Pixie16ComputeRealTime (statistics, mod);
      
      teRealTime[mod]->SetText(Form("%.2f", realTime));
       
      for( int ch = 0; ch < MAXCH ; ch ++){
        
        double ICR = Pixie16ComputeInputCountRate (statistics, mod, ch);
        double OCR = Pixie16ComputeOutputCountRate (statistics, mod, ch);
        double liveTime = Pixie16ComputeLiveTime (statistics, mod, ch);
        
        teRate[mod][ch]->SetText(Form("%.2f[%.2f] %.2f", ICR, OCR, liveTime));
        
      }
      
      gSystem->Sleep(updateTime); 
      
    }
    
  }
  
  printf("quite Update Scalar\n");
  
  return ptr;

}
