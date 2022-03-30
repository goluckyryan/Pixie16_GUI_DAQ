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
static struct termios g_old_kbd_mode;
static void cooked(void);  ///set keyboard behaviour as wait-for-enter
static void uncooked(void);  ///set keyboard behaviour as immediate repsond
static void raw(void);
int getch(void);
int keyboardhit();

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
    //printf("%5d | %f, %f \n", i, baselineTime[i]*1000, baseline[i]);
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
  
  //pixie->SetDigitizerPresetRunTime(100000, 0);
  //pixie->SetDigitizerSynchWait(0, 0); // not simultaneously
  //pixie->SetDigitizerInSynch(1, 0);   //not simultaneously
  
  pixie->PrintDigitizerSettings(0);

  int ch = 6;
  double time = 10.0; ///sec
  
  //pixie->AdjustOffset();
  
  //pixie->SetChannelEnergyRiseTime(2, 0, ch);
  //pixie->SetChannelTriggerThreshold(300, 0, ch);
  //pixie->SetChannelEnergyTau(50, 0, ch);
  //pixie->SetChannelOnOff(true, 0, ch);
  //pixie->SetChannelPositivePolarity(true, 0, ch);
  //pixie->SetChannelTraceOnOff(true, 0, ch);
  //pixie->SetChannelBaseLinePrecent(10, 0, ch);
  //pixie->SetChannelVOffset(-0.5, 0, ch);
  //pixie->SetChannelTraceLenght(10, 0, ch);
  //pixie->SetChannelTraceDelay(2, 0, ch);
  //pixie->SetChannelGain(1, 0, ch);
  //pixie->SaveSettings("test_ryan.set");
  
  
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
    pixie->SaveData();  
    
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

  //kakakaka.join();
  
  //pixie->SaveSettings("/home/ryan/Pixie16/ryan/test_ryan.set");
  
  //pixie->GetTrace(0,0);
  
  /*
  uint32_t StartTime = get_time(), CurrentTime, ElapsedTime;
  uint32_t PreviousTime = get_time();
  
  uint32_t updatePeriod = 1000;
  
  PrintCommands();
  
  //start event loop
  while(!QuitFlag){
    
    if( keyboardhit()){
        char c = getch();
        
        if( c =='s' ){
          pixie->StartRun(1);
          StartTime = get_time();
          
        }
        
        if( c == 'a'){
          pixie->StopRun();
            
        }
        
        if( c == 'q'){
          pixie->StopRun();
          QuitFlag = true;
        }
        
        if( c == 'r'){
          float haha;
          cooked();
          printf("Rise time [us] ? ");
          int temp = scanf("%f", &haha);
          pixie->WriteChannelEnergyRiseTime((double)haha, 0, 0);
          
          printf("Rise time for channle 0 is now : %f us\n", pixie->GetChannelEnergyRiseTime(0,0));
          uncooked();
          
        }

        if( c == 't'){
          float haha;
          cooked();
          printf("Trigger [ADC] ? ");
          int temp = scanf("%f", &haha);
          pixie->WriteChannelTriggerThreshold(haha, 0, 0);
          
          printf("Tigger for channle 0 is now : %f ADC\n", pixie->GetChannelTriggerThreshold(0,0));
          uncooked();
          
        }
      
    }

    
    CurrentTime = get_time();
    ElapsedTime = CurrentTime - PreviousTime; /// milliseconds

    if (ElapsedTime > updatePeriod){
      system("clear");
      printf("\n======== Tree, Histograms, and Table update every ~%.2f sec\n", updatePeriod/1000.);
      if( pixie->IsRunning() ) {
        printf("\033[1;33m DAQ is running \033[m\n");
        printf("        Rise time for channle 0: %f us\n", pixie->GetChannelEnergyRiseTime(0,0));
        printf("         Flat top for channle 0: %f us\n", pixie->GetChannelEnergyFlatTop(0,0));
        printf("       Decay time for channle 0: %f us\n", pixie->GetChannelEnergyTau(0,0));
        printf("Trigger threshold for channle 0: %.0f ADC\n", pixie->GetChannelTriggerThreshold(0,0));
      }
      printf("Time Elapsed         = %.3f sec = %.1f min\n", (CurrentTime - StartTime)/1e3, (CurrentTime - StartTime)/1e3/60.);
      pixie->ReadData(0);
      
      pixie->PrintData();
      
      PreviousTime = CurrentTime;
          
      PrintCommands();
      
    }
    
  }
  */

  
  //delete pixie;
  
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


static void cooked(void){
  tcsetattr(0, TCSANOW, &g_old_kbd_mode);
}

static void uncooked(void){
  struct termios new_kbd_mode;
  /* put keyboard (stdin, actually) in raw, unbuffered mode */
  tcgetattr(0, &g_old_kbd_mode);
  memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
  new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
  new_kbd_mode.c_cc[VTIME] = 0;
  new_kbd_mode.c_cc[VMIN] = 1;
  tcsetattr(0, TCSANOW, &new_kbd_mode);
}

static void raw(void){

  static char init;
  if(init) return;
  /* put keyboard (stdin, actually) in raw, unbuffered mode */
  uncooked();
  /* when we exit, go back to normal, "cooked" mode */
  atexit(cooked);

  init = 1;
}


int getch(void){
  unsigned char temp;
  raw();
  /* stdin = fd 0 */
  if(read(0, &temp, 1) != 1) return 0;
  //printf("%s", &temp);
  return temp;
}




int keyboardhit(){

  struct timeval timeout;
  fd_set read_handles;
  int status;

  raw();
  /* check stdin (fd 0) for activity */
  FD_ZERO(&read_handles);
  FD_SET(0, &read_handles);
  timeout.tv_sec = timeout.tv_usec = 0;
  status = select(0 + 1, &read_handles, NULL, NULL, &timeout);
  if(status < 0){
    printf("select() failed in keyboardhit()\n");
    exit(1);
  }
  return (status);
}
