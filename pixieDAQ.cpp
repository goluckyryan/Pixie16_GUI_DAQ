#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGMenu.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGraph.h>
#include <TAxis.h>
#include "pixieDAQ.h"


enum MenuIdentifiers{
  
  M_FILE_OPEN,
  M_EXIT,
  M_MAIN_CH_SETTINGS,
  M_CH_SETTING,
  M_DIGITIZER_SETTINGS,
  M_DIGITIZER_INFOS
  
};

MainWindow::MainWindow(const TGWindow *p,UInt_t w,UInt_t h) {
  
  openPixie();
  
  /// Create a main frame
  fMain = new TGMainFrame(p,w,h);
  ///fMain->SetWMPosition(500, 500); //does not work
  fMain->Connect("CloseWindow()", "MainWindow", this, "GoodBye()");

  ///menu
  fMenuBar = new TGMenuBar(fMain, 1, 1, kHorizontalFrame);
  fMain->AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

  fMenuFile = new TGPopupMenu(gClient->GetRoot());
  fMenuFile->AddEntry("&Open...", M_FILE_OPEN);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("E&xit", M_EXIT);
  
  fMenuFile->Connect("Activated(Int_t)", "MainWindow", this, "HandleMenu(Int_t)");
  fMenuBar->AddPopup("&File",     fMenuFile,     new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
  
  fMenuSettings = new TGPopupMenu(gClient->GetRoot());
  fMenuSettings->AddEntry("&Main Ch Settings", M_MAIN_CH_SETTINGS);
  fMenuSettings->AddEntry("&Channel Setting", M_CH_SETTING);
  fMenuSettings->AddSeparator();
  fMenuSettings->AddEntry("Digitizer &Settings", M_DIGITIZER_SETTINGS);
  fMenuSettings->AddEntry("Digitizer &Info", M_DIGITIZER_INFOS);
  
  fMenuSettings->Connect("Activated(Int_t)", "MainWindow", this, "HandleMenu(Int_t)");
  fMenuBar->AddPopup("&Settings", fMenuSettings, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));


  /// Create a horizontal frame widget with buttons
  TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain,200,40);
  fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));
  
  ///================= signal Channel group
  TGGroupFrame * group1 = new TGGroupFrame(hframe, "Single Channel", kHorizontalFrame);
  hframe->AddFrame(group1, new  TGLayoutHints(kLHintsCenterX, 5,5,3,3) );
  
  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(group1,200,30);
  group1->AddFrame(hframe1);
  
  TGLabel * lb1 = new TGLabel(hframe1, "Module ID :");
  hframe1->AddFrame(lb1, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
  
  modIDEntry = new TGNumberEntry(hframe1, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  modIDEntry->SetWidth(50);
  modIDEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, pixie->GetNumModule()-1);
  hframe1->AddFrame(modIDEntry, new TGLayoutHints(kLHintsCenterX , 2, 2, 3, 2));
  
  TGLabel * lb2 = new TGLabel(hframe1, "Ch :");
  hframe1->AddFrame(lb2, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 3, 2));
  
  chEntry = new TGNumberEntry(hframe1, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  chEntry->SetWidth(50);
  chEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, pixie->GetDigitizerNumChannel(0));
  hframe1->AddFrame(chEntry, new TGLayoutHints(kLHintsCenterX , 2, 2, 3, 2));
  
  TGTextButton *bGetADCTrace = new TGTextButton(hframe1,"Get &ADC Trace");
  bGetADCTrace->Connect("Clicked()","MainWindow",this,"getADCTrace()");
  hframe1->AddFrame(bGetADCTrace, new TGLayoutHints(kLHintsCenterX, 2,2,3,2));

  TGTextButton *bGetBaseLine = new TGTextButton(hframe1,"Get &BaseLine");
  bGetBaseLine->Connect("Clicked()","MainWindow",this,"getBaseLine()");
  hframe1->AddFrame(bGetBaseLine, new TGLayoutHints(kLHintsCenterX, 2,2,3,2));

  ///================= Start Run group
  TGGroupFrame * group2 = new TGGroupFrame(hframe, "Start run", kHorizontalFrame);
  hframe->AddFrame(group2, new  TGLayoutHints(kLHintsCenterX, 2,2,3,3) );
  
  TGHorizontalFrame *hframe2 = new TGHorizontalFrame(group2,200,30);
  group2->AddFrame(hframe2);
  
  TGTextButton *bStartRun = new TGTextButton(hframe2,"Start &Run");
  bStartRun->Connect("Clicked()","MainWindow",this,"StartRun()");
  hframe2->AddFrame(bStartRun, new TGLayoutHints(kLHintsCenterX, 2,2,3,2));
  
  TGTextButton *bStopRun = new TGTextButton(hframe2,"Stop Run");
  bStopRun->Connect("Clicked()","MainWindow",this,"StopRun()");
  hframe2->AddFrame(bStopRun, new TGLayoutHints(kLHintsCenterX, 2,2,3,4));

  TGTextButton *bScalar = new TGTextButton(hframe2,"Scalar");
  //bScalar->Connect("Clicked()","MainWindow",this,"getADCTrace()");
  hframe2->AddFrame(bScalar, new TGLayoutHints(kLHintsCenterX, 2,2,3,2));


  ///================= Read evt group
  TGGroupFrame * group3 = new TGGroupFrame(hframe, "Read Evt", kHorizontalFrame);
  hframe->AddFrame(group3, new  TGLayoutHints(kLHintsCenterX, 2,2,3,3) );
  
  TGHorizontalFrame *hframe3 = new TGHorizontalFrame(group3,200,30);
  group3->AddFrame(hframe3);
  
  TGTextButton *bOpenEVT = new TGTextButton(hframe3,"OpenEvt");
  //bOpenEVT->Connect("Clicked()","MainWindow",this,"StartRun()");
  hframe3->AddFrame(bOpenEVT, new TGLayoutHints(kLHintsCenterX, 2,2,3,2));
  

  /// Create canvas widget
  fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fMain,800,400);
  fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));

  //===== TODO add msg box


  /// Set a name to the main frame
  fMain->SetWindowName("Pixie16 DAQ");

  /// Map all subwindows of main frame
  fMain->MapSubwindows();

  /// Initialize the layout algorithm
  fMain->Resize(fMain->GetDefaultSize());

  /// Map main frame
  fMain->MapWindow();
  
  
  ///HandleMenu(M_MAIN_CH_SETTINGS);
  
  ///================ pixie
  ///printf("Removing Pixie16Msg.log \n");
  ///remove( "Pixie16Msg.log");
  
  ///pixie = new Pixie16();
  ///if ( pixie->GetStatus() < 0 ) {
  ///  GoodBye();
  ///}
  
}
MainWindow::~MainWindow() {
  /// Clean up used widgets: frames, buttons, layout hints
  fMain->Cleanup();
  delete fMain;
  delete fMenuBar;
  delete fMenuFile;
  delete fMenuSettings;
}


void MainWindow::HandleMenu(Int_t id){
  switch(id){
    
    case M_FILE_OPEN:{
      
    }break;
    
    case M_EXIT: GoodBye(); break;
    
    case M_MAIN_CH_SETTINGS: {
      mainSettings = new MainSettings(gClient->GetRoot(), 600, 600, pixie);
    }break;
    
    
  }
  
}

void MainWindow::openPixie(){
  
  printf("Removing Pixie16Msg.log \n");
  remove( "Pixie16Msg.log");

  pixie = new Pixie16();
  if ( pixie->GetStatus() < 0 ) {
    printf("Exiting program... \n");
    GoodBye();
  }
  
}

void MainWindow::getADCTrace() {
  printf("--------- get ADCTrace \n");
  
  int modID = modIDEntry->GetNumber();
  int ch = chEntry->GetNumber();
  pixie->CaptureADCTrace(modID, ch);
  
  unsigned short * haha =  pixie->GetADCTrace();
  double dt = pixie->GetChannelSetting("XDT", modID, ch); 
  
  TGraph * gTrace = new TGraph();

  for( int i = 0 ; i < pixie->GetADCTraceLength(); i++){
    gTrace->SetPoint(i, i*dt, haha[i]);
  }
  gTrace->GetXaxis()->SetTitle("time [us]");
  gTrace->Draw("APL");
  
  TCanvas *fCanvas = fEcanvas->GetCanvas();
  fCanvas->cd();
  fCanvas->Update();
  
}

void MainWindow::getBaseLine(){

  printf("1 %s \n", pixie->GetSettingFile(0).c_str());
  
  int modID = modIDEntry->GetNumber();
  int ch = chEntry->GetNumber();
  pixie->CaptureBaseLine(modID, ch);

  printf("2 %s \n", pixie->GetSettingFile(0).c_str());
  
  double * baseline = pixie->GetBasline();
  double * baselineTime = pixie->GetBaselineTimestamp();
  
  printf("3 %s \n", pixie->GetSettingFile(0).c_str());
  
  TGraph * gTrace = new TGraph();
  
  for( int i = 0 ; i < pixie->GetBaslineLength(); i++){
    gTrace->SetPoint(i, baselineTime[i]*1000, baseline[i]);
  }
  gTrace->GetXaxis()->SetTitle("time [ns]");
  gTrace->Draw("APL");

  printf("4 %s \n", pixie->GetSettingFile(0).c_str());

  
  TCanvas *fCanvas = fEcanvas->GetCanvas();
  fCanvas->cd();
  fCanvas->Update();
  
}


void MainWindow::GoodBye(){
  
  pixie->CloseDigitizers();
  
  printf("----- bye bye ---- \n");
  
  gApplication->Terminate(0);
  
}

void MainWindow::StartRun(){
  
  pixie->StartRun(1);

  ///start a loop that show scalar, plot

  
}

void MainWindow::StopRun(){
  
  pixie->StopRun();
  
  pixie->PrintStatistics(0);
}


int main(int argc, char **argv) {
  printf(" Welcome to pixie16 DQ \n");
  
  TApplication theApp("App",&argc,argv);
  new MainWindow(gClient->GetRoot(),800,600);
  theApp.Run();
  return 0;
}
