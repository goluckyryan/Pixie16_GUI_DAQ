#include "startStopDialog.h"


TString StartStopDialog::Comment = "";
bool StartStopDialog::isOK = false;

StartStopDialog::StartStopDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, bool isStart){
  
  this->isStart = isStart;
  this->isOK = false;
  Comment = "";
  
  fMain = new TGTransientFrame(p, main, w, h, kVerticalFrame);
  fMain->DontCallClose(); /// to avoid double deletions.
  //fMain->SetCleanup(kDeepCleanup); /// use hierarchical cleaning
  fMain->Connect("CloseWindow()", "StartStopDialog", this, "CloseWindow()");
  
  TGHorizontalFrame * fFrame1 = new TGHorizontalFrame(fMain); fMain->AddFrame(fFrame1, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 5, 5, 5, 5));
  txtComment = new TGTextEntry(fFrame1, ""); fFrame1->AddFrame(txtComment);
  txtComment->Resize(300, 20);
  txtComment->Connect("ReturnPressed()", "StartStopDialog", this, "DoOK()");

  TGHorizontalFrame * fFrame2 = new TGHorizontalFrame(fMain); fMain->AddFrame(fFrame2,new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 5, 5, 5, 5));
  TGTextButton * bOK = new TGTextButton(fFrame2, "OK"); fFrame2->AddFrame(bOK, new TGLayoutHints(kLHintsCenterX));
  bOK->Connect("Clicked()", "StartStopDialog", this, "DoOK()");
  TGTextButton * bCancel = new TGTextButton(fFrame2, "Cancel"); fFrame2->AddFrame(bCancel, new TGLayoutHints(kLHintsCenterX));
  bCancel->Connect("Clicked()", "StartStopDialog", this, "DoClose()");

  TGHorizontalFrame * fFrame3 = new TGHorizontalFrame(fMain); fMain->AddFrame(fFrame3,new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 5, 5, 5, 5));
  
  if( isStart ){
    TGLabel * lbInfo = new TGLabel(fFrame3, "* Cancel or Close this dialog box will cancel START run.\n Only OK or press Enter will process."); fFrame3->AddFrame(lbInfo);
  }else{
    TGLabel * lbInfo = new TGLabel(fFrame3, "* Cancel or Close this dialog box will cancel STOP run.\n* Only OK or press Enter will process."); fFrame3->AddFrame(lbInfo);
  }
  
  fMain->MapSubwindows();
  fMain->Resize();

  fMain->CenterOnParent(); /// position relative to the parent's window
  if( isStart) {
    fMain->SetWindowName("Start Comment");
  }else{
    fMain->SetWindowName("Stop Comment");
  }
  fMain->MapWindow();
  
  gClient->WaitFor(fMain); /// make everything wait for it
  
}


StartStopDialog::~StartStopDialog(){
  printf("-------- %s |%s| %d\n", __func__, Comment.Data(), isOK);
  delete txtComment;
  fMain->DeleteWindow();  /// deletes fMain
}

void StartStopDialog::DoClose(){  
  isOK = false;
  ///Wait for 500 msec
  TTimer::SingleShot(500, "StartStopDialog", this, "CloseWindow()");
}

void StartStopDialog::DoOK(){
  Comment = "";
  Comment = txtComment->GetText();
  isOK = true;
  TTimer::SingleShot(500, "StartStopDialog", this, "CloseWindow()");
}
