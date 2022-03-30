#include <TQObject.h>
#include <RQ_OBJECT.h>

#include <TROOT.h>
#include <TClass.h>
#include <TGClient.h>
#include <TGMenu.h>
#include <TGTextEdit.h>
#include <TThread.h>

#include "Pixie16Class.h"
#include "settingsSummary.h"
#include "scalarPanel.h"

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class TGNumberEntry;

class MainWindow{
   RQ_OBJECT("MainWindow")
private:
   TGMainFrame         *fMain;
   TRootEmbeddedCanvas *fEcanvas;
   
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuSettings;
   
   TGNumberEntry * modIDEntry, *chEntry; 
   TGTextEntry * tePath;
   TGTextEdit * teLog;
   
   static Pixie16 * pixie;
   
   SettingsSummary * mainSettings;
   
   ScalarPanel * scalarPanel;
   
   TThread * thread;


public:
   MainWindow(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~MainWindow();
   
   void HandleMenu(Int_t id);
   
   void GetADCTrace();
   void GetBaseLine();
   void Scope();

   void StartRun(); // *SIGNAL*
   static void * SaveData(void* ptr ); /// thread
   void StopRun();

   void OpenScalar();


   void LogMsg(TString msg);

   void GoodBye();

};
