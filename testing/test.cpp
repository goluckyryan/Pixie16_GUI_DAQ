//*.h are C header
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <bitset>
#include <unistd.h>
#include <limits.h>
#include <ctime>
#include <sys/time.h> /* struct timeval, select() */
#include <termios.h> /* tcgetattr(), tcsetattr() */

#include "../Pixie16Class.h"

#include "TROOT.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "TCutG.h"
#include "TMultiGraph.h"
#include "TApplication.h"
#include "TObjArray.h"
#include "TLegend.h"
#include "TRandom.h"
#include "TLine.h"
#include "TMacro.h"
#include "TRootCanvas.h"


#include <thread>
#include <unistd.h>


#include "evtReader.h"

long get_time();

bool QuitFlag = false;

Pixie16 * pixie = 0;
TGraph * gTrace = 0;
TCanvas * canvas = 0;
TH1F * hch = 0;
TH1F * hE =  0;
int ch2ns;

void PrintCommands(){
  
  if (QuitFlag) return;
  printf("\n");
  printf("\e[96m=============  Command List  ===================\e[0m\n");
  printf("s ) Start acquisition \n");
  printf("a ) Stop acquisition  \n");
  printf("q ) Quit              \n");
  printf("\n");
  printf("r ) RiseTime \n");
  printf("t ) Trigger \n");

}

void GetADCTrace(int ch){
  
  pixie->CaptureADCTrace(0, ch);
  unsigned short * haha =  pixie->GetADCTrace();
  double dt = pixie->GetChannelSetting("XDT", 0, ch); 
  for( int i = 0 ; i < pixie->GetADCTraceLength(); i++){
    gTrace->SetPoint(i, i*dt, haha[i]);
  }
  gTrace->GetXaxis()->SetTitle("time [us]");
  canvas->cd(3); gTrace->Draw("APL");

}

void GetBaseline(int ch){
  
  pixie->CaptureBaseLine(0, ch);
  double * baseline = pixie->GetBasline();
  double * baselineTime = pixie->GetBaselineTimestamp();
  for( int i = 0 ; i < pixie->GetBaslineLength(); i++){
    gTrace->SetPoint(i, baselineTime[i]*1000, baseline[i]);
    ///printf("%5d | %f, %f \n", i, baselineTime[i]*1000, baseline[i]);
  }
  canvas->cd(2); gTrace->Draw("APL");
  
}


void ProcessData(){
  
  printf("========== new thread of processing data\n");
  
  evtReader * evt = new evtReader("haha.evt");
  DataBlock * data = evt->data;
  
  int jaja = 0;
  
  while(true){  
    
    evt->UpdateFileSize();
    
    while( evt->GetFileSize() > 0 && evt->GetFileSize() > evt->GetFilePos()  ){
      
      int status = evt->ReadBlock();

      if( status == -1 ) break;

      //printf("%d, %ld, %ld, %lld\n", status, evt->GetFilePos(), evt->GetFileSize(), evt->GetBlockID());
      //data->Print(0);
      
      hch->Fill( data->ch);
      hE->Fill( data->energy );
      
      if( data->eventID %100 == 0 ){
        if( data->trace_length > 0 ) {
          for( int i = 0 ; i < data->trace_length; i++){
            gTrace->SetPoint(i, i*ch2ns, data->trace[i]);
          }
          //gTrace->GetYaxis()->SetRangeUser(0, 5000);
          canvas->cd(3); gTrace->Draw("APL");        
        }
      }
      
      if( data->eventID %200 == 0 ){
        //data->Print(0);
        canvas->cd(1); hch->Draw();
        canvas->cd(2); hE->Draw();
        canvas->Modified();
        canvas->Update();
        gSystem->ProcessEvents();
      }
      
    }
    
    usleep(500*1000);
    evt->UpdateFileSize();
    
    jaja++;
    //printf("%10d, %d, %ld, %ld \n", jaja, pixie->IsRunning(), evt->GetFilePos(), evt->GetFileSize());
    //if( jaja > 1000) break;
    if( !pixie->IsRunning() && evt->GetFilePos() == evt->GetFileSize()) break;
    
  }
  
  printf("========= finished ProcessData()\n");
  canvas->cd(1); hch->Draw();
  canvas->cd(2); hE->Draw();
  canvas->Modified();
  canvas->Update();
  gSystem->ProcessEvents();
  
}

///##################################################
int main(int argc, char *argv[]){
  
  printf("Removing Pixie16Msg.log \n");
  remove( "Pixie16Msg.log");

  pixie = new Pixie16();
  if ( pixie->GetStatus() < 0 ) {
    QuitFlag = true;
    printf("Exiting program... \n");
    return 0;
  }
  
  TApplication * app = new TApplication("app", &argc, argv); 
  
  canvas = new TCanvas("canvas", "Canvas", 1800, 400);
  canvas->Divide(3,1);
  
  hch = new TH1F("hch", "channel", 16, 0, 16);
  hE = new TH1F("hE", "energy", 400, 0, 30000);
  gTrace = new TGraph();
  
  ch2ns = pixie->GetCh2ns(0);
  gTrace->GetXaxis()->SetTitle("time [ns]");
  
  pixie->PrintDigitizerSettings(0);

  int ch = 6;
  double time = 4.0; ///sec

  pixie->PrintChannelAllSettings(0, ch);
  
  pixie->PrintChannelSettingsSummary(0);
  
  pixie->OpenFile("haha.evt", false);
  
  printf("start run for %f sec\n", time);
  
  uint32_t StartTime = get_time(), CurrentTime = get_time();
  uint32_t ElapsedTime = 0, PresenTime = StartTime;
  
  pixie->StartRun(1);
  
  std::thread kakakaka(ProcessData);
  
  while( CurrentTime - StartTime < time * 1000 ){
    
    pixie->ReadData(0);
    if( pixie->GetnFIFOWords() > 0 ) pixie->SaveData();  
    
    if( pixie->GetnFIFOWords() > 0 ) {/// msec
      printf("number of dataBlack read : %u\n", pixie->ScanNumDataBlockInExtFIFO());
    }
    
    //TODO a quick way to read and fill histogram; the most time consumping part is filling trace
    
    if( ElapsedTime > 1000 ) {
        pixie->PrintStatistics(0);
        PresenTime = CurrentTime;
        ElapsedTime = 0;
    }
    
    CurrentTime = get_time();
    
    ElapsedTime = CurrentTime - PresenTime;
    
  }
  
  pixie->StopRun();
  pixie->CloseFile();
  
  printf("===================================\n");
  pixie->PrintStatistics(0);
  
  unsigned int haha = pixie->GetTotalNumWords();
  printf("=========== total nFIFO words : %d (%f) \n", haha, haha/1256.);
  
  app->Run();
  
  QuitFlag = true;

  printf("================ end of program. \n");
  return 0;
}

///##################################################
long get_time(){
  long time_ms;
  struct timeval t1;
  struct timezone tz;
  gettimeofday(&t1, &tz);
  time_ms = (t1.tv_sec) * 1000 + t1.tv_usec / 1000;
  return time_ms;
}
