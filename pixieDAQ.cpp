#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGraph.h>
#include <TAxis.h>
#include "pixieDAQ.h"

#include "Pixie16Class.h"
   
Pixie16 * pixie;

MainWindow::MainWindow(const TGWindow *p,UInt_t w,UInt_t h) {
  
  openPixie();
  
  /// Create a main frame
  fMain = new TGMainFrame(p,w,h);
  ///fMain->SetWMPosition(500, 500); //does not work

  /// Create canvas widget
  fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fMain,800,400);
  fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  /// Create a horizontal frame widget with buttons
  TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain,200,40);

  TGTextButton *draw = new TGTextButton(hframe,"&Draw");
  draw->Connect("Clicked()","MainWindow",this,"getADCTrace()");
  hframe->AddFrame(draw, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));
  TGTextButton *exit = new TGTextButton(hframe,"&Exit");
  exit->Connect("Clicked()", "MainWindow", this, "GoodBye()");
  
  hframe->AddFrame(exit, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
  fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));

  /// Set a name to the main frame
  fMain->SetWindowName("Pixie16 DAQ");

  /// Map all subwindows of main frame
  fMain->MapSubwindows();

  /// Initialize the layout algorithm
  fMain->Resize(fMain->GetDefaultSize());

  /// Map main frame
  fMain->MapWindow();
  
  
  ///================ pixie
  ///printf("Removing Pixie16Msg.log \n");
  ///remove( "Pixie16Msg.log");
  
  ///pixie = new Pixie16();
  ///if ( pixie->GetStatus() < 0 ) {
  ///  GoodBye();
  ///}
  
}
MainWindow::~MainWindow() {
  /// Clean up used widgets: frames, buttons, layout hints
  fMain->Cleanup();
  delete fMain;
}

void MainWindow::openPixie(){
  
  printf("Removing Pixie16Msg.log \n");
  remove( "Pixie16Msg.log");

  pixie = new Pixie16();
  if ( pixie->GetStatus() < 0 ) {
    printf("Exiting program... \n");
    GoodBye();
  }
  
}

void MainWindow::getADCTrace() {
  /// Draws function graphics in randomly chosen interval
  ///TF1 *f1 = new TF1("f1","sin(x)/x",0,gRandom->Rndm()*10);
  ///f1->SetLineWidth(3);
  ///f1->Draw();

  printf("--------- get ADCTrace \n");
  
  int ch = 6;
  pixie->CaptureADCTrace(0, ch);
  
  unsigned short * haha =  pixie->GetADCTrace();
  double dt = pixie->GetChannelSetting("XDT", 0, ch); 
  
  TGraph * gTrace = new TGraph();
  
  for( int i = 0 ; i < pixie->GetADCTraceLength(); i++){
    gTrace->SetPoint(i, i*dt, haha[i]);
  }
  gTrace->GetXaxis()->SetTitle("time [us]");
  gTrace->Draw("AP");
  
  TCanvas *fCanvas = fEcanvas->GetCanvas();
  fCanvas->cd();
  fCanvas->Update();
  
}


void MainWindow::GoodBye(){
  
  pixie->CloseDigitizers();
  
  printf("----- bye bye ---- \n");
  
  gApplication->Terminate(0);
  
}


int main(int argc, char **argv) {
  printf(" Welcome to pixie16 DQ \n");
  
  TApplication theApp("App",&argc,argv);
  new MainWindow(gClient->GetRoot(),800,600);
  theApp.Run();
  return 0;
}
