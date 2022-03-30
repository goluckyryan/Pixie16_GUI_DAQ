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
#include <TGTextEditor.h>
#include <TAxis.h>
#include <TBenchmark.h>

#include <thread>
#include <unistd.h>

#include "Pixie16Class.h"

#include "pixieDAQ.h"


enum MenuIdentifiers{
  
  M_FILE_OPEN,
  M_EXIT,
  M_CH_SETTINGS_SUMMARY,
  M_CH_SETTING,
  M_MODULE_SETTINGS
  
};

///make static members 
Pixie16 * MainWindow::pixie = NULL;

MainWindow::MainWindow(const TGWindow *p,UInt_t w,UInt_t h) {
  
  printf("Removing Pixie16Msg.log \n");
  remove( "Pixie16Msg.log");

  pixie = new Pixie16();
  if ( pixie->GetStatus() < 0 ) {
    printf("Exiting program... \n");
    GoodBye();
  }
  
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
  fMenuSettings->AddEntry("&Settings Summary", M_CH_SETTINGS_SUMMARY);
  fMenuSettings->AddEntry("&Channel Setting", M_CH_SETTING);
  fMenuSettings->AddSeparator();
  fMenuSettings->AddEntry("Digitizer &Settings", M_MODULE_SETTINGS);
  
  fMenuSettings->Connect("Activated(Int_t)", "MainWindow", this, "HandleMenu(Int_t)");
  fMenuBar->AddPopup("&Settings", fMenuSettings, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));


  TGLayoutHints * uniLayoutHints = new TGLayoutHints(kLHintsNormal, 2,2,10,0); ///left, right, top, bottom

  /// Create a horizontal frame widget with buttons
  TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain,200,40);
  fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));
  
  ///================= signal Channel group
  TGGroupFrame * group1 = new TGGroupFrame(hframe, "Single Channel", kHorizontalFrame);
  hframe->AddFrame(group1 );
  
  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(group1,200,30);
  group1->AddFrame(hframe1 );
  
  TGLabel * lb1 = new TGLabel(hframe1, "Module ID :");
  hframe1->AddFrame(lb1, uniLayoutHints);
  
  modIDEntry = new TGNumberEntry(hframe1, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  modIDEntry->SetWidth(50);
  modIDEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, pixie->GetNumModule()-1);
  hframe1->AddFrame(modIDEntry, uniLayoutHints);
  
  TGLabel * lb2 = new TGLabel(hframe1, "Ch :");
  hframe1->AddFrame(lb2, uniLayoutHints);
  
  chEntry = new TGNumberEntry(hframe1, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  chEntry->SetWidth(50);
  chEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, pixie->GetDigitizerNumChannel(0));
  hframe1->AddFrame(chEntry, uniLayoutHints);
  
  TGTextButton *bGetADCTrace = new TGTextButton(hframe1,"Get &ADC Trace");
  bGetADCTrace->Connect("Clicked()","MainWindow",this,"GetADCTrace()");
  hframe1->AddFrame(bGetADCTrace, uniLayoutHints);

  TGTextButton *bGetBaseLine = new TGTextButton(hframe1,"Get &BaseLine");
  bGetBaseLine->Connect("Clicked()","MainWindow",this,"GetBaseLine()");
  hframe1->AddFrame(bGetBaseLine, uniLayoutHints);

  TGTextButton *bScope = new TGTextButton(hframe1,"&Scope");
  bScope->Connect("Clicked()","MainWindow",this,"Scope()");
  hframe1->AddFrame(bScope, uniLayoutHints);

  ///================= Start Run group
  TGGroupFrame * group2 = new TGGroupFrame(hframe, "Start run", kHorizontalFrame);
  //hframe->AddFrame(group2, new  TGLayoutHints(kLHintsCenterX, 5,5,3,3) );
  hframe->AddFrame(group2);
  
  TGHorizontalFrame *hframe2 = new TGHorizontalFrame(group2,200,30);
  group2->AddFrame(hframe2);
  
  tePath = new TGTextEntry(hframe2, new TGTextBuffer(10));
  tePath->SetText("haha.evt");
  hframe2->AddFrame(tePath, uniLayoutHints);
  
  bStartRun = new TGTextButton(hframe2,"Start &Run");
  bStartRun->Connect("Clicked()","MainWindow",this,"StartRun()");
  hframe2->AddFrame(bStartRun, uniLayoutHints);
  
  bStopRun = new TGTextButton(hframe2,"Stop Run");
  bStopRun->Connect("Clicked()","MainWindow",this,"StopRun()");
  hframe2->AddFrame(bStopRun, uniLayoutHints);

  TGTextButton *bScalar = new TGTextButton(hframe2,"Scalar");
  bScalar->Connect("Clicked()","MainWindow",this,"OpenScalar()");
  hframe2->AddFrame(bScalar, uniLayoutHints);


  ///================= Read evt group
  //TGGroupFrame * group3 = new TGGroupFrame(hframe, "Read Evt", kHorizontalFrame);
  ////hframe->AddFrame(group3, new  TGLayoutHints(kLHintsCenterX, 5,5,3,3) );
  //hframe->AddFrame(group3);
  //
  //TGHorizontalFrame *hframe3 = new TGHorizontalFrame(group3,200,30);
  //group3->AddFrame(hframe3);
  //
  //TGTextButton *bOpenEVT = new TGTextButton(hframe3,"OpenEvt");
  ////bOpenEVT->Connect("Clicked()","MainWindow",this,"StartRun()");
  //hframe3->AddFrame(bOpenEVT, uniLayoutHints);
    

  ///================= Create canvas widget
  fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fMain,900,400);
  fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,10));

  ///================= Log massage
  TGGroupFrame * groupLog = new TGGroupFrame(fMain, "Log Message", kHorizontalFrame);
  fMain->AddFrame(groupLog, new  TGLayoutHints(kLHintsCenterX, 5,5,0,5) );
  
  teLog = new TGTextEdit(groupLog, w, 60);
  groupLog->AddFrame(teLog,  new TGLayoutHints(kLHintsNormal, 0,0,10,0));


  /// Set a name to the main frame
  fMain->SetWindowName("Pixie16 DAQ");

  /// Map all subwindows of main frame
  fMain->MapSubwindows();

  /// Initialize the layout algorithm
  fMain->Resize(fMain->GetDefaultSize());

  /// Map main frame
  fMain->MapWindow();
  
  /// setup thread
  thread = new TThread("hahaha", SaveData, (void *) 1);
  
  settingsSummary = NULL;
  moduleSetting = NULL;
  channelSetting = NULL;
  scalarPanel = NULL;
  
  bStopRun->SetEnabled(false);
  
  ///HandleMenu(M_CH_SETTINGS_SUMMARY);

  
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
    
    ///========================= File Open
    case M_FILE_OPEN:{
      
    }break;
    
    ///========================= Exit
    case M_EXIT: GoodBye(); break;
    
    ///========================= Channel setting summary
    case M_CH_SETTINGS_SUMMARY: {
      if( settingsSummary == NULL ) {
        settingsSummary = new SettingsSummary(gClient->GetRoot(), 600, 600, pixie);
      }else{
        if( !settingsSummary->isOpened ) {
          settingsSummary = new SettingsSummary(gClient->GetRoot(), 600, 600, pixie);
        }
      }
    }break;
    
    case M_CH_SETTING:{
      if( channelSetting == NULL ) {
        channelSetting = new ChannelSetting(gClient->GetRoot(), 600, 600, pixie);
      }else{
        if( !channelSetting->isOpened ) {
          channelSetting = new ChannelSetting(gClient->GetRoot(), 600, 600, pixie);
        }
      }
      
    }break;
    
    ///========================= Module setting
    case M_MODULE_SETTINGS:{
      if( moduleSetting == NULL ) {
        moduleSetting = new ModuleSetting(gClient->GetRoot(), 600, 600, pixie);
      }else{
        if( !moduleSetting->isOpened ) {
          moduleSetting = new ModuleSetting(gClient->GetRoot(), 600, 600, pixie);
        }
      }
      
    }break;
    
  }
  
}


void MainWindow::GetADCTrace() {
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

void MainWindow::GetBaseLine(){

  int modID = modIDEntry->GetNumber();
  int ch = chEntry->GetNumber();
  pixie->CaptureBaseLine(modID, ch);
  
  double * baseline = pixie->GetBasline();
  double * baselineTime = pixie->GetBaselineTimestamp();
  
  TGraph * gTrace = new TGraph();
  
  for( int i = 0 ; i < pixie->GetBaslineLength(); i++){
    gTrace->SetPoint(i, baselineTime[i]*1000, baseline[i]);
  }
  gTrace->GetXaxis()->SetTitle("time [ns]");
  gTrace->Draw("APL");
  
  TCanvas *fCanvas = fEcanvas->GetCanvas();
  fCanvas->cd();
  fCanvas->Update();
  
}

void MainWindow::Scope(){
  
  int modID = modIDEntry->GetNumber();
  int ch = chEntry->GetNumber();
  
  if( pixie->GetChannelOnOff(modID, ch) == false ){
    LogMsg(Form("ch-%d is disabled\n", ch));
    return;
  }
  
  
  double dt = pixie->GetCh2ns(modID);
  
  DataBlock * data = pixie->GetData(); 
  
  LogMsg("Take data for 200 msec.");
  pixie->StartRun(1);
  
  usleep(200*1000);
  pixie->ReadData(0);
  pixie->StopRun();
  
  TGraph * gTrace = new TGraph();
  
  printf("              next word : %u\n", pixie->GetNextWord());
  printf("number of word received : %u\n", pixie->GetnFIFOWords());

  //TODO add statistics, like trigger rate
  
  while(!pixie->ProcessSingleData()){
    
    if( pixie->GetNextWord() >= pixie->GetnFIFOWords() ) break;
    if( data->slot < 2 ) break;
    if(  data->eventID >= pixie->GetnFIFOWords() ) break;
    
    printf("mod:%d, ch:%d, event:%llu, %u, %u\n", data->slot-2, data->ch, data->eventID, pixie->GetNextWord(), pixie->GetnFIFOWords() );
      
    if( data->ch == ch && data->slot == modID + 2 ){

      for( int i = 0 ; i < data->trace_length; i++){
        gTrace->SetPoint(i, i*dt, (data->trace)[i]);
      }
      gTrace->GetXaxis()->SetTitle("time [us]");
      gTrace->SetTitle(Form("mod:%d, ch:%d, event:%llu\n", modID, ch, data->eventID));
      gTrace->Draw("APL");
      
      TCanvas *fCanvas = fEcanvas->GetCanvas();
      fCanvas->cd();
      fCanvas->Update();
    
      break;
      
    }
  }
  
  
  printf("=============== finished \n");
  
}



void MainWindow::GoodBye(){
  
  pixie->CloseDigitizers();
  
  printf("----- bye bye ---- \n");
  
  gApplication->Terminate(0);
  
}

void MainWindow::StartRun(){
  
  pixie->OpenFile(tePath->GetText(), false);
  
  LogMsg(Form("Start Run. Save data at %s.\n", tePath->GetText()));
  
  pixie->StartRun(1);

  if( pixie->IsRunning() ) thread->Run(); /// call SaveData()
  
  bStartRun->SetEnabled(false);
  bStopRun->SetEnabled(true);
}


void * MainWindow::SaveData(void* ptr){
  
  printf("Save Data()\n");
  
  while( pixie->IsRunning() ){
    
    usleep(500*1000); /// 500 msec
    
    pixie->ReadData(0);
    pixie->SaveData();
    
    //TODO Get file size, Fill HISTORGRAM;
    
  }
  
  pixie->ReadData(0);
  pixie->SaveData();
  
  printf("finished Save Data.\n");
  
  return ptr;
  
}

void MainWindow::StopRun(){
  
  pixie->StopRun();
  
  pixie->CloseFile();
  
  LogMsg("Stop Run");
  
  pixie->PrintStatistics(0);
  bStartRun->SetEnabled(true);
  bStopRun->SetEnabled(false);
}


void MainWindow::OpenScalar(){
  
  if( scalarPanel == NULL ) {
    scalarPanel = new ScalarPanel(gClient->GetRoot(), 600, 600, pixie);
  }else{
    if( !scalarPanel->isOpened ) scalarPanel = new ScalarPanel(gClient->GetRoot(), 600, 600, pixie);
  }
}

void MainWindow::LogMsg(TString msg){
  teLog->AddLine(msg);
  teLog->LineDown();
  teLog->End();
}


int main(int argc, char **argv) {
  printf(" Welcome to pixie16 DQ \n");
  
  TApplication theApp("App",&argc,argv);
  new MainWindow(gClient->GetRoot(),800,800);
  theApp.Run();
  return 0;
}
