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

#include "Pixie16Class.h"

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


long get_time();
static struct termios g_old_kbd_mode;
static void cooked(void);  ///set keyboard behaviour as wait-for-enter
static void uncooked(void);  ///set keyboard behaviour as immediate repsond
static void raw(void);
int getch(void);
int keyboardhit();

bool QuitFlag = false;

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

///##################################################
int main(int argc, char *argv[]){

  Pixie16 * pixie = new Pixie16();
  if ( pixie->GetStatus() < 0 ) {
    QuitFlag = true;
    printf("Exiting program... \n");
    return 0;
  }
  
  TApplication * app = new TApplication("app", &argc, argv); 
  
  TCanvas * canvas = new TCanvas("canvas", "Canvas", 1800, 400);
  canvas->Divide(3,1);
  
  TH1F * hch = new TH1F("hch", "channel", 16, 0, 16);
  TH1F * hE = new TH1F("hE", "energy", 400, 0, 30000);
  TGraph * gTrace = new TGraph();
  gTrace->GetXaxis()->SetTitle("time [ch = 4 ns]");

  
  //pixie->SetDigitizerPresetRunTime(100000, 0);
  //pixie->SetDigitizerSynchWait(0, 0); // not simultaneously
  //pixie->SetDigitizerInSynch(1, 0);   //not simultaneously
  
  pixie->PrintDigitizerSettings(0);

  /*
  pixie->GetPolarity(0, 6, 1);
  pixie->SetPolarity(false, 0, 6);
  pixie->GetPolarity(0, 6, 1);
  pixie->SetPolarity(true, 0, 6);
  pixie->GetPolarity(0, 6, 1);
  */

  int ch = 6;
  double time = 1.0; ///sec
  
  /*
  for( int i = 0; i < 16; i++){
      pixie->SetChannelTriggerThreshold(5000, 0, i);
      pixie->SetChannelOnOff(false, 0, i);
      pixie->SetChannelTraceOnOff(false, 0, i);
      pixie->SetChannelVOffset(0, 0, i);
  }
  * */
  //pixie->AdjustOffset();
  
  
  //pixie->SetChannelEnergyRiseTime(2, 0, ch);
  pixie->SetChannelTriggerThreshold(300, 0, ch);
  //pixie->SetChannelEnergyTau(50, 0, ch);
  //pixie->SetChannelOnOff(true, 0, ch);
  //pixie->SetChannelPositivePolarity(true, 0, ch);
  //pixie->SetChannelTraceOnOff(true, 0, ch);
  pixie->SetChannelBaseLinePrecent(90, 0, ch);
  //pixie->SetChannelVOffset(0.0, 0, ch);
  //pixie->SetChannelTraceLenght(10, 0, ch);
  //pixie->SetChannelTraceDelay(2, 0, ch);
  pixie->SetChannelGain(1, 0, ch);
  pixie->SaveSettings("test_ryan.set");
  
  
  pixie->PrintChannelAllSettings(0, ch);
  
  pixie->PrintChannelsMainSettings(0);
  
  /*
  pixie->CaptureADCTrace(0, ch);
  unsigned short * haha =  pixie->GetADCTrace();
  for( int i = 0 ; i < pixie->GetADCTraceLength(); i++){
    gTrace->SetPoint(i, i, haha[i]);
  }
  gTrace->GetXaxis()->SetTitle("time [us]");
  canvas->cd(3); gTrace->Draw("APL");
  */
  
  //pixie->CaptureBaseLine(0, ch);
  //double * baseline = pixie->GetBasline();
  //double * baselineTime = pixie->GetBaselineTimestamp();
  //for( int i = 0 ; i < pixie->GetBaslineLength(); i++){
  //  gTrace->SetPoint(i, baselineTime[i]*1000, baseline[i]);
  //  //printf("%5d | %f, %f \n", i, baselineTime[i]*1000, baseline[i]);
  //}
  //canvas->cd(3); gTrace->Draw("APL");
  

  
  printf("start run for %f sec\n", time);
  
  uint32_t StartTime = get_time(), CurrentTime = get_time();
  pixie->StartRun(1);
  
  DataBlock * data = pixie->GetData();
  
  while( CurrentTime - StartTime < time * 1000 ){
    
    pixie->ReadData(0);
    
    while( pixie->GetNextWord() < pixie->GetnFIFOWords() ){

      //if( data->eventID %100 == 99 ) pixie->PrintExtFIFOWords();
      bool breakFlag = pixie->ProcessSingleData();

      //data->Print(0);
      //printf("--------------next  word : %d (%d) | event lenght : %d \n", pixie->GetNextWord(), pixie->GetnFIFOWords(),  data->eventLength);
      
        hch->Fill( data->ch);
        hE->Fill( data->energy );
        
        if( data->eventID %10 == 0 ){
          if( data->trace_length > 0 ) {
            for( int i = 0 ; i < data->trace_length; i++){
              gTrace->SetPoint(i, i, data->trace[i]);
            }
            gTrace->GetYaxis()->SetRangeUser(0, 5000);
            canvas->cd(3); gTrace->Draw("APL");        
          }
        }
        
        if( data->eventID %100 == 0 ){
          //data->Print(0);
          canvas->cd(1); hch->Draw();
          canvas->cd(2); hE->Draw();
          canvas->Modified();
          canvas->Update();
          gSystem->ProcessEvents();
        }
        if( breakFlag ) break;
    }
    
    CurrentTime = get_time();
  }
  
  pixie->StopRun();
  
  //pixie->PrintData();
  
  
  printf("===================================\n");
  pixie->PrintStatistics(0);

  
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
  /**/

  
  //delete pixie;
  
  app->Run();
  
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
