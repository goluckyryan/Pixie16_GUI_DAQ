#include <TQObject.h>
#include <RQ_OBJECT.h>

#include <TROOT.h>
#include <TClass.h>
#include <TGClient.h>
#include <TGMenu.h>
#include <TGTextEdit.h>
#include <TThread.h>
#include <TH1F.h>
#include <TGraph.h>
#include <TBenchmark.h>
#include <TGTextEditor.h>

#include "Pixie16Class.h"
#include "global_macro.h"
#include "settingsSummary.h"
#include "scalarPanel.h"
#include "moduleSetting.h"
#include "channelSetting.h"
#include "startStopDialog.h"

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class TGNumberEntry;

class MainWindow{
   RQ_OBJECT("MainWindow")
private:
   TGMainFrame         *fMain;
   static TRootEmbeddedCanvas *fEcanvas;
   
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuSettings, *fMenuUtility;
   
   static TGNumberEntry * modIDEntry, *chEntry; 

   TGNumberEntry * runIDEntry;
   TGTextEntry * tePath;
   
   static TGTextEdit * teLog;
   
   TGTextButton *bStartRun;
   TGTextButton *bStopRun;

   TGTextButton *bFitTrace;

   static Pixie16 * pixie;
   
   SettingsSummary * settingsSummary;
   ModuleSetting * moduleSetting;
   ChannelSetting * channelSetting;
   ScalarPanel * scalarPanel;
   StartStopDialog * startStopDialog;
   
   TThread * saveDataThread;
   TThread * fillHistThread;

   static TH1F * hEnergy[MAXMOD][MAXCH];
   static TH1F * hChannel[MAXMOD];
   static bool isEnergyHistFilled;

   TGraph * gTrace;
   
   TGTextEditor * configEditor;


public:
   MainWindow(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~MainWindow();
   
   void HandleMenu(Int_t id);
   
   void GetADCTrace();
   void GetBaseLine();
   void Scope();
   void FitTrace();

   void StartRun();
   static void * SaveData(void* ptr ); /// thread
   void StopRun();

   void OpenScalar();

   static void * FillHistogram(void * ptr); /// thread
   void ChangeMod();
   void ChangeChannel();


   void LogMsg(TString msg);

   void GoodBye();

};
