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
  
  pixie->GetDigitizerSettings(0);

  ///pixie->GetHostCSR(0);
  
  ///pixie->SaveSettings("haha.set");
  
  /*
  pixie->GetPolarity(0, 6, 1);
  pixie->SetPolarity(false, 0, 6);
  pixie->GetPolarity(0, 6, 1);
  pixie->SetPolarity(true, 0, 6);
  pixie->GetPolarity(0, 6, 1);
  */

  
  int ch = 6;
  double time = 0.5; ///sec
  
  /*
  for( int i = 0; i < 16; i++){
    if( i == ch ){
      pixie->WriteChannelTriggerThreshold(10, 0, i);
      pixie->WriteChannelEnergyRiseTime(4, 0, i);    
      pixie->WriteChannelEnergyTau(50, 0, i);
      pixie->WriteChannelTraceLenght(1, 0, i);
      pixie->WriteChannelTraceDelay(0.5, 0, i);
      pixie->SetPositivePolarity(true, 0, i);
      pixie->SetTraceOnOff(false, 0, i);
  
    }else{
      pixie->WriteChannelTriggerThreshold(500, 0, i);
      pixie->SetChannleOnOff(false, 0, i);
    }
  }
  */
  pixie->WriteChannelTriggerThreshold(500, 0, 6);
  pixie->SaveSettings("test_ryan.set");
  
  pixie->PrintChannelsMainSettings(0);
  
  printf("start run for %f sec\n", time);
  pixie->StartRun(1);
  
  ///pixie->GetHostCSR(0);
  
  usleep(time*1e6);
  pixie->StopRun();
  
  pixie->ReadData(0);
  
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
