#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>

#include "mainSettings.h"


MainSettings::MainSettings(const TGWindow *p, UInt_t w, UInt_t h){
  
  /// Create a main frame
  fMain = new TGMainFrame(p,w,h);
  
  /// Set a name to the main frame
  fMain->SetWindowName("Pixie16 Main Settings");

  /// Map main frame
  fMain->MapWindow();
}


MainSettings::~MainSettings(){
  
  fMain->Cleanup();
  delete fMain;
  
}
