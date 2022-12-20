//*.h are C header
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <bitset>
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
#include "evtReader.h"

long get_time();
void ProcessData();  // using evtReader.h
void FillHistograms(uint32_t period = 500);
void PrintCommands();
void GetADCTrace(int ch);
void GetBaseline(int ch);

bool QuitFlag = false;

Pixie16 * pixie = 0;
TGraph * gTrace = 0;
TCanvas * canvas = 0;
TH1F * hch = 0;
TH1F * hE[16] = {0};
int ch2ns;
uint32_t StartTime = 0, CurrentTime =0 ;
uint32_t ElapsedTime = 0, PresenTime = 0;

///##################################################
int main(int argc, char *argv[]){

  printf("Removing Pixie16Msg.log \n");
  remove( "Pixie16Msg.log");

  //============= Root things
  TApplication * app = new TApplication("app", &argc, argv); 

  canvas = new TCanvas("canvas", "Canvas", 1600, 2000);
  canvas->Divide(4,5);
  
  hch = new TH1F("hch", "channel", 16, 0, 16);
  for( int i = 0; i < 16 ; i ++){
    hE[i] = new TH1F(Form("hE%02d",i), Form("energy-%02d", i), 400, 0, 30000);
  }
  gTrace = new TGraph();
  

  //=========================== open pixie digitizer
  pixie = new Pixie16();
  if ( pixie->GetStatus() < 0 ) {
    printf("Exiting program... \n");
    return 0;
  }

  ch2ns = pixie->GetCh2ns(0);
  gTrace->GetXaxis()->SetTitle("time [ns]");  
  pixie->PrintDigitizerSettings(0);

  double time = 1.0; ///sec

  pixie->PrintChannelSettingsSummary(0);
  pixie->OpenFile("haha.evt", false);

  pixie->ClearFIFOData();
  

  //=========================== Start run loop
  printf("start run for %f sec\n", time);
  
  StartTime = get_time(), CurrentTime = get_time();
  ElapsedTime = 0, PresenTime = StartTime;
  
  pixie->StartRun(1);  
  while( CurrentTime - StartTime < time * 1000 ){
    
    pixie->ReadData(0); // this will reset the FIFONumDataBlock;
    if( pixie->GetnFIFOWords() > 0 ) {
      pixie->SaveData();  
      FillHistograms();
    }
    CurrentTime = get_time();    
    ElapsedTime = CurrentTime - PresenTime;
  }
  
  pixie->StopRun();
  pixie->ReadData(0);
  if( pixie->GetnFIFOWords() > 0 ) pixie->SaveData();  
  pixie->CloseFile();
  
  FillHistograms(0);
  
  pixie->PrintStatistics(0);
  
  unsigned int haha = pixie->GetTotalNumWords();
  unsigned int kaka = pixie->GetAccumulatedFIFONumDataBlock();
  printf("=========== total nFIFO words : %d (%d) \n", haha, kaka);



  app->Run();  
  printf("================ end of program. \n");
  return 0;
}

///##################################################
void FillHistograms(uint32_t period){

  unsigned int nData = pixie->ScanNumDataBlockInExtFIFO();
  unsigned short * energies =  pixie->GetFIFOEnergies();
  unsigned short * channels =  pixie->GetFIFOChannels();
  //unsigned long long * timestamps = pixie->GetFIFOTimestamps();
  
  for( unsigned int h = 0; h < nData; h++) {
    //printf(" %3u| %8u,  %8u , %20llu\n", h, channels[h],  energies[h], timestamps[h]);
    hch->Fill(channels[h] );
    hE[channels[h]]->Fill( energies[h] );
  }
  
  if( ElapsedTime > period ) {
      pixie->PrintStatistics(0);
      PresenTime = CurrentTime;
      ElapsedTime = 0;
      
      for( int i = 0; i < 16; i++){
        canvas->cd(i+1); hE[i]->Draw();
      }
      canvas->cd(17); hch->Draw();
      canvas->Modified();
      canvas->Update();
      gSystem->ProcessEvents();
  }

}

long get_time(){
  long time_ms;
  struct timeval t1;
  struct timezone tz;
  gettimeofday(&t1, &tz);
  time_ms = (t1.tv_sec) * 1000 + t1.tv_usec / 1000;
  return time_ms;
}


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
      hE[data->ch]->Fill( data->energy );
      
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
        canvas->cd(17); hch->Draw();
        for( int i = 0; i < 16; i++) {canvas->cd(i+1); hE[i]->Draw();}
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
  canvas->cd(17); hch->Draw();
  for( int i = 0; i < 16; i++) {canvas->cd(i+1); hE[i]->Draw();}
  canvas->Modified();
  canvas->Update();
  gSystem->ProcessEvents();
  
}
