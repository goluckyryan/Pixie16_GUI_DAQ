#ifndef START_STOP_DIALOG_H
#define START_STOP_DIALOG_H

#include <TQObject.h>
#include <RQ_OBJECT.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TTimer.h>
#include <TGTextEntry.h>

class TGWindow;
class TGMainFrame;

class StartStopDialog{
  RQ_OBJECT("StartStopDialog")
  private:
    
    TGTextEntry * txtComment;
    bool isStart;
  
  public:
    StartStopDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, bool isStart);
    virtual ~StartStopDialog();
    
    TGTransientFrame * fMain;
    
    void DoClose();
    void DoOK();
    
    void CloseWindow() { delete this;}
    
    static TString Comment;
    static bool isOK;

};

#endif
