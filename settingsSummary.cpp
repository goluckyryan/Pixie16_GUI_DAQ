#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGTableContainer.h>
#include <TGFileDialog.h>

#include "settingsSummary.h"


SettingsSummary::SettingsSummary(const TGWindow *p, UInt_t w, UInt_t h, Pixie16 * pixie){
  
  this->pixie = pixie; 
  
  settingFileName = pixie->GetSettingFile(0);
  
  fMain = new TGMainFrame(p,w,h);
  fMain->SetWindowName("Pixie16 Settings Summary");
  fMain->Connect("CloseWindow()", "SettingsSummary", this, "CloseWindow()");
  //fMain->SetMWMHints(1,4, 0);
  
  gClient->GetColorByName("red", red);
  gClient->GetColorByName("black", black);
  
  {///======================= Module choose and settings file
    TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain, w, 50 );
    fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX, 2,2,2,2));
  
    TGLabel * lb1 = new TGLabel(hframe, "Module ID :");
    hframe->AddFrame(lb1, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    modIDEntry = new TGNumberEntry(hframe, 0, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    modIDEntry->SetWidth(50);
    modIDEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, pixie->GetNumModule()-1);
    modIDEntry->Connect("Modified()", "SettingsSummary", this, "ChangeMod()"); 
    if( pixie->GetNumModule() == 1 ) modIDEntry->SetState(false);
    hframe->AddFrame(modIDEntry, new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
  
    TGLabel * lb2 = new TGLabel(hframe, "Setting File :");
    hframe->AddFrame(lb2, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    teFileName = new TGTextEntry(hframe, new TGTextBuffer(50));
    teFileName->SetEnabled(false);
    teFileName->SetText( settingFileName );
    hframe->AddFrame(teFileName, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));
  
    TGTextButton * bOpen = new TGTextButton(hframe,"&Open");
    bOpen->Connect("Clicked()","SettingsSummary",this, "OpenFile()");
    hframe->AddFrame(bOpen, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));
  
    TGTextButton * bSave = new TGTextButton(hframe,"&Save");
    bSave->Connect("Clicked()","SettingsSummary",this, "SaveSetting()");
    hframe->AddFrame(bSave, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));

    TGTextButton * bGetSettingsFromDigitizer = new TGTextButton(hframe,"&Refresh from digitizer");
    bGetSettingsFromDigitizer->Connect("Clicked()","SettingsSummary",this, "GetSettingsFromDigitizer()");
    hframe->AddFrame(bGetSettingsFromDigitizer, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));

  }
  
  {///======================= Setting
    int modID = modIDEntry->GetNumber();
  
    int numItems = 15;
    TString labelText[numItems] = {"CH", "ON/off", "Gain", "Trig L", "Trig G", "Threshold", "Polarity", "Energy L", "Energy G", "Tau", "Trace I/O", "Trace Len", "Trace delay", "V offset", "BaseLine"};
    double width[numItems]     = {  50,       50,    50,     60,       60,           80,       60,       60,          60,       60,        50,         80,           80,             70,      80};

    TGVerticalFrame *hframeSettings = new TGVerticalFrame(fMain, w, 600 );
    fMain->AddFrame(hframeSettings, new TGLayoutHints(kLHintsCenterX, 2,2,2,2));

    ///---------- label row
    TGHorizontalFrame * hframeLabel = new TGHorizontalFrame(hframeSettings, w, 50);
    hframeSettings->AddFrame(hframeLabel, new TGLayoutHints(kLHintsExpandX, 2,2,0,0)); 
    TGLabel * labelItems[numItems];
    for(int i = 0; i < numItems; i++){
      labelItems[i] = new TGLabel(hframeLabel, Form("%s", labelText[i].Data()));
      labelItems[i]->Resize(width[i], 20);
      labelItems[i]->SetMargins(0, (i == 0 ? 0 : 10) , 0, 0);
      hframeLabel->AddFrame(labelItems[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 2, 2));
    }


    TGHorizontalFrame * hframeCh[MAXCH];    
    for( int i = 0; i < MAXCH ; i ++){

      //printf("-----------------------%d\n", i);
      hframeCh[i] = new TGHorizontalFrame(hframeSettings, w, 600 );
      hframeSettings->AddFrame(hframeCh[i], new TGLayoutHints(kLHintsCenterX, 2,2,0,0));
    
      int col = 0;
      lbCh[i] = new TGLabel(hframeCh[i] , Form("%02d", i));
      lbCh[i]->SetWidth(width[col]);
      lbCh[i]->SetTextColor(red);
      hframeCh[i]->AddFrame(lbCh[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 2, 2));
    
      col++;
      cbOnOff[i] = new TGComboBox(hframeCh[i], i);
      cbOnOff[i]->AddEntry("ON", 1);
      cbOnOff[i]->AddEntry("off", 0);
      cbOnOff[i]->Resize(width[col], 20);
      if( pixie->GetChannelOnOff(modID, i) ){
	cbOnOff[i]->Select(1);
	lbCh[i]->SetTextColor(red);
      }else{
	cbOnOff[i]->Select(0);
	lbCh[i]->SetTextColor(black);
      }
      cbOnOff[i]->Connect("Selected(Int_t, Int_t)", "SettingsSummary", this, Form("ChangeOnOff(=%d)", i));  
      hframeCh[i]->AddFrame(cbOnOff[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 2, 2));

      col++;
      cbGain[i] = new TGComboBox(hframeCh[i], i);
      cbGain[i]->AddEntry("x1", 1);
      cbGain[i]->AddEntry("x1/4", 0);
      cbGain[i]->Resize(width[col], 20);
      pixie->GetChannelGain(modID, i) ? cbGain[i]->Select(1) : cbGain[i]->Select(0);
      cbGain[i]->Connect("Selected(Int_t, Int_t)", "SettingsSummary", this, Form("ChangeGain(=%d)", i));  
      hframeCh[i]->AddFrame(cbGain[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 2, 2));

      col++;
      neTrigL[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelTriggerRiseTime(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
      neTrigL[i]->SetWidth(width[col]);
      neTrigL[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 2.0);
      neTrigL[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeTrigL(=%d)", i)); 
      hframeCh[i]->AddFrame(neTrigL[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));

      col++;
      neTrigG[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelTriggerFlatTop(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
      neTrigG[i]->SetWidth(width[col]);
      neTrigG[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 2.0);
      neTrigG[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeTrigG(=%d)", i)); 
      hframeCh[i]->AddFrame(neTrigG[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
      col++;
      neThreshold[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelTriggerThreshold(modID, i), 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
      neThreshold[i]->SetWidth(width[col]);
      neThreshold[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 4000);
      neThreshold[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeThreshold(=%d)", i));  
      hframeCh[i]->AddFrame(neThreshold[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
      col++;
      cbPol[i] = new TGComboBox(hframeCh[i], i);
      cbPol[i]->AddEntry("Pos +", 1);
      cbPol[i]->AddEntry("Neg -", 0);
      cbPol[i]->Resize(width[col], 20);
      pixie->GetChannelPolarity(modID, i) ? cbPol[i]->Select(1) : cbPol[i]->Select(0);
      cbPol[i]->Connect("Selected(Int_t, Int_t)", "SettingsSummary", this, Form("ChangePol(=%d)", i));  
      hframeCh[i]->AddFrame(cbPol[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 2, 2));
    
      col++;
      neEngL[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelEnergyRiseTime(modID, i), 0, 0, TGNumberFormat::kNESRealOne, TGNumberFormat::kNEANonNegative);
      neEngL[i]->SetWidth(width[col]);
      neEngL[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 10.0);
      neEngL[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeEngL(=%d)", i));  
      hframeCh[i]->AddFrame(neEngL[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
      col++;
      neEngG[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelEnergyFlatTop(modID, i), 0, 0, TGNumberFormat::kNESRealOne, TGNumberFormat::kNEANonNegative);
      neEngG[i]->SetWidth(width[col]);
      neEngG[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 10.0);
      neEngG[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeEngG(=%d)", i));  
      hframeCh[i]->AddFrame(neEngG[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));

      col++;
      neTau[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelEnergyTau(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
      neTau[i]->SetWidth(width[col]);
      neTau[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 300.0);
      neTau[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeTau(=%d)", i));  
      hframeCh[i]->AddFrame(neTau[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
  
      col++;
      cbTraceOnOff[i] = new TGComboBox(hframeCh[i], i);
      cbTraceOnOff[i]->AddEntry("On", 1);
      cbTraceOnOff[i]->AddEntry("off", 0);
      cbTraceOnOff[i]->Resize(width[col], 20);    
      pixie->GetChannelTraceOnOff(modID, i) ? cbTraceOnOff[i]->Select(1) : cbTraceOnOff[i]->Select(0);
      cbTraceOnOff[i]->Connect("Selected(Int_t, Int_t)", "SettingsSummary", this, Form("ChangeTraceOnOff(=%d)", i));  
      hframeCh[i]->AddFrame(cbTraceOnOff[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 2, 2));
    
      col++;
      neTraceLength[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelTraceLength(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
      neTraceLength[i]->SetWidth(width[col]);
      neTraceLength[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 20.0);
      neTraceLength[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeTraceLenght(=%d)", i));     
      hframeCh[i]->AddFrame(neTraceLength[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
      col++;
      neTraceDelay[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelTraceDelay(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
      neTraceDelay[i]->SetWidth(width[col]);
      neTraceDelay[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 20.0);
      neTraceDelay[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeTraceDelay(=%d)", i)); 
      hframeCh[i]->AddFrame(neTraceDelay[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
      col++;
      neVoff[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelVOffset(modID, i), 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEAAnyNumber);
      neVoff[i]->SetWidth(width[col]);
      neVoff[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, -2, 2);
      neVoff[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeVoff(=%d)", i)); 
      hframeCh[i]->AddFrame(neVoff[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    
      col++;
      neBL[i] = new TGNumberEntry(hframeCh[i], pixie->GetChannelBaseLinePrecent(modID, i), 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
      neBL[i]->SetWidth(width[col]);
      neBL[i]->SetLimits(TGNumberFormat::kNELLimitMinMax, 0, 100);
      neBL[i]->Connect("Modified()", "SettingsSummary", this, Form("ChangeBL(=%d)", i)); 
      hframeCh[i]->AddFrame(neBL[i], new TGLayoutHints(kLHintsCenterX , 5, 5, 3, 4));
    }
  }

  TGHorizontalFrame * hframeLabel2 = new TGHorizontalFrame(fMain, w, 50);
  fMain->AddFrame(hframeLabel2, new TGLayoutHints(kLHintsExpandX, 2,2,0,0));
  
  TGLabel * lb999 = new TGLabel(hframeLabel2, "When this panel open, it read all settings from the digitizer.");
  hframeLabel2->AddFrame(lb999, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 3, 4));
  
  
  fMain->MapSubwindows();
  fMain->Resize(fMain->GetDefaultSize());
  fMain->MapWindow();
  
  isOpened = true;
}


SettingsSummary::~SettingsSummary(){
  
  isOpened = false;
  
  for (int i = 0; i< MAXCH; i++) {
    delete lbCh[i];
    delete cbOnOff[i] ;
    delete cbGain[i] ;
    delete cbPol[i] ;
    delete cbTraceOnOff[i];
    delete neTrigL[i] ; 
    delete neTrigG[i] ; 
    delete neThreshold[i] ;
    delete neEngL[i]  ;
    delete neEngG[i]  ;
    delete neTau[i]   ;
    delete neTraceLength[i] ;
    delete neTraceDelay[i]  ;
    delete neVoff[i]  ;  
    delete neBL[i];
  }

  delete modIDEntry;
  delete teFileName;
  
  /// fMain must be delete last;
  fMain->Cleanup();
  delete fMain;
  
}

void SettingsSummary::GetSettingsFromDigitizer(){

  printf("===== %s \n", __func__);
  
  short modID = modIDEntry->GetNumber();
  
  for( int i=0; i < MAXCH; i++){
    if( pixie->GetChannelOnOff(modID, i) ){
      cbOnOff[i]->Select(1);
      lbCh[i]->SetTextColor(red);
    }else{
      cbOnOff[i]->Select(0);
      lbCh[i]->SetTextColor(black);
    }

    pixie->GetChannelGain(modID, i) ? cbGain[i]->Select(1) : cbGain[i]->Select(0);

    neTrigL[i]->SetNumber(pixie->GetChannelTriggerRiseTime(modID, i));
    neTrigG[i]->SetNumber(pixie->GetChannelTriggerFlatTop(modID, i));
    neThreshold[i]->SetNumber(pixie->GetChannelTriggerThreshold(modID, i));
    pixie->GetChannelPolarity(modID, i) ? cbPol[i]->Select(1) : cbPol[i]->Select(0);

    neEngL[i]->SetNumber(pixie->GetChannelEnergyRiseTime(modID, i));
    neEngG[i]->SetNumber(pixie->GetChannelEnergyFlatTop(modID, i));
    neTau[i]->SetNumber(pixie->GetChannelEnergyTau(modID, i));

    pixie->GetChannelTraceOnOff(modID, i) ? cbTraceOnOff[i]->Select(1) : cbTraceOnOff[i]->Select(0);
    neTraceLength[i]->SetNumber(pixie->GetChannelTraceLength(modID, i));
    neTraceDelay[i]->SetNumber(pixie->GetChannelTraceDelay(modID, i));

    neVoff[i]->SetNumber(pixie->GetChannelVOffset(modID, i));
    neBL[i]->SetNumber(pixie->GetChannelBaseLinePrecent(modID, i));
    
  }
}
void SettingsSummary::ChangeOnOff(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  int val = cbOnOff[ch]->GetSelected();
  pixie->SetChannelOnOff(val, modID, ch);
  if( val ) {
    lbCh[ch]->SetTextColor(red);
  }else{
    lbCh[ch]->SetTextColor(black);
  }
  teFileName->SetText(settingFileName + "  (not saved)");
}
void SettingsSummary::ChangeGain(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  int val = cbGain[ch]->GetSelected();
  pixie->SetChannelGain(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangePol(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  int val = cbPol[ch]->GetSelected();
  pixie->SetChannelPositivePolarity(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeTraceOnOff(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  int val = cbTraceOnOff[ch]->GetSelected();
  pixie->SetChannelTraceOnOff(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeTrigL(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neTrigL[ch]->GetNumber();  
  pixie->SetChannelTriggerRiseTime(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeTrigG(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neTrigG[ch]->GetNumber();  
  pixie->SetChannelTriggerFlatTop(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}
void SettingsSummary::ChangeThreshold(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neThreshold[ch]->GetNumber();  
  pixie->SetChannelTriggerThreshold(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeEngL(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neEngL[ch]->GetNumber();  
  pixie->SetChannelEnergyRiseTime(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeEngG(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neEngG[ch]->GetNumber();
  pixie->SetChannelEnergyFlatTop(val, modID, ch);  
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeTau(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neTau[ch]->GetNumber();  
  pixie->SetChannelEnergyTau(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeTraceLenght(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neTraceLength[ch]->GetNumber();  
  pixie->SetChannelTraceLenght(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeTraceDelay(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neTraceDelay[ch]->GetNumber();  
  pixie->SetChannelTraceDelay(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeVoff(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neVoff[ch]->GetNumber();  
  pixie->SetChannelVOffset(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}

void SettingsSummary::ChangeBL(unsigned short ch){
  short modID = modIDEntry->GetNumber();
  double val = neBL[ch]->GetNumber();  
  pixie->SetChannelBaseLinePrecent(val, modID, ch);
  teFileName->SetText(settingFileName + "  (not saved)");
}


void SettingsSummary::ChangeMod(){
  short modID = modIDEntry->GetNumber();
  settingFileName = pixie->GetSettingFile(modID);
  teFileName->SetText(settingFileName);
}

void SettingsSummary::OpenFile(){
  
  static TString dir(".");
  TGFileInfo fi;
  const char *filetypes[] = { "Setting file",     "*.set",};
  fi.fFileTypes = filetypes;
  fi.SetIniDir(dir);
  printf("fIniDir = %s\n", fi.fIniDir);
  new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fi);
  printf("Open file: |%s| (dir: %s)\n", fi.fFilename, fi.fIniDir);
  if( fi.fFilename != NULL ) {
    settingFileName = fi.fFilename;
    teFileName->SetText(settingFileName + "  (not saved)");
    //TODO change the pixie->DSPParFile[]
  }
  dir = fi.fIniDir;  
}

void SettingsSummary::SaveSetting(){
  pixie->SaveSettings(settingFileName.Data());
  teFileName->SetText(settingFileName + "  (saved)");
}
