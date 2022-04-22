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
#include <TF1.h>

#include <unistd.h>
#include <ctime>

#include "Pixie16Class.h"

#include "pixieDAQ.h"


enum MenuIdentifiers{
  
  M_FILE_OPEN,
  M_EXIT,
  M_CH_SETTINGS_SUMMARY,
  M_CH_SETTING,
  M_MODULE_SETTINGS,
  M_PROGRAM_SETTINGS,
  M_FINDPEAKS,
  M_SHOW_CHANNELS_RATE
  
};

//TODO timed Run, //pixie->SetDigitizerPresetRunTime(100000, 0);

///make static members 
Pixie16 * MainWindow::pixie = NULL;
TGTextEdit * MainWindow::teLog = NULL;
TRootEmbeddedCanvas * MainWindow::fEcanvas = NULL;
TGNumberEntry * MainWindow::modIDEntry = NULL;
TGNumberEntry * MainWindow::chEntry = NULL; 
TH1F * MainWindow::hEnergy[MAXMOD][MAXCH]={NULL};
TH1F * MainWindow::hChannel[MAXMOD]={NULL};
bool MainWindow::isEnergyHistFilled = false;

MainWindow::MainWindow(const TGWindow *p,UInt_t w,UInt_t h) {
  
  printf("Removing Pixie16Msg.log \n");
  remove( "Pixie16Msg.log");
  
  pixie = new Pixie16();
  if ( pixie->GetStatus() < 0 ) {
    printf("Exiting program... \n");
    GoodBye();
  }
  
  for( unsigned int i = 0; i < pixie->GetNumModule() ; i++){
    for( int j = 0; j < MAXCH ; j++){
      hEnergy[i][j] = new TH1F(Form("hEnergy%02d_%02d", i, j), Form("Energy mod:%2d ch:%02d (down scaled)", i, j), 200, 0, 160000);
    }
    hChannel[i] = new TH1F(Form("hChannel%02d", i), Form("Channel Stat. mod:%2d (down scaled)", i), MAXCH, 0, MAXCH);
  }
  
  /// Create a main frame
  fMain = new TGMainFrame(p,w,h);
  ///fMain->SetWMPosition(500, 500); //does not work
  fMain->Connect("CloseWindow()", "MainWindow", this, "GoodBye()");

  ///======================= menu
  fMenuBar = new TGMenuBar(fMain, 1, 1, kHorizontalFrame);
  fMain->AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

  fMenuFile = new TGPopupMenu(gClient->GetRoot());
  fMenuFile->AddEntry("&Open Pixie16.config", M_FILE_OPEN);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("E&xit", M_EXIT);
  fMenuFile->Connect("Activated(Int_t)", "MainWindow", this, "HandleMenu(Int_t)");
  fMenuBar->AddPopup("&File",     fMenuFile,     new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
  
  fMenuSettings = new TGPopupMenu(gClient->GetRoot());
  fMenuSettings->AddEntry("&Settings Summary", M_CH_SETTINGS_SUMMARY);
  fMenuSettings->AddEntry("&Channel Setting", M_CH_SETTING);
  fMenuSettings->AddSeparator();
  fMenuSettings->AddEntry("&Digitizer Settings", M_MODULE_SETTINGS);
  fMenuSettings->AddSeparator();
  fMenuSettings->AddEntry("Program Settings", M_PROGRAM_SETTINGS);
  fMenuSettings->Connect("Activated(Int_t)", "MainWindow", this, "HandleMenu(Int_t)");
  fMenuBar->AddPopup("&Settings", fMenuSettings, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));

  fMenuUtility = new TGPopupMenu(gClient->GetRoot());
  fMenuUtility->AddEntry("Plot Channels Rate", M_SHOW_CHANNELS_RATE);
  fMenuSettings->AddSeparator();
  fMenuUtility->AddEntry("Find &Peaks", M_FINDPEAKS);
  fMenuUtility->Connect("Activated(Int_t)", "MainWindow", this, "HandleMenu(Int_t)");
  fMenuBar->AddPopup("&Utility", fMenuUtility, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));

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
  modIDEntry->Connect("Modified()", "MainWindow", this, "ChangeMod()"); 
  if( pixie->GetNumModule() == 1 ) modIDEntry->SetState(false);
  hframe1->AddFrame(modIDEntry, uniLayoutHints);
  
  TGLabel * lb2 = new TGLabel(hframe1, "Ch :");
  hframe1->AddFrame(lb2, uniLayoutHints);
  
  chEntry = new TGNumberEntry(hframe1, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  chEntry->SetWidth(50);
  chEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, pixie->GetDigitizerNumChannel(0));
  chEntry->Connect("Modified()", "MainWindow", this, "ChangeChannel()"); 
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

  bFitTrace = new TGTextButton(hframe1,"&Fit");
  bFitTrace->SetEnabled(false);
  bFitTrace->Connect("Clicked()","MainWindow",this,"FitTrace()");
  hframe1->AddFrame(bFitTrace, uniLayoutHints);

  ///================= Start Run group
  TGGroupFrame * group2 = new TGGroupFrame(hframe, "Start run", kHorizontalFrame);
  //hframe->AddFrame(group2, new  TGLayoutHints(kLHintsCenterX, 5,5,3,3) );
  hframe->AddFrame(group2);
  
  TGHorizontalFrame *hframe2 = new TGHorizontalFrame(group2,200,30);
  group2->AddFrame(hframe2);
  
  TGLabel * lb1Prefix = new TGLabel(hframe2, "Save Prefix:");
  hframe2->AddFrame(lb1Prefix, uniLayoutHints);
  
  tePath = new TGTextEntry(hframe2, new TGTextBuffer(30));
  tePath->SetWidth(50);
  tePath->SetText("haha");
  hframe2->AddFrame(tePath, uniLayoutHints);
  
  runIDEntry = new TGNumberEntry(hframe2, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
  runIDEntry->SetWidth(50);
  runIDEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 999);
  hframe2->AddFrame(runIDEntry, uniLayoutHints);
  
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
  
  teLog = new TGTextEdit(groupLog, w, 100);
  groupLog->AddFrame(teLog,  new TGLayoutHints(kLHintsNormal, 0,0,10,0));


  /// Set a name to the main frame
  fMain->SetWindowName("Pixie16 DAQ");

  /// Map all subwindows of main frame
  fMain->MapSubwindows();

  /// Initialize the layout algorithm
  fMain->Resize(fMain->GetDefaultSize());
  fMain->SetWMPosition(200, 200); //does not work??
  
  /// Map main frame
  fMain->MapWindow();
  
  /// setup thread
  saveDataThread = new TThread("hahaha", SaveData, (void *) 1);
  fillHistThread = new TThread("kakaka", FillHistogram, (void *) 1);
  
  settingsSummary = NULL;
  moduleSetting = NULL;
  channelSetting = NULL;
  scalarPanel = NULL;
  
  bStopRun->SetEnabled(false);
  
  LogMsg("Boot OK and ready to run.");
  
  gTrace = new TGraph();
  
  ///HandleMenu(M_CH_SETTINGS_SUMMARY);

  
}
MainWindow::~MainWindow() {

  delete fMenuBar;
  delete fMenuFile;
  delete fMenuSettings;
  
  delete modIDEntry;
  delete chEntry;
  delete tePath;
  delete teLog;
  
  delete bStartRun;
  delete bStopRun;
  delete bFitTrace;
  
  delete pixie;
  
  delete settingsSummary;
  delete moduleSetting;
  delete channelSetting;
  delete scalarPanel;
  
  delete saveDataThread;
  delete fillHistThread;
  
  delete gTrace;
  
  delete configEditor;
  
  /// Clean up used widgets: frames, buttons, layout hints
  fMain->Cleanup();
  delete fMain;
}


void MainWindow::HandleMenu(Int_t id){
  switch(id){
    
    ///========================= File Open
    case M_FILE_OPEN:{
      
      configEditor = new TGTextEditor("Pixie16.config", gClient->GetRoot(), 1200);
      
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

    ///========================= Program setting
    case M_PROGRAM_SETTINGS:{
      LogMsg("[Program settings] Not impelmented");
    }break;
    
    ///====================== Show channel rate;
    case M_SHOW_CHANNELS_RATE:{
      
      int modID = modIDEntry->GetNumber();
      fEcanvas->GetCanvas()->cd();
      hChannel[modID]->Draw();
      fEcanvas->GetCanvas()->Update();      
      
    }break;
    
    
    ///====================== Fit Gaussian
    case M_FINDPEAKS:{
      
      LogMsg("[Find Peaks] Not impelmented");
      
    }break;
    
  }
  
}


void MainWindow::GetADCTrace() {
  
  int modID = modIDEntry->GetNumber();
  int ch = chEntry->GetNumber();

  LogMsg(Form("Get ADCTrace for mod:%d, ch-%02d", modID, ch));

  pixie->CaptureADCTrace(modID, ch);  
  unsigned short * haha =  pixie->GetADCTrace();
  double dt = pixie->GetChannelSetting("XDT", modID, ch); 
  
  TGraph * gTrace = new TGraph();

  for( int i = 0 ; i < pixie->GetADCTraceLength(); i++){
    gTrace->SetPoint(i, i*dt, haha[i]);
  }
  gTrace->GetXaxis()->SetTitle("time [us]");
  gTrace->GetXaxis()->SetRangeUser(0, pixie->GetADCTraceLength()*dt);
  gTrace->SetTitle(Form("Trace mod:%d ch:%0d", modID, ch));
  
  fEcanvas->GetCanvas()->cd();
  gTrace->Draw("APL");
  fEcanvas->GetCanvas()->Update();
  
}

void MainWindow::GetBaseLine(){

  int modID = modIDEntry->GetNumber();
  int ch = chEntry->GetNumber();
  
  LogMsg(Form("Get Baseline for mod:%d, ch-%02d", modID, ch));
  
  pixie->CaptureBaseLine(modID, ch);
  
  double * baseline = pixie->GetBasline();
  double * baselineTime = pixie->GetBaselineTimestamp();
  
  TGraph * gTrace = new TGraph();
  
  for( int i = 0 ; i < pixie->GetBaslineLength(); i++){
    gTrace->SetPoint(i, baselineTime[i]*1000, baseline[i]);
  }
  gTrace->GetXaxis()->SetTitle("time [ns]");
  gTrace->SetTitle(Form("Baseline mod:%d ch:%0d", modID, ch));
  
  fEcanvas->GetCanvas()->cd();
  gTrace->Draw("APL");
  fEcanvas->GetCanvas()->Update();
  
}

void MainWindow::Scope(){
  
  int modID = modIDEntry->GetNumber();
  int ch = chEntry->GetNumber();
  
  if( pixie->GetChannelOnOff(modID, ch) == false ){
    LogMsg(Form("mod:%d, ch-%d is disabled\n", modID, ch));
    return;
  } 
  
  LogMsg(Form("Get trace for mod:%d, ch-%02d", modID, ch));
  
  double dt = pixie->GetCh2ns(modID);
  
  DataBlock * data = pixie->GetData(); 
  
  int runDuration = 200; ///msec
  
  LogMsg(Form("[Scope] Take data for %d msec.", runDuration));
  pixie->StartRun(1);
  
  usleep(runDuration*1000);
  pixie->ReadData(0);
  pixie->StopRun();
  
  delete gTrace;
  gTrace = new TGraph();
  
  ///printf("              next word : %u\n", pixie->GetNextWord());
  ///printf("number of word received : %u\n", pixie->GetnFIFOWords());

  while( pixie->ProcessSingleData() <= 1 ){ /// full set of dataBlack
    
    if( pixie->GetNextWord() >= pixie->GetnFIFOWords() ) break;
    if( data->slot < 2 ) break;
    if(  data->eventID >= pixie->GetnFIFOWords() ) break;
    
    ///printf("mod:%d, ch:%d, event:%llu, %u, %u\n", data->slot-2, data->ch, data->eventID, pixie->GetNextWord(), pixie->GetnFIFOWords() );
      
    if( data->ch == ch && data->slot == modID + 2 ){

      for( int i = 0 ; i < data->trace_length; i++){
        gTrace->SetPoint(i, i*dt, (data->trace)[i]);
      }
      gTrace->GetXaxis()->SetTitle("time [ns]");
      gTrace->GetXaxis()->SetRangeUser(0, data->trace_length * dt);
      gTrace->SetTitle(Form("mod-%d, ch-%02d\n", modID, ch));

      fEcanvas->GetCanvas()->cd();
      gTrace->Draw("APL");
      fEcanvas->GetCanvas()->Update();
    
      bFitTrace->SetEnabled(true);
      
      break;
      
    }
  }
  
  ///printf("=============== finished \n");
  
}

Double_t standardPulse(Double_t *x, Double_t * par){
  
  /// par[0] = start time
  /// par[1] = rise time
  /// par[2] = decay time
  /// par[3] = amplitude
  /// par[4] = baseline
  
  Double_t z = x[0] - par[0];
  
  if( z <= 0 ) {
    return par[4];
  }else{
    return par[3]*(1-TMath::Exp(-1*z/par[1]))*TMath::Exp(-1*z/par[2]) + par[4];
  }

}


void MainWindow::FitTrace(){
  
  int modID = modIDEntry->GetNumber();
  int ch = chEntry->GetNumber();

  ///estimation of parameter;
  double t0 = pixie->GetChannelTraceDelay(modID, ch)*1000.;

  double x, y;
  double xMin = 1e10, xMax = 0, yMin = 1e10, yMax = 0;
  int nData = gTrace->GetN();
  for( int i = 0 ; i < nData ; i++){
    gTrace->GetPoint(i, x, y);
    if( x < xMin) xMin = x;
    if( x > xMax) xMax = x;
    if( y < yMin) yMin = y;
    if( y > yMax) yMax = y;
  }

  double baseline = yMin;
  double amp = yMax-yMin;
  
  double riseTime = 200;
  double decayTime = 40000;
  
  TF1 * traceFunc = new TF1("fit", standardPulse, xMin + t0*0.1, xMax - t0*0.1, 5);
  
  traceFunc->SetNpx(1000);
  traceFunc->SetParameter(0, t0);
  traceFunc->SetParameter(1, riseTime); /// ns
  traceFunc->SetParameter(2, decayTime); /// ns
  traceFunc->SetParameter(3, amp);
  traceFunc->SetParameter(4, baseline);
  
  traceFunc->SetParLimits(0, t0*0.9, t0*1.1);
  traceFunc->SetParLimits(1, 10., 500);
  traceFunc->SetParLimits(2, 10., 100000);
  traceFunc->SetParLimits(3, 10., yMax);
  traceFunc->SetParLimits(4, 10., 2*yMin);
  
  gTrace->Fit("fit", "qR0");
  
  fEcanvas->GetCanvas()->cd();
  traceFunc->Draw("same");
  fEcanvas->GetCanvas()->Update();
  
  LogMsg("========= fit result");
  TString text[5] ={"start time", "rise time", "decay time", "amp", "baseline"};
  TString unit[5] ={"ns", "ns", "ns", "unit", "unit"};
  
  for( int i = 0; i < 5; i++){
    LogMsg(Form(" %10s : %8.2f(%8.2f) %s", text[i].Data(), traceFunc->GetParameter(i), traceFunc->GetParError(i), unit[i].Data()));
  }
  
  delete traceFunc;
}



void MainWindow::GoodBye(){
  
  pixie->CloseDigitizers();
  
  printf("----- bye bye ---- \n");
  
  gApplication->Terminate(0);
  
}

void MainWindow::StartRun(){
  
  TString saveFileName = Form("%s_%03lu.evt", tePath->GetText(), runIDEntry->GetIntNumber());
  
  pixie->OpenFile(saveFileName.Data(), false);
  
  LogMsg(Form("Start Run. Save data at %s.\n", saveFileName.Data()));
  
  ///clear histogram;
  isEnergyHistFilled = false;
  for( unsigned int iMod = 0 ; iMod < pixie->GetNumModule(); iMod++){
    for( int j = 0; j < MAXCH ; j++){
      hEnergy[iMod][j]->Reset();
    }
  }
  
  pixie->StartRun(1);
  if( pixie->IsRunning() ) saveDataThread->Run(); /// call SaveData()
  if( pixie->IsRunning() ) fillHistThread->Run(); /// call SaveData()
  
  bStartRun->SetEnabled(false);
  bStopRun->SetEnabled(true);
}


void MainWindow::StopRun(){
  
  pixie->StopRun();
  pixie->ReadData(0);
  pixie->SaveData();
  
  LogMsg("Stop Run");
  LogMsg(Form("File Size : %.2f MB", pixie->GetFileSize()/1024./1024.));
  pixie->CloseFile();

  pixie->PrintStatistics(0);
  bStartRun->SetEnabled(true);
  bStopRun->SetEnabled(false);
  
  runIDEntry->SetIntNumber(runIDEntry->GetIntNumber()+1);

}


void MainWindow::OpenScalar(){
  
  if( scalarPanel == NULL ) {
    scalarPanel = new ScalarPanel(gClient->GetRoot(), 600, 600, pixie);
  }else{
    if( !scalarPanel->isOpened ) scalarPanel = new ScalarPanel(gClient->GetRoot(), 600, 600, pixie);
  }
}


void MainWindow::ChangeMod(){
  
  if( isEnergyHistFilled ){
    int modID = modIDEntry->GetNumber();
    int ch = chEntry->GetNumber();
    
    fEcanvas->GetCanvas()->cd();
    hEnergy[modID][ch]->Draw();
    fEcanvas->GetCanvas()->Update();  
  }
  
}


void MainWindow::ChangeChannel(){
  ChangeMod();
}


void MainWindow::LogMsg(TString msg){
  
  time_t now = time(0);
  tm * ltm = localtime(&now);
  int year = 1900 + ltm->tm_year;
  int month = 1 + ltm->tm_mon;
  int day = ltm->tm_mday;
  int hour = ltm->tm_hour;
  int minute = ltm->tm_min;
  int secound = ltm->tm_sec;
  
  teLog->AddLine(Form("[%4d-%02d-%02d %02d:%02d:%02d] ", year, month, day, hour, minute, secound) + msg);
  teLog->LineDown();
  teLog->ShowBottom();
}

//############################################ Threads

void * MainWindow::SaveData(void* ptr){
  
  printf("Save Data()\n");
  
  double oldFileSize = 0, newFileSize = 0;
  double oldTime = 0, newTime = 0;
  TBenchmark localClock;
  localClock.Reset();
  localClock.Start("timer");
  
  int pauseTime = 10 ; ///msec

  while( pixie->IsRunning() ){
    
    if( pauseTime > 10 ) usleep(pauseTime*1000); 
    
    if( !pixie->IsRunning() ) break;
    
    pixie->ReadData(0);
    
    if( pixie->GetnFIFOWords() > 0 ) {
      pixie->SaveData();
      ///ScanNumDataBlockInExtFIFO() should be here after ReadData(). becasue not a whlole dataBlock is in FIFO.
      pixie->ScanNumDataBlockInExtFIFO();  //TODO need to check the time comsumtion
      pixie->SetFIFOisUsed(false);
    
      localClock.Stop("timer");
      newTime = localClock.GetRealTime("timer"); /// sec
      localClock.Start("timer");

      ///printf("Thread[SaveData] , FIFO: %u  nData %u | %f\n", pixie->GetnFIFOWords(),  nData, newTime);
      
      if( newTime - oldTime > 1 ) {
        ///double MByteRead = pixie->GetnFIFOWords()*4./1024./1024.;
        ///if( MByteRead > 70 && pauseTime > 10) pauseTime = pauseTime - 20;
      
        time_t now = time(0);
        tm * ltm = localtime(&now);
        int year = 1900 + ltm->tm_year;
        int month = 1 + ltm->tm_mon;
        int day = ltm->tm_mday;
        int hour = ltm->tm_hour;
        int minute = ltm->tm_min;
        int secound = ltm->tm_sec;
        
        newFileSize = pixie->GetFileSize()/1024./1024.;
        
        double rate = (newFileSize - oldFileSize)/ (newTime-oldTime); 
        oldFileSize = newFileSize;
        oldTime = newTime;

        ///teLog->AddLine(Form("[%4d-%02d-%02d %02d:%02d:%02d] File Size : %.2f MB [%.2f MB/s], %.2f MB readed", 
        ///                     year, month, day, hour, minute, secound, newFileSize, rate, MByteRead));
        teLog->AddLine(Form("[%4d-%02d-%02d %02d:%02d:%02d] File Size : %.2f MB [%.2f MB/s] Number of events recorded : %u", 
                             year, month, day, hour, minute, secound, newFileSize, rate, pixie->GetAccumulatedFIFONumDataBlock()));
        teLog->LineDown();
      }
    }
  }
  
  pixie->ReadData(0);
  pixie->SaveData();
  
  printf("finished Save Data.\n");
  
  return ptr;
  
}

void * MainWindow::FillHistogram(void *ptr){
  
  ///double oldTime = 0, newTime = 0;
  ///TBenchmark localClock;
  ///localClock.Reset();
  ///localClock.Start("timer");
  
  while( pixie->IsRunning() ){
    
    if( pixie->GetFIFOisUsed() == false && pixie->GetnFIFOWords() > 0 && pixie->GetFIFONumDataBlock() > 0 ) {
      
      ///localClock.Stop("timer");
      ///newTime = localClock.GetRealTime("timer"); /// sec
      ///localClock.Start("timer");
      
      ///unsigned int nextWord = pixie->GetNextWord();
      unsigned int nData = pixie->GetFIFONumDataBlock();
      unsigned short channels[nData];
      unsigned short energies[nData];
      unsigned short mods[nData];
      long long int  timestamps[nData];
      std::memcpy(mods,     pixie->GetFIFOMods(), sizeof(unsigned short) * nData);
      std::memcpy(channels, pixie->GetFIFOChannels(), sizeof(unsigned short) * nData);
      std::memcpy(energies, pixie->GetFIFOEnergies(), sizeof(unsigned short) * nData);
      std::memcpy(timestamps, pixie->GetFIFOTimestamps(), sizeof(long long int) * nData);
    
      ///printf("Thread[FillHistogram] ============= %u, nData : %d  | %f\n",pixie->GetnFIFOWords(), nData, newTime);
      for( unsigned int i = 0; i < nData; i++){
        ///printf("%3u| %2u, %3u, %7u, %llu \n", i, mods[i], channels[i], energies[i], timestamps[i]);
        hEnergy[mods[i]][channels[i]]->Fill(energies[i]);
        hChannel[mods[i]]->Fill(channels[i]);
      }
      isEnergyHistFilled = true;
      pixie->SetFIFOisUsed(true);
      
    }
    
    int modID = modIDEntry->GetNumber();
    int ch = chEntry->GetNumber();

    fEcanvas->GetCanvas()->cd();
    hEnergy[modID][ch]->Draw();
    fEcanvas->GetCanvas()->Update();  
    
    ///usleep(300*1000);
    
  }
  
  return ptr;
}


//############################################
int main(int argc, char **argv) {
  printf(" Welcome to pixie16 DQ \n");
  
  TApplication theApp("App",&argc,argv);
  new MainWindow(gClient->GetRoot(),800,800);
  theApp.Run();
  return 0;
}
