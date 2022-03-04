
#ifndef DATABLOCK_H
#define DATABLOCK_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

class DataBlock{

public:
  unsigned short               ch;
  unsigned short             slot;
  unsigned short            crate;
  unsigned short     headerLength;  /// headerLength > 4, more data except tarce.
  unsigned short      eventLength;  /// eventLength = headerLength + trace 
  bool                     pileup;
  unsigned long long         time;
  unsigned short              cfd;
  unsigned short           energy;
  unsigned short     trace_length;
  bool         trace_out_of_range;
  
  int            trailing;
  int             leading;
  int                 gap;
  int            baseline;
  int           QDCsum[8];
  
  unsigned long long     eventID;
  unsigned short trace[4000];
  
  DataBlock(){
    Clear();
  };
  
  ~DataBlock(){};
  
  void Clear(){
    ch = 0;
    slot = 0;
    crate = 0;
    headerLength = 0;
    eventLength = 0;
    pileup = false;
    time = 0;
    cfd = 0;
    energy = 0;
    trace_length = 0;
    trace_out_of_range = 0;

    eventID = 0;
    ClearQDC();
    ClearTrace();
  }
  
  void ClearQDC(){
    trailing = 0;
    leading = 0;
    gap = 0;
    baseline = 0;
    for( int i = 0; i < 8; i++) QDCsum[i] = -1;
  }
  
  void ClearTrace(){
    for( int i = 0; i < 1024; i++) trace[i] = 0;    
  }
  
  
  void Print(bool printTrace = false){
    printf("============== eventID : %llu\n", eventID);
    printf("Crate: %d, Slot: %d, Ch: %d  \n", crate, slot, ch);
    printf("HeaderLength: %d, Event Length: %d, energy: %d, timeStamp: %llu\n", headerLength, eventLength, energy, time);
    printf("trace_length: %d, pile-up:%d\n", trace_length, pileup); 
    if( headerLength > 4 ){
      if( headerLength > 12 ){
        printf(" trailing : %d\n", trailing);
        printf(" leading  : %d\n", leading);
        printf(" gap      : %d\n", gap);
        printf(" baseLine : %d\n", baseline);
      }
      printf(" QDCsum : \n");
      for( int i = 0; i < 8; i++) printf("    %-10d\n", QDCsum[i]);
    }
    if( printTrace && eventLength > headerLength ){
      printf(" trace:\n");
      for( int i = 0 ; i < trace_length ; i++)printf("%3d|     %-10d\n",i, trace[i]);
    }
  }
  
};

#endif
